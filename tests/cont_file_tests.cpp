//
// Created by fatih on 10/20/17.
//

#include <catch.hpp>
#include <fs270/contiguous_data.hpp>
#include <fs270/ram_block_dev.hpp>

TEST_CASE("cont file", "[fs][cont_file]")
{
    fs::ram_block_dev dev(10 * 1024 * 1024, 4096);
    fs::cont_file file = fs::create(&dev);

    REQUIRE(file.get_block_count() == 0);

    file.push_block(3);
    REQUIRE(file.get_block_count() == 1);
    REQUIRE(file.get_actual_block(0) == 3);

    file.push_block(12);
    REQUIRE(file.get_block_count() == 2);
    REQUIRE(file.get_actual_block(0) == 3);
    REQUIRE(file.get_actual_block(1) == 12);

    REQUIRE_THROWS(file.get_actual_block(2));

    file.pop_block();
    REQUIRE(file.get_block_count() == 1);
    REQUIRE_THROWS(file.get_actual_block(1));
}

TEST_CASE("calc_path direct", "[fs][cont_file][cont_data]")
{
    fs::detail::contiguous_data data = {};
    REQUIRE(data.block_count == 0);

    REQUIRE_THROWS(fs::detail::calc_path(0, data, 4096));

    data.block_count = 6;

    auto p = fs::detail::calc_path(0, data, 4096);
    REQUIRE(p.size() == 1);
    REQUIRE(p[0] == 0);

    p = fs::detail::calc_path(5, data, 4096);
    REQUIRE(p.size() == 1);
    REQUIRE(p[0] == 5);
}

TEST_CASE("calc_path indirect", "[fs][cont_file][cont_data]")
{
    fs::detail::contiguous_data data = {};
    REQUIRE(data.block_count == 0);

    data.block_count = 8;
    auto p = fs::detail::calc_path(6, data, 4096);
    REQUIRE(p.size() == 2);
    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 0);

    p = fs::detail::calc_path(7, data, 4096);
    REQUIRE(p.size() == 2);
    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 1);

    data.block_count = 2048;
    p = fs::detail::calc_path(1029, data, 4096);
    REQUIRE(p.size() == 2);
    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 1023);

    p = fs::detail::calc_path(1030, data, 4096);
    REQUIRE(p.size() == 2);
    REQUIRE(p[0] == 1);
    REQUIRE(p[1] == 0);
}

TEST_CASE("calc_path double indirect", "[fs][cont_file][cont_data]")
{
    fs::detail::contiguous_data data = {};
    data.block_count = 3072 + 6 + 3072; // double indirect

    auto p = fs::detail::calc_path(3078, data, 4096);
    REQUIRE(p.size() == 3);
    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 0);
    REQUIRE(p[2] == 0);

    p = fs::detail::calc_path(3087, data, 4096);
    REQUIRE(p.size() == 3);
    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 0);
    REQUIRE(p[2] == 9);

    p = fs::detail::calc_path(3087 + 1024, data, 4096);
    REQUIRE(p.size() == 3);
    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 1);
    REQUIRE(p[2] == 9);
}
