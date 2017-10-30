//
// Created by fatih on 10/22/17.
//

#include <fs270/mkfs.hpp>

int main(int argc, char** argv)
{
    fs::config::block_dev_type blk_dev(10 * 1024 * 1024, 4096);
    fs::make_fs(blk_dev, {});
}