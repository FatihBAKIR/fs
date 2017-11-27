//
// Created by fatih on 11/18/17.
//

#include <fs270/directory.hpp>
#include <tuple>
#include <fs270/fsexcept.hpp>

namespace fs
{
    void directory::add_entry(boost::string_view name, int inumber) {
        uint8_t name_len = name.size();
        m_inode->write(m_inode->size(), &name_len, sizeof name_len);
        m_inode->write(m_inode->size(), name.data(), name_len);
        m_inode->write(m_inode->size(), &inumber, sizeof inumber);

        m_inode->get_fs().get_inode(inumber)->increment_hardlinks();
    }

    uint64_t next_dirent(const inode* dir_inode, uint64_t cur_ptr)
    {
        uint8_t len;
        dir_inode->read(cur_ptr, &len, sizeof len);
        return cur_ptr + sizeof len + len + sizeof(int);
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

    int dir_iterator::get_inumber() const {
        uint8_t len;
        m_dir_inode->read(m_dir_pos, &len, sizeof len);
        int inum;
        m_dir_inode->read(m_dir_pos + sizeof len + len, &inum, sizeof inum);
        return inum;
    }

    std::string dir_iterator::get_name() const {
        uint8_t len;
        m_dir_inode->read(m_dir_pos, &len, sizeof len);
        char fname[255];
        m_dir_inode->read(m_dir_pos + sizeof len, fname, len);
        fname[len] = 0;
        return fname;
    }
    dir_iterator directory::find(boost::string_view name) const {
        return dir_iterator(m_inode.get(), find(name, true));
    }

    uint64_t directory::find(boost::string_view name, bool) const {
        uint64_t cur_ptr = 0;
        while (cur_ptr != m_inode->size())
        {
            uint8_t len;
            m_inode->read(cur_ptr, &len, sizeof len);
            if (len == name.size())
            {
                char fname[255];
                m_inode->read(cur_ptr + sizeof len, fname, len);
                fname[len] = 0;
                if (fname == name)
                {
                    break;
                }
            }
            cur_ptr = next_dirent(m_inode.get(), cur_ptr);
        }
        return cur_ptr;
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
        uint64_t cur_ptr = find(name, true);
        auto it = dir_iterator(m_inode.get(), cur_ptr);
        auto inumber = (*it).second;

        auto entry_len = sizeof(uint8_t) + name.size() + sizeof inumber;

        //found it
        char copy_buffer[4096];
        auto next = next_dirent(m_inode.get(), cur_ptr);
        while (next < m_inode->size())
        {
            auto sz = std::min<uint64_t>(4096, m_inode->size() - next);
            m_inode->read(next, copy_buffer, sz);
            m_inode->write(cur_ptr, copy_buffer, sz);
            next = next + sz;
            cur_ptr = cur_ptr + sz;
        }

        m_inode->truncate(m_inode->size() - entry_len);

        m_inode->get_fs().get_inode(inumber)->decrement_hardlinks();
    }

    directory::directory(const inode_ptr &ptr)
            : m_inode(std::move(ptr)) {
        if (m_inode->get_type() != inode_type::directory)
        {
            throw inode_type_error{};
        }
    }
}