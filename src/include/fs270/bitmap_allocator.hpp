//
// Created by Mehmet Fatih BAKIR on 09/10/2017.
//

#pragma once

#include <fs270/config.hpp>
#include <fs270/block_cache.hpp>

namespace fs
{
    class bitmap_allocator
    {
    public:
        /**
         * Creates a new bitmap allocator in the device, putting it's private data
         * at `start_sector`
         * @param cache cache for the device to be used
         * @param start_sector first block that can be used by the allocator
         * @return newly created allocator
         */
        static bitmap_allocator create(block_cache* cache, config::sector_id_t start_sector);

        /**
         * Loads an existing bitmap allocator from the device, where the private data is stored at
         * `start_sector`
         * @param cache cache for the device to be used
         * @param start_sector first block of the allocator
         * @return stored allocator
         */
        static bitmap_allocator load(block_cache* cache, config::sector_id_t start_sector);

        /**
         * Marks a block as used
         * @param id id of the block to be marked as used
         */
        void mark_used(config::sector_id_t id);

        /**
         * Allocates `num_blocks` many contiguous blocks from the disk
         * If the request cannot be fulfilled, nullsect will be returned
         * @param num_blocks number of contiguous blocks to allocate
         * @return starting block of the allocation
         */
        config::sector_id_t alloc(int num_blocks);

        /**
         * Returns `num_blocks` many blocks starting at `from` to the disk
         * to be reallocated
         * @param from starting block id
         * @param num_blocks number of contiguous blocks to free
         */
        void free(config::sector_id_t from, int num_blocks);

    private:
        bitmap_allocator(block_cache* cache, config::sector_id_t start_sector);

        void set(config::sector_id_t id, bool val);
        bool get(config::sector_id_t id);

        block_cache* m_cache;
        config::sector_id_t m_start;
    };

}
