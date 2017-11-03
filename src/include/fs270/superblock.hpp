//
// Created by fatih on 10/16/17.
//

#pragma once

#include <array>
#include <fs270/config.hpp>
namespace fs
{
struct superblock
{
    config::address_t allocator_data_address;
    config::address_t ilist_address;

    uint32_t total_blocks;
    uint32_t block_size;
};
}
