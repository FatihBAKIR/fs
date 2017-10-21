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
    if (b->m_refcnt == 1)
    {
        b->m_cache->finalize(b);
        return;
    }
    b->m_refcnt--;
}
}