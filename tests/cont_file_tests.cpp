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