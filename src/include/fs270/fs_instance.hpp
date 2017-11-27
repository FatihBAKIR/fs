//
// Created by fatih on 10/31/17.
//

#pragma once

#include <fs270/inode.hpp>
#include <fs270/superblock.hpp>
#include <fs270/bitmap_allocator.hpp>
#include <memory>

namespace fs
{
    using inode_ptr = boost::intrusive_ptr<inode>;
    using const_inode_ptr = boost::intrusive_ptr<const inode>;
    /**
     * This class represents a mounted file system
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
        bitmap_allocator* allocator() { return m_alloc.get(); }

        block_cache* blk_cache() { return m_cache; }

        /**
         * Allocates an inode number
         * @return allocated inode number
         */
        int32_t create_inode();

        /**
         * Loads the inode and returns a pointer to it
         * @param inum inode number
         * @return inode pointer
         */
        inode_ptr get_inode(int32_t inum);

        /**
         * Frees the given inode number
         * @param inum inode number
         */
        void remove_inode(int32_t inum);

        /**
         * Loads the filesystem from the given block device
         * @param dev device to load the filesystem from
         * @return the loaded filesystem
         */
        static fs_instance load(std::unique_ptr<config::block_dev_type> dev);

        static std::unique_ptr<fs_instance> load_heap(std::unique_ptr<config::block_dev_type> dev);

        int get_total_blocks() const
        {
            return m_superblk.total_blocks;
        }

        int get_number_inodes() const {
            int nin;
            m_ilist->read(sizeof(int), &nin, sizeof(int)); // number of inodes
            return nin;
        }

        fs_instance(fs_instance&&) = default;
        ~fs_instance();
    private:
        std::unique_ptr<config::block_dev_type> m_device;
        superblock m_superblk;

        /**
         * Store ilist as an inode so that we can grow/shrink it easily
         */
        std::unique_ptr<inode> m_ilist;
        std::map<int32_t, inode> m_ilist_map;

        std::unique_ptr<bitmap_allocator> m_alloc;

        block_cache* m_cache;

        explicit fs_instance(std::unique_ptr<config::block_dev_type> dev);
    public:

        inode* get_ilist() {
            return m_ilist.get();
        }

        /**
         * This function is called when the ref count of an inode reaches 0
         * The inode is then scheduled for flushing
         * @param inode inode to finalize
         */
        void inode_return(const inode* in);
    };

}

