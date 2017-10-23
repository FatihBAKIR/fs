//
// Created by fatih on 10/22/17.
//

#pragma once

#include <cstdint>
#include <cstddef>
namespace fs
{
namespace detail
{
struct blk_device
{
    virtual void read(int32_t sector, void* data) = 0;
    virtual void write(int32_t sector, const void* data) = 0;

    uint16_t get_block_size() const { return m_blk_sz; }
    size_t get_capacity() const { return m_cap; }
protected:


private:

    uint16_t m_blk_sz;
    size_t m_cap;
};

template <class BlkDevT>
struct blk_dev
{

};
}
}

