//
// Created by fatih on 10/22/17.
//

#pragma once

#include <cstdint>
#include <fs270/contiguous_data.hpp>
namespace fs
{
/**
 * inode only data that's read and written from/to the disk
 */
struct inode_data
{
    uint32_t m_file_size;
    uint8_t m_refcnt;
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
    auto size() const
    { return m_data.m_file_size; }

    /**
     * Current capacity of the underlying contiguous file
     * Until this capacity is exhausted, no new block allocations are required
     * @return The capacity of the underlying blocks in total bytes
     */
    auto capacity() const
    { return m_blocks.get_block_count(); };

    /**
     * Number of hardlinks pointing to this inode
     * @return Number of hardlinks
     */
    auto get_hardlinks() const
    { return m_data.m_refcnt; }
};
}

