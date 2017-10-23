//
// Created by fatih on 10/16/17.
//

#pragma once

#include <array>
#include <vector>
#include <fs270/config.hpp>
#include "block_cache.hpp"

namespace fs
{
namespace detail
{
/**
 * This is the on-disk data of a contiguous file
 * Basically stores the current block count and
 * the block pointers, nothing fancy
 */
struct contiguous_data
{
    int32_t block_count;
    std::array<config::block_dev_type::sector_id_t, config::direct_pointers>
        direct_blocks;
    std::array<config::block_dev_type::sector_id_t, config::first_indirects>
        first_indirect_blocks;
    std::array<config::block_dev_type::sector_id_t, config::second_indirects>
        second_indirect_blocks;
    std::array<config::block_dev_type::sector_id_t, config::third_indirects>
        third_indirect_blocks;
};

static_assert(std::is_trivially_copyable<contiguous_data>{}, "Must be trivally copyable");

std::vector<config::sector_id_t> calc_path(config::sector_id_t, const contiguous_data&, uint16_t blksize);
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
    config::block_dev_type::sector_id_t get_actual_block(virtual_block_id id);

    /**
     * Returns the current number of blocks in the list
     * @return Number of blocks
     */
    int32_t get_block_count() const;

    /**
     * Pushes the given sector id to the back of the list
     * @param id Id to append
     */
    void push_block(config::block_dev_type::sector_id_t id);

    /**
     * Pops the last sector id from the list
     * If the list is empty, the behaviour is undefined
     */
    void pop_block();

private:
    detail::contiguous_data m_data;
    config::block_dev_type* m_device;
    block_cache* m_cache;

    cont_file(const detail::contiguous_data& data, config::block_dev_type* dev)
        : m_data(data), m_device(dev), m_cache(get_cache(*dev)) {};

    friend cont_file read_cont_file(config::block_dev_type *device, config::address_t addr);
    friend void write_cont_file(config::block_dev_type *device, config::address_t addr, const cont_file &);

    friend cont_file create_cont_file(config::block_dev_type *device);

    std::vector<config::sector_id_t> calc_path(virtual_block_id);
};

/**
 * Reads a contiguous file from the given device at the given address
 * @param device Device to read from
 * @param addr Address of the data
 * @return Contiguous file object
 */
cont_file read_cont_file(config::block_dev_type *device, config::address_t addr);

/**
 * Writes the given contiguous file object to the given device at the given address
 * @param device Device to write to
 * @param addr Address to write the object to
 */
void write_cont_file(config::block_dev_type *device, config::address_t addr, const cont_file &);

/**
 * Creates an empty contiguous file object in memory
 * @param device Device the file will belong to
 * @return Newly created contiguous file
 */
cont_file create_cont_file(config::block_dev_type *device);
}
