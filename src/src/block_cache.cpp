//
// Created by fatih on 10/21/17.
//

#include <fs270/block_cache.hpp>
#include <fs270/block.hpp>
#include <iostream>
#include <fs270/fsexcept.hpp>

namespace fs {
    block_ptr block_cache::load(config::sector_id_t id, bool zero)
    {
        if (id==0) {
            throw null_block_exception();
        }

        auto it = m_cache.find(id);
        if (it!=m_cache.end()) {
            return block_ptr(&it->second);
        }

        it = m_cache.emplace(id, block{}).first;
        auto& blk = it->second;
        blk.m_id = id;
        blk.m_cache = this;
        blk.m_refcnt = 0;
        blk.m_writeback = false;
        blk.m_data = std::make_unique<uint8_t[]>(m_device->get_block_size()+1024);

        if (!zero) {
            m_device->read(id, blk.m_data.get()+512);
        }

        auto res = block_ptr(&it->second);
        return res;
    }

    void block_cache::finalize(block* b)
    {
        auto it = m_cache.find(b->m_id);
        flush(b);
        m_cache.erase(it);
    }

    void block_cache::flush(block* b)
    {
        if (!b->m_writeback) return;
        m_device->write(b->m_id, b->m_data.get()+512);
        b->m_writeback = false;
    }

    static std::map<uintptr_t, block_cache*> caches;

    block_cache* get_cache(config::block_dev_type& device)
    {
        auto it = caches.find(reinterpret_cast<uintptr_t>(&device));
        if (it!=caches.end()) {
            return it->second;
        }
        it = caches.emplace(reinterpret_cast<uintptr_t>(&device), new block_cache(device)).first;
        return it->second;
    }

    void block_cache::sync()
    {
        for (auto& cached : m_cache) {
            flush(&cached.second);
        }
    }
}