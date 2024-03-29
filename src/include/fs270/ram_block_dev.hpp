//
// Created by Mehmet Fatih BAKIR on 09/10/2017.
//

#pragma once

#include <cstddef>
#include <cstdint>

namespace fs
{
class ram_block_dev
{
public:
    using sector_id_t = int;

    void write(sector_id_t id, const void *data);

    void read(sector_id_t id, void *data);

    uint16_t get_block_size() const;
    size_t capacity() const;

    ram_block_dev(size_t size, uint16_t block_size);
    ~ram_block_dev();

private:
    char *m_memory;
    size_t m_capacity;

    uint16_t m_blk_size;

    char *get_block_start(sector_id_t sector);
};
}


