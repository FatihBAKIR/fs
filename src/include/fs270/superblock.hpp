//
// Created by fatih on 10/16/17.
//

#pragma once

#include <array>
#include <fs270/config.hpp>
#include <tuple>
#include <cstdint>

namespace fs {
    struct superblock {
        config::address_t ilist_address;

        std::uint32_t total_blocks;
        std::uint32_t block_size;
    };

    inline bool operator==(const superblock &a, const superblock &b) {
        return std::tie(a.ilist_address, a.total_blocks, a.block_size) ==
               std::tie(b.ilist_address, b.total_blocks, b.block_size);
    }
}
