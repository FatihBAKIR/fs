//
// Created by fatih on 10/21/17.
//

#include <fs270/block_cache.hpp>
#include <fs270/block.hpp>
#include <iostream>
#include <fs270/fsexcept.hpp>

namespace fs
{
block_ptr block_cache::load(config::sector_id_t id)
{
#if !defined(NDEBUG)
    if (id == 0)
    {
        throw null_block_exception();
    }
#endif

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
    blk.m_data = std::make_unique<uint8_t[]>(m_device->get_block_size() + 1024);
    std::fill(blk.m_data.get(), blk.m_data.get() + 512, 0xF0);
    std::fill(blk.m_data.get() + 512 + m_device->get_block_size(),
            blk.m_data.get() + m_device->get_block_size() + 1024, 0xF0);
    m_device->read(id, blk.m_data.get() + 512);

    return block_ptr(&it->second);
}

void block_cache::finalize(block *b)
{
    auto it = m_cache.find(b->m_id);
    /*for (int i = 0; i < 512; ++i)
    {
        if (b->m_data[i] != 0xF0 || b->m_data[512 + m_device->get_block_size() + i] != 0xF0)
        {
            std::cerr << "Block corruption in block " << b->m_id << " detected!" << '\n';
            break;
        }
    }*/
    flush(b);
    m_cache.erase(it);
}

void block_cache::flush(block* b)
{
    if (!b->m_writeback) return;
    m_device->write(b->m_id, b->m_data.get() + 512);
    b->m_writeback = false;
}

static std::map<uintptr_t, block_cache> caches;
block_cache *get_cache(config::block_dev_type &device)
{
    auto it = caches.find(reinterpret_cast<uintptr_t>(&device));
    if (it != caches.end())
    {
        return &it->second;
    }
    it = caches.emplace(reinterpret_cast<uintptr_t>(&device), block_cache(device)).first;
    return &it->second;
}

    void block_cache::sync()
    {
        for (auto& cached : m_cache)
        {
            flush(&cached.second);
        }
    }
}