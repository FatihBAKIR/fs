//
// Created by fatih on 10/29/17.
//

#include <catch.hpp>
#include <fs270/mkfs.hpp>
#include <fs270/block_cache.hpp>
#include <fs270/superblock.hpp>
#include <fs270/contiguous_data.hpp>
#include <fs270/bitmap_allocator.hpp>

TEST_CASE("mkfs basic", "[fs][mkfs]")
{
    fs::config::block_dev_type blk_dev(10 * 1024 * 1024, 4096);

    fs::make_fs(blk_dev, {});

    auto cache = get_cache(blk_dev);
    auto first_blk = cache->load(1);

    auto sb = first_blk->data<fs::superblock>();

    REQUIRE(sb->block_size == 4096);
    REQUIRE(sb->total_blocks == blk_dev.capacity() / 4096);
    REQUIRE(sb->ilist_address == sizeof(fs::superblock) + 128 + blk_dev.get_block_size());
    REQUIRE(sb->allocator_data_address == sizeof(fs::superblock) + blk_dev.get_block_size());

    auto alloc = fs::bitmap_allocator::load(get_cache(blk_dev), 2);

    auto f_blocks = alloc.alloc(5);
    auto o_block = alloc.alloc(1);
    REQUIRE(f_blocks != o_block);
    REQUIRE(o_block == f_blocks + 5);
    alloc.free(f_blocks, 5);
    REQUIRE(alloc.alloc(5) == f_blocks);
    alloc.free(f_blocks, 5);
    alloc.free(o_block, 1);

    fs::config::sector_id_t id;
    while ((id = alloc.alloc(1)) != fs::config::nullsect)
    {
        REQUIRE((id != 1 && id != 2));
    }

    //auto ilist_file = fs::read_cont_file(&blk_dev, sb->ilist_address);
    //REQUIRE(ilist_file.get_capacity() == 0);
}