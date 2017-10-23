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
    config::sector_id_t allocator_data_begin;
    config::address_t ilist_address;

    uint32_t total_blocks;
    uint32_t total_inodes;
    uint32_t block_size;

    std::array<config::address_t, 3> replicate_addrs;
};
}
