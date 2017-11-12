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

    private:
        //TODO: ADD ALLOCATOR
        superblock m_superblk;

        /**
         * Store ilist as an inode so that we can grow/shrink it easily
         */
        inode m_ilist;

        config::block_dev_type m_device;
        block_cache* m_cache;

    public:

        /**
         * This function is called when the ref count of an inode reaches 0
         * The inode is then scheduled for flushing
         * @param inode inode to finalize
         */
        void inode_return(inode* inode);

        inode* create_inode(); // should this take data as an arg?

        void add_inode(inode* inode);

        void get_inode(int32_t inum);

        void remove_inode(int32_t inum);
    };
}

