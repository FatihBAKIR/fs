//
// Created by fatih on 10/29/17.
//

#include <catch.hpp>
#include <fs270/mkfs.hpp>
#include <fs270/block_cache.hpp>
#include <fs270/superblock.hpp>
#include <fs270/contiguous_data.hpp>

TEST_CASE("mkfs basic", "[fs][mkfs]")
{
    fs::config::block_dev_type blk_dev(10 * 1024 * 1024, 4096);

    fs::make_fs(blk_dev, {});

    auto cache = get_cache(blk_dev);
    auto first_blk = cache->load(0);

    auto sb = first_blk->data<fs::superblock>();

    REQUIRE(sb->block_size == 4096);
    REQUIRE(sb->total_inodes == 0);
    REQUIRE(sb->total_blocks == (10 * 1024 * 1024) / 4096);
    REQUIRE(sb->ilist_address == sizeof(fs::superblock) + 128);
    REQUIRE(sb->allocator_data_address == sizeof(fs::superblock));

    auto alloc_file = fs::read_cont_file(&blk_dev, sb->allocator_data_address);

    REQUIRE(alloc_file.get_capacity() == 0);

    auto ilist_file = fs::read_cont_file(&blk_dev, sb->ilist_address);

    REQUIRE(ilist_file.get_capacity() == 0);
}