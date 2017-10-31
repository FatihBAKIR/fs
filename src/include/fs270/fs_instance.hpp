//
// Created by fatih on 10/31/17.
//

#pragma once

#include <fs270/inode.hpp>
#include "superblock.hpp"

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
        superblock m_superblk;
        config::block_dev_type m_device;

    public:

        /**
         * This function is called when the ref count of an inode reaches 0
         * The inode is then scheduled for flushing
         * @param inode inode to finalize
         */
        void inode_return(inode* inode);
    };
}

