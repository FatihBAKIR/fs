//
// Created by Mehmet Fatih BAKIR on 09/10/2017.
//

#include <fs270/ram_block_dev.hpp>
#include <algorithm>

namespace fs
{
char *ram_block_dev::get_block_start(ram_block_dev::sector_id_t sector)
{
    return m_memory + sector * m_blk_size;
}

void ram_block_dev::write(ram_block_dev::sector_id_t id, const void *data)
{
    auto ptr = reinterpret_cast<const char *>(data);
    std::copy(ptr, ptr + m_blk_size, get_block_start(id));
}

void ram_block_dev::read(ram_block_dev::sector_id_t id, void *data)
{
    auto ptr = reinterpret_cast<char *>(data);
    std::copy(get_block_start(id), get_block_start(id) + m_blk_size, ptr);
}

uint16_t ram_block_dev::get_block_size() const
{
    return m_blk_size;
}

ram_block_dev::ram_block_dev(size_t size, uint16_t block_size)
    : m_blk_size(block_size), m_memory(new char[size]), m_capacity(size)
{
}

size_t ram_block_dev::capacity() const
{
    return m_capacity;
}
ram_block_dev::~ram_block_dev()
{
    delete[] m_memory;
}
}