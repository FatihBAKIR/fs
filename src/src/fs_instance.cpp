//
// Created by fatih on 10/31/17.
//

#include <fs270/fs_instance.hpp>
#include <fs270/fsexcept.hpp>
#include <fs270/bitmap_allocator.hpp>

namespace fs {
//    inode_ptr fs_instance::find_inode(int32_t inode_id)
//    {
//        if (inode_id >= 0)
//        {
//            throw inode_not_found(inode_id);
//        }
//
//        auto inodes_per_block = m_device.get_block_size() / (sizeof(inode_data) + sizeof(cont_file));
//
//        auto file_index = inode_id / inodes_per_block;
//        auto inode_offset = inode_id % inodes_per_block;
//
//        auto blk = m_cache->load(file_index);
//        auto data = blk->data(inode_offset * (sizeof(inode_data) + sizeof(cont_file)));
//
//
//    }

    fs_instance::fs_instance(std::unique_ptr<config::block_dev_type> dev)
    {
        auto total_size = dev->capacity();
        auto blk_size = dev->get_block_size();
        auto total_blocks = total_size / blk_size;

        config::sector_id_t superblocks[3];

        superblocks[0] = 1;
        superblocks[1] = total_blocks / 2;
        superblocks[2] = total_blocks - 1;

        auto cache = get_cache(*dev);

        superblock replicas[3];

        replicas[0] = *(cache->load(superblocks[0])->data<const superblock>());
        replicas[1] = *(cache->load(superblocks[1])->data<const superblock>());
        replicas[2] = *(cache->load(superblocks[2])->data<const superblock>());

        superblock match {};
        if (replicas[0] == replicas[1] || replicas[0] == replicas[2])
        {
            match = replicas[0];
        }
        else
        {
            match = replicas[1];
        }

        auto alloc = std::make_unique<bitmap_allocator>(bitmap_allocator::load(cache, 2));

        m_superblk = match;
        m_alloc = std::move(alloc);
        m_cache = cache;
        m_device = std::move(dev);
        m_ilist = std::make_unique<inode>(inode::read(*this, match.ilist_address));
    }

    fs_instance fs_instance::load(std::unique_ptr<config::block_dev_type> dev) {
        return fs_instance(std::move(dev));
    }

    void fs_instance::inode_return(inode *inode) {
    }

    void fs_instance::inode_return(inode* inode) {
        return;
    }

    inode* fs_instance::create_inode() { // should this take data as an arg?
        return NULL;
    }

    void fs_instance::add_inode(inode* inode) {
        return;
    }

    void fs_instance::get_inode(int32_t inum) {
        return;
    }

    void fs_instance::remove_inode(int32_t inum) {
        return;
    }

}