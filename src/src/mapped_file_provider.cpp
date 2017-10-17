//
// Created by fatih on 10/16/17.
//

#include <fs270/mapped_file_provider.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace fs
{
void mapped_file_provider::write(mapped_file_provider::sector_id_t id, const void *data)
{
    auto ptr = reinterpret_cast<const char*>(data);
    std::copy(ptr, ptr + m_blk_size, get_block_start(id));
}
void mapped_file_provider::read(mapped_file_provider::sector_id_t id, void *data)
{
    auto ptr = reinterpret_cast<char*>(data);
    std::copy(get_block_start(id), get_block_start(id) + m_blk_size, ptr);
}
mapped_file_provider::mapped_file_provider(const std::string &path, size_t size, uint16_t block_size)
{
    m_capacity = size;
    m_blk_size = block_size;
    auto fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ftruncate(fd, size);
    m_memory = reinterpret_cast<char*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    close(fd);
}
char *mapped_file_provider::get_block_start(mapped_file_provider::sector_id_t sector)
{
    return m_memory + sector * m_blk_size;
}
mapped_file_provider::~mapped_file_provider()
{
    munmap(m_memory, m_capacity);
}
}
