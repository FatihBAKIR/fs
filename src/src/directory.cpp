//
// Created by fatih on 11/18/17.
//

#include <fs270/directory.hpp>

namespace fs
{
    void directory::add_entry(boost::string_view name, int inumber) {
        uint8_t name_len = name.size();
        m_inode->write(m_inode->size() - 1, &name_len, sizeof name_len);
        m_inode->write(m_inode->size() - 1, name.data(), name_len);
        m_inode->write(m_inode->size() - 1, &inumber, sizeof inumber);

        m_inode->get_fs().get_inode(inumber)->increment_hardlinks();
    }
}