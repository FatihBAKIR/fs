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
        : public std::iterator<std::forward_iterator_tag, std::pair<std::string, int>>
{
public:
    std::pair<std::string, int> operator*() const;

    dir_iterator operator++(int);
    dir_iterator& operator++();

    bool operator==(const dir_iterator& rhs) const;
    bool operator!=(const dir_iterator& rhs) const;

    int get_inumber() const;
    std::string get_name() const;

    bool compare_name(boost::string_view name) const;

    friend class directory;
private:
    dir_iterator(const inode* in, uint64_t pos) :
        m_dir_inode(in), m_dir_pos(pos) {}

    const_inode_ptr m_dir_inode;
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

    dir_iterator begin() const;
    dir_iterator end() const;

    dir_iterator find(boost::string_view name) const;

    explicit directory(const inode_ptr& ptr);
private:
    uint64_t find(boost::string_view name, bool) const;

    inode_ptr m_inode{};
};

inline int lookup(fs::fs_instance& fs, boost::string_view path)
{
    auto inum = 1;
    auto cur = fs.get_inode(1); // start at root

    path = path.substr(1, path.size());

    while (!path.empty())
    {
        if (cur->get_type() != fs::inode_type::directory)
        {
            return 0;
        }

        auto next = path.find('/');
        auto curdir = fs::directory(cur);
        auto it = curdir.find(path.substr(0, next));
        if (it == curdir.end())
        {
            return 0;
        }

        inum = it.get_inumber();
        if (next == path.npos)
        {
            break;
        }
        cur = fs.get_inode(it.get_inumber());
        path = path.substr(next + 1, path.size());
    }

    return inum;
}

}

