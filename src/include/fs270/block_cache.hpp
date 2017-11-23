//
// Created by fatih on 10/21/17.
//

#pragma once

#include <map>
#include <fs270/config.hpp>
#include <memory>
#include <fs270/block.hpp>
#include <set>
#include <thread>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_set.h>

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
     * @param zero_sector whether it's a new sector that's never been written
     * @return pointer to the block
     */
    block_ptr load(config::sector_id_t id, bool zero_sector = false);

    /**
     * Gets the underlying device
     * @return Underlying device
     */
    const config::block_dev_type* device() const { return m_device; }

    /**
     * Gets the underlying device
     * @return Underlying device
     */
    config::block_dev_type* device() { return m_device; }

    /**
     * Synchronizes the current state to the disk
     * Flushes everything in the cache
     */
    void sync();

    /**
     * Starts background thread to flush stuff
     */
    void start_thread();
private:
    config::block_dev_type* m_device;
    std::map<config::sector_id_t, block> m_cache;
    tbb::concurrent_queue<block*> m_writeback;
    tbb::concurrent_queue<config::sector_id_t> m_delq;
    std::thread m_sync;

    block_cache(config::block_dev_type& device) : m_device(&device) {}
    std::atomic<bool> m_running;

    void thread_tick();

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
