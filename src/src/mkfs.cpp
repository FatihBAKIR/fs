//
// Created by fatih on 10/22/17.
//

#include <fs270/mkfs.hpp>
#include <fs270/superblock.hpp>
#include <fs270/contiguous_data.hpp>
#include <fs270/inode.hpp>
#include <fs270/bitmap_allocator.hpp>
#include <fs270/fs_instance.hpp>

namespace fs
{
fs_instance make_fs(std::unique_ptr<config::block_dev_type> dev, const fs_parameters &params)
{
    auto total_size = dev->capacity();
    auto blk_size = dev->get_block_size();
    auto total_blocks = total_size / blk_size;

    // decide on where to put the superblocks
    // decide on where to put the allocator
    // create the superblock
    // create the allocator
    // create the free list

    config::sector_id_t superblocks[3]; // redundant copies, maybe unneeded

    superblocks[0] = 1;
    superblocks[1] = total_blocks / 2;
    superblocks[2] = total_blocks - 1;

    superblock sb;
    sb.block_size = blk_size;
    sb.total_blocks = total_blocks;
    sb.ilist_address = sizeof(superblock) + 128 + blk_size;

    config::sector_id_t allocator = 1;

    auto cache = get_cache(*dev);

    auto alloc = fs::bitmap_allocator::create(cache, 2);
    alloc.mark_used(0);
    alloc.mark_used(1);
    alloc.mark_used(total_blocks / 2);
    alloc.mark_used(total_blocks - 1);

    //auto ilist_inode = create_inode(get_cache(dev));
    //write_inode(get_cache(dev), sb.ilist_address, ilist_inode);

    detail::create_raw(cache, sb.ilist_address);

    block_ptr sbs[3] = {
        cache->load(superblocks[0]),
        cache->load(superblocks[1]),
        cache->load(superblocks[2])
    };

    for (auto& sbp : sbs)
    {
        *sbp->data<superblock>() = sb;
        sbp->flush();
    }

    return fs::fs_instance::load(std::move(dev));
}
}