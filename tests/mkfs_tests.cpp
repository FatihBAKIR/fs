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
    auto blk_dev = std::make_unique<fs::ram_block_dev>(10 * 1024 * 1024, 4096);

    auto fs = fs::make_fs(std::move(blk_dev), {});

    auto cache = fs.blk_cache();
    auto first_blk = cache->load(1);

    auto sb = first_blk->data<fs::superblock>();
    auto dev = cache->device();

    REQUIRE(sb->block_size == 4096);
    REQUIRE(sb->total_blocks == dev->capacity() / 4096);
    REQUIRE(sb->ilist_address == sizeof(fs::superblock) + 128 + dev->get_block_size());

    auto alloc = fs::bitmap_allocator::load(cache, 2);

    auto f_blocks = alloc.alloc(5);
    auto o_block = alloc.alloc(1);
    REQUIRE(f_blocks != o_block);
    REQUIRE(o_block == f_blocks + 5);
    alloc.free(f_blocks, 5);
    REQUIRE(alloc.alloc(5) == f_blocks);
    alloc.free(f_blocks, 5);
    alloc.free(o_block, 1);

    std::set<fs::config::sector_id_t> allocd;
    allocd.insert(0);
    allocd.insert(1);
    allocd.insert(2);
    allocd.insert(dev->capacity() / dev->get_block_size() - 1);
    allocd.insert((dev->capacity() / dev->get_block_size())/2);
    fs::config::sector_id_t id;
    while ((id = alloc.alloc(1)) != fs::config::nullsect)
    {
        REQUIRE(allocd.find(id) == allocd.end());
        allocd.insert(id);
    }

    for (int i = 0; i < dev->capacity()/ dev->get_block_size(); ++i)
    {
        REQUIRE(allocd.find(i) != allocd.end());
    }

    //auto ilist_file = fs::read_cont_file(&blk_dev, sb->ilist_address);
    //REQUIRE(ilist_file.get_capacity() == 0);
}