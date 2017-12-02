//
// Created by Mehmet Fatih BAKIR on 18/11/2017.
//

#include <fs270/directory.hpp>
#include <catch.hpp>
#include <fs270/mkfs.hpp>
#include <iostream>
#include <numeric>
#include <random>
#include <fs270/fsexcept.hpp>
#include "tests_common.hpp"

TEST_CASE("dir test", "[fs][dir]")
{
    auto dev = fs::tests::get_block_dev();
    auto fs = fs::make_fs(std::move(dev), {});

    auto innum = fs.create_inode();
    auto dir_in = fs.get_inode(innum);
    dir_in->set_type(fs::inode_type::directory);

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

    dir.add_entry("hello.txt", child);
    REQUIRE(ch_in->get_hardlinks() == 1);
    REQUIRE(dir_in->size() == asd_size + hello_sz);
    REQUIRE(std::distance(dir.begin(), dir.end()) == 2);
    it = std::find_if(dir.begin(), dir.end(), [](const auto& ent) { return ent.first == "hello.txt"; });
    REQUIRE(it != dir.end());

    dir.del_entry("hello.txt");
    REQUIRE(ch_in->get_hardlinks() == 0);
    REQUIRE(dir_in->size() == asd_size);
    REQUIRE(std::distance(dir.begin(), dir.end()) == 1);
    it = std::find_if(dir.begin(), dir.end(), [](const auto& ent) { return ent.first == "hello.txt"; });
    REQUIRE(it == dir.end());

    dir.del_entry("asdasdasd.txt");
    REQUIRE(dir_in->size() == 0);
    REQUIRE(std::distance(dir.begin(), dir.end()) == 0);
}

TEST_CASE("dir large", "[fs][dir]")
{
    auto dev = fs::tests::get_block_dev();
    auto fs = fs::make_fs(std::move(dev), {});

    auto innum = fs.create_inode();
    auto dir_in = fs.get_inode(innum);
    dir_in->set_type(fs::inode_type::directory);

    auto dir = fs::directory(dir_in);

    std::vector<int> nums(1000);
    std::iota(nums.begin(), nums.end(), 0);
    std::shuffle(nums.begin(), nums.end(), std::random_device());

    for (auto& i : nums)
    {
        auto name = "file_" + std::to_string(i) + ".txt";
        auto inum = fs.create_inode();
        auto inode = fs.get_inode(inum);
        inode->write(0, name.c_str(), name.size() + 1);
        dir.add_entry(name, inum);
    }

    REQUIRE(std::distance(dir.begin(), dir.end()) == 1000);

    std::shuffle(nums.begin(), nums.end(), std::random_device());
    for (auto& i : nums)
    {
        auto name = "file_" + std::to_string(i) + ".txt";
        auto it = dir.find(name);
        REQUIRE(it != dir.end());
        char buf[255];
        auto inode = fs.get_inode((*it).second);
        REQUIRE(inode->get_hardlinks() == 1);
        inode->read(0, buf, inode->size());
        REQUIRE(buf == name);
    }

    std::shuffle(nums.begin(), nums.end(), std::random_device());
    for (auto& i : nums)
    {
        auto name = "file_" + std::to_string(i) + ".txt";
        auto it = dir.find(name);
        REQUIRE(it != dir.end());
        dir.del_entry(name);
        it = dir.find(name);
        REQUIRE(it == dir.end());
    }
}

TEST_CASE("dir lookup", "[fs][dir]")
{
    auto dev = fs::tests::get_block_dev();
    auto fs = fs::make_fs(std::move(dev), {});

    auto dir_in = fs.get_inode(1);
    dir_in->set_type(fs::inode_type::directory);
    auto dir = fs::directory(dir_in);
    fs.get_inode(1)->set_type(fs::inode_type::directory);
    dir.add_entry("dir1", fs.create_inode());
    dir.add_entry("dir2", fs.create_inode());

    fs.get_inode(2)->set_type(fs::inode_type::directory);
    fs.get_inode(3)->set_type(fs::inode_type::directory);
    auto dir1 = fs::directory(fs.get_inode(2));
    auto dir2 = fs::directory(fs.get_inode(3));
    dir1.add_entry("dir3", fs.create_inode());
    dir2.add_entry("file", fs.create_inode());

    fs.get_inode(4)->set_type(fs::inode_type::directory);
    auto dir3 = fs::directory(fs.get_inode(4));

    auto file = fs.get_inode(5);
    file->write(0, "I'm a file", 10);

    dir3.add_entry("file", fs.create_inode());

    auto ofile = fs.get_inode(6);
    ofile->write(0, "I'm another file", 16);

    auto in = fs.get_inode(fs::lookup(fs, "/dir1/file"));
    REQUIRE(in == nullptr);

    in = fs.get_inode(fs::lookup(fs, "/dir2/file"));
    REQUIRE(in != nullptr);
    REQUIRE(in->size() == 10);
    REQUIRE(in->get_type() == fs::inode_type::regular);

    in = fs.get_inode(fs::lookup(fs, "/dir1/dir3"));
    REQUIRE(in != nullptr);
    REQUIRE(in->get_type() == fs::inode_type::directory);

    in = fs.get_inode(fs::lookup(fs, "/dir1/dir3/file"));
    REQUIRE(in != nullptr);
    REQUIRE(in->size() == 16);
    REQUIRE(in->get_type() == fs::inode_type::regular);
}

TEST_CASE("dir errors", "[fs][dir]")
{
    auto dev = fs::tests::get_block_dev();
    auto fs = fs::make_fs(std::move(dev), {});

    auto dir_in = fs.get_inode(1);
    dir_in->set_type(fs::inode_type::directory);
    auto dir = fs::directory(dir_in);

    std::string n;
    for (int i = 0; i < 300; ++i)
    {
        n += 'x';
    }
    REQUIRE_THROWS_AS(dir.add_entry(n, fs.create_inode()), fs::name_too_long);
}