//
// Created by fatih on 11/4/17.
//

#include <fs270/bitmap_allocator.hpp>
#include <cstring>
#include <cmath>
#include <fs270/fsexcept.hpp>

namespace fs {
    bitmap_allocator::bitmap_allocator(block_cache* cache, config::sector_id_t start_sector)
            :m_cache(cache), m_meta_sector(start_sector), m_start(start_sector + 1)
    {
        m_persist = *cache->load(start_sector)->data<detail::bm_data>();
    }

    bitmap_allocator::~bitmap_allocator() {
        auto blk = m_cache->load(m_meta_sector);
        *blk->data<detail::bm_data>() = m_persist;
    }

    bool bitmap_allocator::get(config::sector_id_t id)
    {
        auto bits_per_block = m_cache->device()->get_block_size()*8;
        auto blk_id = id/bits_per_block;
        auto bit_offset = id%bits_per_block;
        auto word_offset = bit_offset/64;
        auto bit_in_word = bit_offset%64;

        auto blk = m_cache->load(m_start + blk_id);
        auto buffer = blk->data<uint64_t>();
        auto& word = buffer[word_offset];

        return bool(word & (1LL << bit_in_word));
    }

    void bitmap_allocator::set(config::sector_id_t id, bool val)
    {
        auto bits_per_block = m_cache->device()->get_block_size()*8;
        auto blk_id = id/bits_per_block;
        auto bit_offset = id%bits_per_block;
        auto word_offset = bit_offset/64;
        auto bit_in_word = bit_offset%64;

        auto blk = m_cache->load(m_start + blk_id);
        auto buffer = blk->data<uint64_t>();
        auto& word = buffer[word_offset];

        if (val) {
            word |= (1LL << bit_in_word);
        }
        else {
            word &= ~(1LL << bit_in_word);
        }
    }

    void bitmap_allocator::mark_used(config::sector_id_t id)
    {
        set(id, true);
    }

    bitmap_allocator bitmap_allocator::create(block_cache* cache, config::sector_id_t start_sector)
    {
        auto dev = cache->device();
        auto total_blks = dev->capacity()/dev->get_block_size();

        start_sector = start_sector + 1;

        detail::bm_data data;
        data.last_alloc = 0;
        data.free_blocks = total_blks;

        auto blk = cache->load(start_sector - 1);
        *blk->data<detail::bm_data>() = data;

        // need total_blks many bits
        auto total_bytes = total_blks/8;

        // we need this many blocks to store the whole bitmap
        auto bitmap_block_count = int(std::ceil(float(total_bytes)/dev->get_block_size()));

        // set each block to have only 0s, so all blocks are available
        for (auto i = 0; i<bitmap_block_count; ++i) {
            auto blk = cache->load(i+start_sector);
            std::memset(blk->data(), 0, dev->get_block_size());
        }

        // create the actual object
        bitmap_allocator alloc(cache, start_sector - 1);

        alloc.mark_used(start_sector - 1);
        // mark the bitmap blocks as used so they are not allocated
        for (auto i = 0; i < bitmap_block_count; ++i)
        {
            alloc.mark_used(i + start_sector);
        }

        return alloc;
    }

    bitmap_allocator bitmap_allocator::load(block_cache* cache, config::sector_id_t start_sector)
    {
        // do any sanity checks here
        return bitmap_allocator(cache, start_sector);
    }

    config::sector_id_t bitmap_allocator::alloc(int num_blocks)
    {
        // this is extremely inefficient, optimize somehow
        config::sector_id_t begin = m_persist.last_alloc;
        int len = 0;
        auto total = m_cache->device()->capacity() / m_cache->device()->get_block_size();
        while (len < num_blocks)
        {
            if (begin + len > total)
            {
                return fs::config::nullsect;
            }
            if (!get(begin + len))
            {
                ++len;
            }
            else
            {
                ++begin;
                len = 0;
            }
        }
        for (int i = 0; i < len; ++i)
        {
            mark_used(i + begin);
        }
        m_persist.last_alloc = begin + len;
        m_persist.free_blocks -= len;
        return begin;
    }

    void bitmap_allocator::free(config::sector_id_t from, int num_blocks)
    {
        // this can be optimized
        for (int i = 0; i < num_blocks; ++i)
        {
            if (get(i + from) == false)
            {
                throw fs::double_free{};
            }
            set(i + from, false);
        }
        m_persist.free_blocks += num_blocks;
        m_persist.last_alloc = std::min(from, m_persist.last_alloc);
    }
}