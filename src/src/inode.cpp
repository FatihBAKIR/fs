//
// Created by fatih on 10/22/17.
//

#include <fs270/inode.hpp>
#include <fs270/fs_instance.hpp>
#include <fs270/bitmap_allocator.hpp>

namespace fs
{
    void inode::update_mod_time()
    {
        auto time = std::chrono::system_clock::now();
        auto u_time = std::chrono::system_clock::to_time_t(time);
        m_data.mod_time = u_time;
    }

    void inode::update_access_time()
    {
        auto time = std::chrono::system_clock::now();
        auto u_time = std::chrono::system_clock::to_time_t(time);
        m_data.access_time = u_time;
    }

    void inode::set_times(clock::time_point create, clock::time_point mod, clock::time_point access)
    {
        m_data.creat_time = clock::to_time_t(create);
        m_data.mod_time = clock::to_time_t(mod);
        m_data.access_time = clock::to_time_t(access);
    }

    auto inode::get_modification_time() const -> clock::time_point
    {
        return clock::from_time_t(m_data.mod_time);
    }

    auto inode::get_creation_time() const -> clock::time_point
    {
        return clock::from_time_t(m_data.creat_time);
    }

    auto inode::get_access_time() const -> clock::time_point
    {
        return clock::from_time_t(m_data.access_time);
    }

    void inode::set_owner(int32_t user_id)
    {
        m_data.owner = user_id;
    }

    int32_t inode::get_owner() const
    {
        return m_data.owner;
    }

    void inode::set_group(int32_t group_id)
    {
        m_data.group = group_id;
    }

    int32_t inode::get_group() const
    {
        return m_data.group;
    }

    void intrusive_ptr_add_ref(inode* n)
    {
        n->m_refcnt++;
    }

    void intrusive_ptr_release(inode* n)
    {
        n->m_refcnt--;
        if (n->m_refcnt == 0)
        {
            n->m_fs->inode_return(n);
        }
    }

    void inode::truncate(int32_t new_size)
    {
        m_data.file_size = new_size;

        auto needed_blk_count = new_size / m_fs->blk_cache()->device()->get_block_size();

        for (int i = m_blocks.get_block_count() - needed_blk_count;
             i < m_blocks.get_block_count();
             ++i)
        {
            m_fs->allocator()->free(m_blocks.get_actual_block(i), 1);
        }
    }
}