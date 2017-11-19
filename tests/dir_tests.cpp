//
// Created by Mehmet Fatih BAKIR on 18/11/2017.
//

#include <fs270/directory.hpp>
#include <catch.hpp>
#include <fs270/mkfs.hpp>
#include <iostream>
#include "tests_common.hpp"

TEST_CASE("dir test", "[fs][dir]")
{
    auto dev = fs::tests::get_block_dev();
    auto fs = fs::make_fs(std::move(dev), {});

    auto innum = fs.create_inode();
    auto dir_in = fs.get_inode(innum);

    auto child = fs.create_inode();
    auto ch_in = fs.get_inode(child);

    auto dir = fs::directory(dir_in);
    dir.add_entry("hello.txt", child);

    dir.add_entry("asdasdasd.txt", fs.create_inode());

    REQUIRE(ch_in->get_hardlinks() == 1);

    for (auto ent : dir)
    {
        std::cout << ent.first << " : " << ent.second << '\n';
    }

    std::cout << " ----- \n";

    dir.del_entry("hello.txt");

    for (auto ent : dir)
    {
        std::cout << ent.first << " : " << ent.second << '\n';
    }
}