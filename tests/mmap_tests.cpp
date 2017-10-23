//
// Created by fatih on 10/22/17.
//

#include <catch.hpp>
#include <fs270/mmap_block_dev.hpp>
#include <array>

TEST_CASE("mmap blk dev", "[fs][mmap]")
{
    fs::mmap_block_dev dev("/tmp/test", 10 * 1024 * 1024, 4096);
    REQUIRE(dev.capacity() == 10 * 1024 * 1024);
    REQUIRE(dev.get_block_size() == 4096);

    std::array<char, 4096> buf;
    buf.fill('a');

    dev.write(0, buf.data());

    buf.fill('b');
    dev.read(0, buf.data());

    REQUIRE(std::all_of(begin(buf), end(buf), [](char x){ return x == 'a'; }));
}

TEST_CASE("mmap blk dev read", "[fs][mmap]")
{
    fs::mmap_block_dev dev("/tmp/test", 10 * 1024 * 1024, 4096);

    std::array<char, 4096> buf;
    buf.fill('b');

    dev.read(0, buf.data());
    REQUIRE(std::all_of(begin(buf), end(buf), [](char x){ return x == 'a'; }));
}