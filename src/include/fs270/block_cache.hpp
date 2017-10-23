//
// Created by fatih on 10/21/17.
//

#pragma once

#include <map>
#include <fs270/config.hpp>
#include <memory>
#include <fs270/block.hpp>
#include <set>

namespace fs
{

class block_cache
{
public:
    /**
     * Loads the sector with the given id to the cache and returns
     * a pointer to the block object.
     * If the object is already in the cache, it's not loaded again
     * @param id id of the sector
     * @return pointer to the block
     */
    block_ptr load(config::sector_id_t id);

    /**
     * Gets the underlying device
     * @return Underlying device
     */
    const config::block_dev_type* device() const { return m_device; }
private:
    config::block_dev_type* m_device;
    std::map<config::sector_id_t, block> m_cache;

    block_cache(config::block_dev_type& device) : m_device(&device) {}

    friend block_cache* get_cache(config::block_dev_type&);
public:

    /**
     * Flushes the given block to the underlying device
     * @param b Block to flush
     */
    void flush(block* b);

    /**
     * Finalizes the given block and may remove it from the cache
     * @param b Block to finalize
     */
    void finalize(block* b);
};

block_cache* get_cache(config::block_dev_type& device);
}
