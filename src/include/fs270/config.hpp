//
// Created by Mehmet Fatih BAKIR on 09/10/2017.
//

#pragma once

#include "in_memory_data.hpp"
#include "bitmap_allocator.hpp"

namespace fs
{
    struct configuration
    {
        using raw_data_type =  in_memory_data;
        using allocator_type = bitmap_allocator;


    };
}
