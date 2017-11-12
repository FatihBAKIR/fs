//
// Created by fatih on 10/31/17.
//

#include <fs270/fs_instance.hpp>
#include <fs270/fsexcept.hpp>

namespace fs
{
    inode_ptr fs_instance::find_inode(int32_t inode_id)
    {
        if (inode_id >= 0)
        {
            throw inode_not_found(inode_id);
        }

        auto inodes_per_block = m_device.get_block_size() / (sizeof(inode_data) + sizeof(cont_file));

        auto file_index = inode_id / inodes_per_block;
        auto inode_offset = inode_id % inodes_per_block;

        auto blk = m_cache->load(file_index);
        auto data = blk->data(inode_offset * (sizeof(inode_data) + sizeof(cont_file)));


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