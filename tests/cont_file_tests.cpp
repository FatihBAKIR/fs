//
// Created by fatih on 10/20/17.
//

#include <catch.hpp>
#include <fs270/contiguous_data.hpp>
#include <fs270/ram_block_dev.hpp>

TEST_CASE("cont file", "[fs][cont_file]")
{
    fs::ram_block_dev dev(10 * 1024 * 1024, 4096);
    fs::cont_file file = fs::create_cont_file(&dev);

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

TEST_CASE("cont file large", "[fs][cont_file]")
{
    fs::ram_block_dev dev(100 * 1024 * 1024, 4096);
    fs::cont_file file = fs::create_cont_file(&dev);

    for (int i = 0; i < 6; ++i)
    {
        REQUIRE(file.push_block(i + 100));
    }

    for (int i = 6; i < 25; ++i)
    {
        REQUIRE_FALSE(file.push_block(i + 100));
    }

    int k = 0;
    while(file.get_pushable_count() < 25000)
    {
        file.alloc_indirect_block(2000 + k++);
    }

    for (int i = 6; i < 25000; ++i)
    {
        REQUIRE(file.push_block(i + 100));
    }

    REQUIRE(file.get_actual_block(5) == 105);
    REQUIRE(file.get_actual_block(6) == 106);
    REQUIRE(file.get_actual_block(24000) == 24100);
}

TEST_CASE("cont file capacity", "[fs][cont_file]")
{
    fs::ram_block_dev dev(10 * 1024 * 1024, 4096);
    fs::cont_file file = fs::create_cont_file(&dev);

    REQUIRE(file.get_block_count() == 0);
    REQUIRE(file.get_capacity() == 0);

    file.push_block(1);
    REQUIRE(file.get_capacity() == dev.get_block_size());
    file.push_block(2);
    REQUIRE(file.get_capacity() == dev.get_block_size() * 2);
}

TEST_CASE("cont file write_cont_file/read", "[fs][cont_file]")
{
    fs::ram_block_dev dev(10 * 1024 * 1024, 4096);
    {
        fs::cont_file file = fs::create_cont_file(&dev);
        REQUIRE(file.push_block(3));
        REQUIRE(file.get_block_count() == 1);
        REQUIRE(file.get_actual_block(0) == 3);

        REQUIRE(file.push_block(12));
        REQUIRE(file.get_block_count() == 2);
        REQUIRE(file.get_actual_block(0) == 3);
        REQUIRE(file.get_actual_block(1) == 12);

        write_cont_file(&dev, 10000, file);
    }

    {
        fs::cont_file file = read_cont_file(&dev, 10000);
        REQUIRE(file.get_block_count() == 2);
        REQUIRE(file.get_actual_block(0) == 3);
        REQUIRE(file.get_actual_block(1) == 12);
    }
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
