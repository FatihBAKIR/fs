//
// Created by Chani Jindal on 11/18/17.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

namespace fs
{
    class disk_block_dev
    {
    public:
        using sector_id_t = int;

        int write(sector_id_t id, const void *data) noexcept;

        int read(sector_id_t id, void *data) noexcept;

        uint16_t get_block_size() const noexcept;
        size_t capacity() const noexcept
        { return m_capacity; }

        disk_block_dev(const std::string &path, size_t size, uint16_t block_size);
        ~disk_block_dev();

    private:
        size_t m_capacity;
        int fd;
        uint16_t m_blk_size;

        uint64_t get_block_offset(sector_id_t sector) noexcept;
    };
}