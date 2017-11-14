//
// Created by fatih on 10/31/17.
//

#pragma once

#include <fs270/inode.hpp>
#include <fs270/superblock.hpp>

namespace fs
{
    using inode_ptr = boost::intrusive_ptr<inode>;
    /**
     * This class represents a mount
     */
    class fs_instance
    {
    public:
        /**
         * Traverses the ilist file to find the inode requested
         * @param inode_id id of the inode to load
         * @return shared pointer to the inode
         */
        inode_ptr find_inode(int32_t inode_id);

        /**
         * Returns the allocator for this file system
         * @return the allocator
         */
        bitmap_allocator* allocator() { return m_alloc; }

        block_cache* blk_cache() { return m_cache; }


        void put_inode(const inode& in, int index);

    private:
        //TODO: ADD ALLOCATOR
        superblock m_superblk;

        /**
         * Store ilist as an inode so that we can grow/shrink it easily
         */
        inode m_ilist;

        bitmap_allocator* m_alloc;
        config::block_dev_type m_device;
        block_cache* m_cache;

    public:

        /**
         * This function is called when the ref count of an inode reaches 0
         * The inode is then scheduled for flushing
         * @param inode inode to finalize
         */
        void inode_return(inode* inode);
    };
}

