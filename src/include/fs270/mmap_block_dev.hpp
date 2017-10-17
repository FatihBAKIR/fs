//
// Created by fatih on 10/8/17.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

namespace fs
{
class mmap_block_dev
{
    public:
        using sector_id_t = int;

        void write(sector_id_t id, const void* data);
        void read(sector_id_t id, void* data);

        uint16_t get_block_size() const;

        mmap_block_dev(const std::string& path, size_t size, uint16_t block_size);
        ~mmap_block_dev();

    private:
        char* m_memory;
        size_t m_capacity;

        uint16_t m_blk_size;

        char* get_block_start(sector_id_t sector);
};
}