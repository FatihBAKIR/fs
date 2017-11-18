//
// Created by fatih on 11/17/17.
//

#include "tests_common.hpp"
#include <fs270/mmap_block_dev.hpp>
#include <fs270/ram_block_dev.hpp>

namespace fs
{
    template <class T> struct type {};
    std::unique_ptr<ram_block_dev> get_blk_dev(type<ram_block_dev>)
    {
        return std::make_unique<ram_block_dev>(100LL * 1024 * 1024, 4096);
    }

    std::unique_ptr<mmap_block_dev> get_blk_dev(type<mmap_block_dev>)
    {
        return std::make_unique<mmap_block_dev>("/tmp/test_fs", 100LL * 1024 * 1024, 4096);
    }

    std::unique_ptr<config::block_dev_type> tests::get_block_dev() {
        return get_blk_dev(type<config::block_dev_type>{});
    }
}

