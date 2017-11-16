//
// Created by fatih on 10/22/17.
//

#include <fs270/mkfs.hpp>

int main(int argc, char** argv)
{
    auto blk_dev = std::make_unique<fs::config::block_dev_type>(10 * 1024 * 1024, 4096);
    fs::make_fs(std::move(blk_dev), {});
}