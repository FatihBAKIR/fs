//
// Created by fatih on 10/21/17.
//

#include <fs270/block_cache.hpp>
#include <fs270/block.hpp>

namespace fs
{
block_ptr block_cache::load(config::sector_id_t id)
{
    auto it = m_cache.find(id);
    if (it != m_cache.end())
    {
        return block_ptr( &it->second );
    }

    it = m_cache.emplace(id, block{}).first;
    auto& blk = it->second;
    blk.m_id = id;
    blk.m_cache = this;
    blk.m_refcnt = 0;
    blk.m_writeback = false;
    blk.m_data = std::make_unique<uint8_t[]>(m_device->get_block_size());
    m_device->read(id, blk.m_data.get());

    return block_ptr(&it->second);
}

void block_cache::finalize(block *b)
{
    auto it = m_cache.find(b->m_id);
    flush(b);
    m_cache.erase(it);
}

void block_cache::flush(block* b)
{
    if (!b->m_writeback) return;
    m_device->write(b->m_id, b->m_data.get());
}

block_cache *get_cache(config::block_dev_type &device)
{
    static block_cache cache(device);
    return &cache;
}
}