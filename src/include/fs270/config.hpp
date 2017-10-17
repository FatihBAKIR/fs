//
// Created by Mehmet Fatih BAKIR on 09/10/2017.
//

#pragma once

#include "ram_block_dev.hpp"
#include "bitmap_allocator.hpp"

namespace fs
{
    struct configuration
    {
        using raw_data_type =  ram_block_dev;
        using allocator_type = bitmap_allocator;


    };
}
