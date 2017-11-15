//
// Created by fatih on 10/22/17.
//

#include <catch.hpp>
#include <fs270/inode.hpp>
#include <fs270/mkfs.hpp>
#include <fs270/fs_instance.hpp>
#include <cstring>

TEST_CASE("inode basics", "[fs][inode]")
{
    auto blk_dev = std::make_unique<fs::ram_block_dev>(100LL * 1024 * 1024, 4096);
    fs::make_fs(*blk_dev, {});

    auto fs = fs::fs_instance::load(std::move(blk_dev));

    auto in = fs::inode::create(fs);

    REQUIRE(in.size() == 0);

    in.truncate(100);

    REQUIRE(in.size() == 100);

    in.write(0, "hello", 5);

    std::array<char, 5> buf;
    in.read(0, buf.data(), 5);

    REQUIRE(std::memcmp(buf.data(), "hello", 5) == 0);

    in.write(10000, "world", 5);

    REQUIRE(in.size() == 10005);

    in.read(10000, buf.data(), 5);
    REQUIRE(std::memcmp(buf.data(), "world", 5) == 0);

    fs::inode::write(6000, in);

    in = fs::inode::read(fs, 6000);
}