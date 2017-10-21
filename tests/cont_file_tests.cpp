//
// Created by fatih on 10/20/17.
//

#define CATCH_CONFIG_MAIN
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
}

TEST_CASE("calc_path", "[fs][cont_file][cont_data]")
{
    fs::detail::contiguous_data data = {};
    REQUIRE(data.block_count == 0);

    REQUIRE_THROWS(fs::detail::calc_path(0, data, 4096));

    data.block_count = 1;

    auto p = fs::detail::calc_path(0, data, 4096);
    REQUIRE(p.size() == 1);
    REQUIRE(p[0] == 0);

    data.block_count = 8;
    p = fs::detail::calc_path(6, data, 4096);
    REQUIRE(p.size() == 2);
    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 0);

    p = fs::detail::calc_path(7, data, 4096);
    REQUIRE(p.size() == 2);
    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 1);
}