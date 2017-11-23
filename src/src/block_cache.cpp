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
#if !defined(NDEBUG)
        if (id==0) {
            throw null_block_exception();
        }
#endif

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

        /*std::fill(blk.m_data.get(), blk.m_data.get() + 512, 0xF0);
        std::fill(blk.m_data.get() + 512 + m_device->get_block_size(),
                blk.m_data.get() + m_device->get_block_size() + 1024, 0xF0);*/

        if (!zero) {
            m_device->read(id, blk.m_data.get()+512);
        }

        auto res = block_ptr(&it->second);
        if (m_cache.size() > ((100 * 1024 * 1024) / m_device->get_block_size()))
        {
            // free up space

            int done = 0;
            while (done < 1000 && !m_delq.empty())
            {
                config::sector_id_t id = config::nullsect;
                if (!m_delq.try_pop(id))
                {
                    break;
                }
                done++;
                auto it = m_cache.find(id);
                if (it == m_cache.end()
                        || it->second.m_refcnt != 0)
                {
                    continue;
                }
                m_cache.erase(it);
            }
        }

        return res;
    }

    void block_cache::start_thread()
    {
        m_running = true;
        m_sync = std::thread([&]{
            while (m_running)
            {
                this->thread_tick();
            }
        });
    }

    void block_cache::thread_tick()
    {
        int done = 0;
        while (done < 1000 && !m_writeback.empty())
        {
            block* blk;
            if (!m_writeback.try_pop(blk))
            {
                break;
            }

            flush(blk);
            m_delq.push(blk->m_id);
            done++;
        }
    }

    void block_cache::finalize(block* b)
    {
        if (b->m_writeback)
        {
            m_writeback.push(b);
            return;
        }
        m_delq.push(b->m_id);
        //auto it = m_cache.find(b->m_id);
        /*for (int i = 0; i < 512; ++i)
        {
            if (b->m_data[i] != 0xF0 || b->m_data[512 + m_device->get_block_size() + i] != 0xF0)
            {
                std::cerr << "Block corruption in block " << b->m_id << " detected!" << '\n';
                break;
            }
        }*/
        //flush(b);
        //m_cache.erase(it);
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
        m_running = false;
        m_sync.join();
        std::cout << "ELEM COUNT: " << m_cache.size() << '\n';
        for (auto& cached : m_cache) {
            flush(&cached.second);
        }
    }
}