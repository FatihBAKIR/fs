//
// Created by fatih on 10/21/17.
//

#include <fs270/block.hpp>
#include <fs270/block_cache.hpp>

namespace fs
{
void intrusive_ptr_add_ref(block *b)
{
    b->m_refcnt++;
}
void intrusive_ptr_release(block *b)
{
    auto prev = b->m_refcnt.fetch_sub(1, std::memory_order_relaxed);
    if (prev == 1)
    {
        b->m_cache->finalize(b);
    }
}

void block::flush()
{
    if (m_writeback)
    {
        m_cache->flush(this);
    }
}

}