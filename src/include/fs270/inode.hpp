//
// Created by fatih on 10/22/17.
//

#pragma once

#include <cstdint>
#include <fs270/contiguous_data.hpp>
namespace fs
{
enum class inode_flags : uint8_t
{
    directory = 1,
    symlink = 2
};

/**
 * inode only data that's read and written from/to the disk
 */
struct inode_data
{
    int32_t file_size;
    uint8_t ref_cnt;
    inode_flags flags;
    int32_t owner;
    int32_t group;
};

static_assert(std::is_trivially_copyable<inode_data>{}, "Must be trivally copyable");

/**
 * This class implements the necessary inode functionalities needed
 */
class inode
{
    inode_data m_data;
    cont_file  m_blocks;

public:

    /**
     * Size of the file in bytes
     * @return size of the file
     */
    int32_t size() const
    { return m_data.file_size; }

    /**
     * Current capacity of the underlying contiguous file
     * Until this capacity is exhausted, no new block allocations are required
     * @return The capacity of the underlying blocks in total bytes
     */
    int32_t capacity() const
    { return m_blocks.get_capacity(); };

    /**
     * Number of hardlinks pointing to this inode
     * @return Number of hardlinks
     */
    uint8_t get_hardlinks() const
    { return m_data.ref_cnt; }
};
}

