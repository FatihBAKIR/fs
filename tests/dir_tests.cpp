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
    auto hello_sz = dir_in->size();

    dir.add_entry("asdasdasd.txt", fs.create_inode());
    auto asd_size = dir_in->size() - hello_sz;

    REQUIRE(ch_in->get_hardlinks() == 1);

    REQUIRE(std::distance(dir.begin(), dir.end()) == 2);

    auto it = std::find_if(dir.begin(), dir.end(), [](const auto& ent) { return ent.first == "hello.txt"; });
    REQUIRE(it != dir.end());
    it = std::find_if(dir.begin(), dir.end(), [](const auto& ent) { return ent.first == "asdasdasd.txt"; });
    REQUIRE(it != dir.end());
    it = std::find_if(dir.begin(), dir.end(), [](const auto& ent) { return ent.first == "hello.png"; });
    REQUIRE(it == dir.end());

    REQUIRE(ch_in->get_hardlinks() == 1);
    dir.del_entry("hello.txt");
    REQUIRE(ch_in->get_hardlinks() == 0);

    REQUIRE(dir_in->size() == asd_size);
    REQUIRE(std::distance(dir.begin(), dir.end()) == 1);
    it = std::find_if(dir.begin(), dir.end(), [](const auto& ent) { return ent.first == "hello.txt"; });
    REQUIRE(it == dir.end());
}

TEST_CASE("dir large", "[fs][dir]")
{
    auto dev = fs::tests::get_block_dev();
    auto fs = fs::make_fs(std::move(dev), {});

    auto innum = fs.create_inode();
    auto dir_in = fs.get_inode(innum);

    auto dir = fs::directory(dir_in);

    for (int i = 0; i < 1000; ++i)
    {
        auto name = "file_" + std::to_string(i) + ".txt";
        auto inum = fs.create_inode();
        auto inode = fs.get_inode(inum);
        inode->write(0, name.c_str(), name.size() + 1);
        dir.add_entry(name, inum);
    }

    REQUIRE(std::distance(dir.begin(), dir.end()) == 1000);

    for (int i = 0; i < 1000; ++i)
    {
        auto name = "file_" + std::to_string(i) + ".txt";
        auto it = std::find_if(dir.begin(), dir.end(), [&name](const auto& ent) { return ent.first == name; });
        REQUIRE(it != dir.end());
        char buf[255];
        auto inode = fs.get_inode((*it).second);
        REQUIRE(inode->get_hardlinks() == 1);
        inode->read(0, buf, inode->size());
        REQUIRE(buf == name);
    }
}