//
// Created by fatih on 11/18/17.
//

#pragma once

#include <iterator>
#include <boost/utility/string_view.hpp>
#include "fs_instance.hpp"

namespace fs
{
class dir_iterator
        : std::iterator<std::forward_iterator_tag, std::pair<std::string, int>>
{
    std::pair<std::string, int> operator*() const;

    dir_iterator operator++(int) const;
    dir_iterator& operator++() const;

    bool operator==(const dir_iterator& rhs);

private:
    inode_ptr m_dir_inode;
    uint64_t m_dir_pos;
};

class directory
{
public:
    /**
     * Adds the given inode number to this directory with the given name
     * Can be called while iterating over the directory
     * @param name name of the entry
     * @param inumber inumber of the entry
     */
    void add_entry(boost::string_view name, int inumber);

    /**
     * Deletes the entry with the given name from this directory
     * If there is a directory traversal when this function is called,
     * the behaviour is undefined
     * @param name
     */
    void del_entry(boost::string_view name);

private:

    inode_ptr m_inode;
};

}

