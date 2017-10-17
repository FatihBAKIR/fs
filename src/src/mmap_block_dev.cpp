//
// Created by fatih on 10/16/17.
//

#include <fs270/mmap_block_dev.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace fs
{
void mmap_block_dev::write(mmap_block_dev::sector_id_t id, const void *data)
{
    auto ptr = reinterpret_cast<const char *>(data);
    std::copy(ptr, ptr + m_blk_size, get_block_start(id));
}
void mmap_block_dev::read(mmap_block_dev::sector_id_t id, void *data)
{
    auto ptr = reinterpret_cast<char *>(data);
    std::copy(get_block_start(id), get_block_start(id) + m_blk_size, ptr);
}
mmap_block_dev::mmap_block_dev(const std::string &path, size_t size, uint16_t block_size)
{
    m_capacity = size;
    m_blk_size = block_size;
    auto fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ftruncate(fd, size);
    m_memory = reinterpret_cast<char *>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    close(fd);
}
char *mmap_block_dev::get_block_start(mmap_block_dev::sector_id_t sector)
{
    return m_memory + sector * m_blk_size;
}
uint16_t mmap_block_dev::get_block_size() const
{
    return m_blk_size;
}
mmap_block_dev::~mmap_block_dev()
{
    munmap(m_memory, m_capacity);
}
}
