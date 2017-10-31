//
// Created by fatih on 10/22/17.
//

#pragma once

#include <cstdint>
#include <fs270/contiguous_data.hpp>
#include <chrono>

namespace fs
{
enum class inode_flags: uint8_t
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
    uint8_t permissions;
    int32_t owner;
    int32_t group;
    time_t mod_time;
    time_t creat_time;
    time_t access_time;
};

static_assert(std::is_trivially_copyable<inode_data>{}, "Must be trivally copyable");

class fs_instance;

/**
 * This class implements the necessary inode functionalities needed
 */
class inode
{
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

    /**
     * Increment the number of directory entries pointing to this inode
     */
    void increment_hardlinks()
    { m_data.ref_cnt++; }

    /**
     * Decrement the number of directory entries pointing to this inode
     */
    void decrement_hardlinks()
    { m_data.ref_cnt--; }

    /**
     * Sets the owner user id of this file
     * @param user_id id of the owner
     */
    void set_owner(int32_t user_id);

    /**
     * Returns the id of the owner user
     * @return owner user's id
     */
    int32_t get_owner() const;

    /**
     * Sets the group id of this file
     * @param group_id group id
     */
    void set_group(int32_t group_id);

    /**
     * Returns the group id of this file
     * @return group id
     */
    int32_t get_group() const;

    /**
     * Sets the size for the file of this inode
     * If the size is greater than the current size, the gap will be zero filled
     * @param new_size size to set
     */
    void truncate(int32_t new_size);

    /**
     * Reads `len` bytes starting at `from` to the buffer pointed by `buf`
     * @param from start index on file
     * @param buf buffer to copy into
     * @param len length of the buffer
     * @return number of bytes read
     */
    int32_t read(uint32_t from, void *buf, int32_t len);

    /**
     * Writes `len` bytes starting at `from` from the buffer pointed by `buf`
     * @param from start index on file
     * @param buf buffer to copy from
     * @param len length of the buffer
     */
    void write(uint32_t from, const void *buf, int32_t len);

    using clock = std::chrono::system_clock;

    /**
     * Returns the time of the last modification to the file related with this inode
     * @return modification time
     */
    clock::time_point get_modification_time() const;

    /**
     * Returns the time of the creation of the file related with this inode
     * @return creation time
     */
    clock::time_point get_creation_time() const;

    /**
     * Returns the time of the latest access to the file related with this inode
     * @return access time
     */
    clock::time_point get_access_time() const;

private:
    //TODO: add allocator here
    inode_data m_data;
    cont_file m_blocks;
    fs_instance* m_fs;
    int m_refcnt;

    /**
     * Sets the latest update time of this inode to the current time
     */
    void update_mod_time();

    /**
     * Sets the access time of this inode to the current time
     */
    void update_access_time();

    /**
     * Sets the times of this inode
     * @param create creation time
     * @param mod modification time
     * @param access access time
     */
    void set_times(clock::time_point create, clock::time_point mod, clock::time_point access);

    friend void intrusive_ptr_add_ref(inode* n);
    friend void intrusive_ptr_release(inode* n);
};

/**
 * Reads an inode at the given address from the given device
 * @param cache block cache for the device
 * @param at address of the inode in device
 * @return inode at the address
 */
inode read_inode(block_cache *cache, config::address_t at);

/**
 * Writes the given inode to the device at the given address
 * @param cache block cache for the device
 * @param at address in device to write the inode to
 * @param inode inode to write to
 */
void write_inode(block_cache *cache, config::address_t at, const inode &inode);

/**
 * Creates an empty inode associated with the given device
 * @param cache block cache for the device
 * @return empty inode
 */
inode create_inode(block_cache *cache);
}

