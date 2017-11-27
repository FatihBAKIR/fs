//
// Created by fatih on 10/16/17.
//

#pragma once

#include <array>
#include <vector>
#include <fs270/config.hpp>
#include "block_cache.hpp"
#include <boost/container/static_vector.hpp>

namespace fs
{
namespace detail
{
/**
 * This is the on-disk data of a contiguous file
 * Basically stores the current block count and
 * the block pointers, nothing fancy
 */
struct alignas(64) contiguous_data
{
    int32_t block_count;
    int32_t pushable_count;
    int16_t indirect_count;
    std::array<config::block_dev_type::sector_id_t, config::direct_pointers>
        direct_blocks;
    std::array<config::block_dev_type::sector_id_t, config::first_indirects>
        first_indirect_blocks;
    std::array<config::block_dev_type::sector_id_t, config::second_indirects>
        second_indirect_blocks;
    std::array<config::block_dev_type::sector_id_t, config::third_indirects>
        third_indirect_blocks;
    char pad[8];
};

static_assert(std::is_trivially_copyable<contiguous_data>{}, "Must be trivally copyable");
static_assert(sizeof(contiguous_data) == 64, "Contiguous data size is incorrect!");

boost::container::static_vector<config::sector_id_t, 3>
calc_path(config::sector_id_t, const contiguous_data&, uint16_t blksize);
}

/**
 * This class represents a contiguous sequence of bytes on the disk
 * Physical blocks of the file could be separated on disk, but through
 * this interface, the file seems to have all blocks from [0, block_count) blocks
 */
class cont_file
{
public:
    using virtual_block_id = int;

    /**
     * Translates the file block index to an actual sector id on device
     * @param id File block index
     * @return Device sector index
     */
    config::block_dev_type::sector_id_t get_actual_block(virtual_block_id id) const;

    /**
     * Returns the current number of blocks in the list
     * @return Number of blocks
     */
    int32_t get_block_count() const;

    /**
     * Calculates the number of blocks that can be pushed to the list
     * without requiring an indirect block allocation
     * @return number of blocks that can be pushed
     */
    int32_t get_pushable_count() const;

    /**
     * Pushes the given sector id to the back of the list
     * @param id id to append
     * @return whether the push required an indirect block allocation
     */
    bool push_block(config::block_dev_type::sector_id_t id);

    /**
     * Contiguous file uses the given block as an indirect block
     * @param id block for the indirect block
     */
    void push_indirect_block(config::block_dev_type::sector_id_t id);

    /**
     * Removes an unused indirect block
     * @return id of the indirect block, nullsect if none available
     */
    config::sector_id_t pop_indirect_block();

    int get_indirect_count() const;

    /**
     * Pops the last sector id from the list
     * If the list is empty, the behaviour is undefined
     */
    void pop_block();

    /**
     * Total number of bytes this file can store
     * @return Number of bytes
     */
    int32_t get_capacity() const;

private:
    detail::contiguous_data m_data;
    block_cache* m_cache;

    cont_file(const detail::contiguous_data& data, block_cache* cache)
        : m_data(data), m_cache(cache) {};

    friend cont_file read_cont_file(block_cache* device, config::address_t addr);
    friend void write_cont_file(block_cache* device, config::address_t addr, const cont_file &);

    friend cont_file create_cont_file(block_cache* device);

    boost::container::static_vector<config::sector_id_t, 3>
    calc_path(virtual_block_id) const;
};

/**
 * Reads a contiguous file from the given device at the given address
 * @param device Device to read from
 * @param addr Address of the data
 * @return Contiguous file object
 */
cont_file read_cont_file(block_cache* device, config::address_t addr);

/**
 * Writes the given contiguous file object to the given device at the given address
 * @param device Device to write to
 * @param addr Address to write the object to
 */
void write_cont_file(block_cache* device, config::address_t addr, const cont_file &);

/**
 * Creates an empty contiguous file object in memory
 * @param device Device the file will belong to
 * @return Newly created contiguous file
 */
cont_file create_cont_file(block_cache* device);
}

