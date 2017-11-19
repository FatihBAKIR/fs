//
// Created by fatih on 11/18/17.
//

#include <fs270/directory.hpp>
#include <tuple>

namespace fs
{
    void directory::add_entry(boost::string_view name, int inumber) {
        uint8_t name_len = name.size();
        m_inode->write(m_inode->size(), &name_len, sizeof name_len);
        m_inode->write(m_inode->size(), name.data(), name_len);
        m_inode->write(m_inode->size(), &inumber, sizeof inumber);

        m_inode->get_fs().get_inode(inumber)->increment_hardlinks();
    }

    int next_dirent(const inode* dir_inode, int cur_ptr)
    {
        uint8_t len;
        dir_inode->read(cur_ptr, &len, sizeof len);
        return cur_ptr + sizeof len + len + sizeof cur_ptr;
    }

    dir_iterator dir_iterator::operator++(int)
    {
        auto copy = *this;
        ++(*this);
        return copy;
    }

    dir_iterator directory::begin() const
    {
        return { m_inode.get(), 0 };
    }

    dir_iterator directory::end() const
    {
        return { m_inode.get(), (uint64_t)m_inode->size() };
    }

    std::pair<std::string, int> dir_iterator::operator*() const
    {
        uint8_t len;
        m_dir_inode->read(m_dir_pos, &len, sizeof len);
        char fname[255];
        m_dir_inode->read(m_dir_pos + sizeof len, fname, len);
        fname[len] = 0;
        int inum;
        m_dir_inode->read(m_dir_pos + sizeof len + len, &inum, sizeof inum);
        return { std::string(fname), inum };
    }

    dir_iterator& dir_iterator::operator++()
    {
        m_dir_pos = next_dirent(m_dir_inode.get(), m_dir_pos);
        return *this;
    }

    bool dir_iterator::operator==(const dir_iterator& rhs) const
    {
        return m_dir_inode.get() == rhs.m_dir_inode.get() &&
                m_dir_pos == rhs.m_dir_pos;
    }

    bool dir_iterator::operator!=(const dir_iterator& rhs) const
    {
        return !(*this == rhs);
    }

    void directory::del_entry(boost::string_view name)
    {
        auto cur_ptr = 0;
        uint8_t len;
        m_inode->read(cur_ptr, &len, sizeof len);
        if (len == name.size())
        {
            char fname[255];
            m_inode->read(cur_ptr + sizeof len, fname, len);
            fname[len] = 0;
            if (name == fname)
            {
                auto entry_len = sizeof len + len + sizeof cur_ptr;
                //found it
                char copy_buffer[4096];
                auto next = next_dirent(m_inode.get(), cur_ptr);

                while (next < m_inode->size())
                {
                    auto sz = std::min(4096, m_inode->size() - next);
                    m_inode->read(next, copy_buffer, sz);
                    m_inode->write(cur_ptr, copy_buffer, sz);
                    next = next + sz;
                    cur_ptr = cur_ptr + sz;
                }

                m_inode->truncate(m_inode->size() - entry_len);
            }
            else
            {
                cur_ptr = next_dirent(m_inode.get(), cur_ptr);
            }
        }
        else
        {
            cur_ptr = next_dirent(m_inode.get(), cur_ptr);
        }
    }
}