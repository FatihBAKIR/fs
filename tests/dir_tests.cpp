//
// Created by Mehmet Fatih BAKIR on 18/11/2017.
//

#include <fs270/directory.hpp>
#include <catch.hpp>
#include <fs270/mkfs.hpp>
#include "tests_common.hpp"

TEST_CASE("dir test", "[fs][dir]")
{
    auto dev = fs::tests::get_block_dev();
    auto fs = fs::make_fs(std::move(dev), {});

    auto innum = fs.create_inode();
    auto dir_in = fs.get_inode(innum);

    

}