//
// Created by Mehmet Fatih BAKIR on 09/10/2017.
//

#include <fs270/in_memory_data.hpp>
#include <algorithm>

namespace fs
{
    char* in_memory_data::get_block_start(in_memory_data::sector_id_t sector)
    {
        return m_memory + sector * m_blk_size;
    }

    void in_memory_data::write(in_memory_data::sector_id_t id, const void* data)
    {
        auto ptr = reinterpret_cast<const char*>(data);
        std::copy(ptr, ptr + m_blk_size, get_block_start(id));
    }

    void in_memory_data::read(in_memory_data::sector_id_t id, void* data)
    {
        auto ptr = reinterpret_cast<char*>(data);
        std::copy(get_block_start(id), get_block_start(id) + m_blk_size, ptr);
    }

    in_memory_data::in_memory_data(size_t size, uint16_t block_size)
        : m_blk_size(block_size), m_memory(new char[size]), m_capacity(size)
    {
    }
}