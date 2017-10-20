//
// Created by Mehmet Fatih BAKIR on 09/10/2017.
//

#pragma once

#include <fs270/fs_fwd.hpp>
#include <fs270/ram_block_dev.hpp>
#include <fs270/bitmap_allocator.hpp>

namespace fs
{
    struct config
    {
        using block_dev_type = ram_block_dev;
        using allocator_type = bitmap_allocator;

        static constexpr auto direct_pointers = 6;
        static constexpr auto first_indirects = 3;
        static constexpr auto second_indirects = 1;
        static constexpr auto third_indirects = 1;

        static inline constexpr block_dev_type::sector_id_t nullsect = 0;

        using address_t = uint32_t;

        using sector_id_t = block_dev_type::sector_id_t;
    };
}
