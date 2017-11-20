//
// Created by Chani Jindal on 11/18/17.
//

#include <catch.hpp>
#include <fs270/block_dev.hpp>
#include <array>

#define BLOCK_SIZE 10 * 1024 * 1024
#define EMULATOR_SIZE 4096
TEST_CASE("blk dev", "[fs][block_device]")
{
    fs::block_dev dev("/tmp/filesystem", 10 * 1024 * 1024, 4096);
    REQUIRE(dev.capacity() == 10 * 1024 * 1024);
    REQUIRE(dev.get_block_size() == 4096);

    std::array<char, 4096> buf;
    buf.fill('a');

    dev.write(1, buf.data());

    buf.fill('b');
    dev.read(1, buf.data());

    REQUIRE(std::all_of(begin(buf), end(buf), [](char x){ return x == 'a'; }));
}

TEST_CASE("blk dev read", "[fs][block_device]")
{
    fs::block_dev dev("/tmp/filesystem", 10 * 1024 * 1024, 4096);

    std::array<char, 4096> buf;
    buf.fill('b');

    dev.read(1, buf.data());
    REQUIRE(std::all_of(begin(buf), end(buf), [](char x){ return x == 'a'; }));
}