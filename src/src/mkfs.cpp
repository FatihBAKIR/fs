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
void make_fs(config::block_dev_type& dev, const fs_parameters &params)
{
    auto total_size = dev.capacity();
    auto blk_size = dev.get_block_size();
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

    auto cache = get_cache(dev);

    auto alloc = fs::bitmap_allocator::create(cache, 2);
    alloc.mark_used(0);
    alloc.mark_used(1);
    alloc.mark_used(total_blocks / 2);
    alloc.mark_used(total_blocks - 1);

    // create ilist
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

    // write iblock0 to ilist, 0 for "next inode" since ilist is empty, and NULL for free pointer since there
    // is no fragmenation yet, hence no free list
    auto fs = fs::fs_instance::load(std::move(dev));
    auto ilist_inode = inode::read(fs, sb.ilist_address);
    int32_t buf[fs::inode_size/sizeof(int32_t)];
    buf[0] = 0;
    buf[1] = 0;
    ilist_inode.write(0, buf, fs::inode_size);

}
}