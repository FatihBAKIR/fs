//
// Created by A on 11/18/17.
//
#include <catch.hpp>
#include <fs270/inode.hpp>
#include <fs270/mkfs.hpp>
#include <fs270/fs_instance.hpp>
#include <cstring>
#include "tests_common.hpp"
#include <vector>
#include<iostream>

TEST_CASE("ilists test", "[fs][inode]")
{
  // need to test find_inode, create_inode, get_inode, remove_inode, inode_return???
  auto blk_dev = fs::tests::get_block_dev();
  auto fsi = fs::make_fs(std::move(blk_dev), {});
  std::vector<int32_t> inns; // inode numbers
  int n_ins = fsi.get_number_inodes();


  for(int i=0; i<100; i++) {
    inns.push_back(fsi.create_inode());
    REQUIRE(fsi.get_number_inodes() == n_ins+1);
    n_ins = fsi.get_number_inodes();
  }
  REQUIRE(fsi.get_number_inodes() == 100);

  for(int i=0; i<100; i++) {
    auto res = fsi.get_inode(inns[i]);
    //std::cout << res << std::endl; // what's this return if it fails?
  }

  for(int i=0; i<100; i++) {
    fsi.remove_inode(inns[i]);
  }

  // make sure ilist is empty
  REQUIRE(fsi.get_number_inodes() == 0);

}

TEST_CASE("inode_return test", "[fs][inode]")
{
  // need to test find_inode, create_inode, get_inode, remove_inode, inode_return???
  auto blk_dev = fs::tests::get_block_dev();
  auto fsi = fs::make_fs(std::move(blk_dev), {});
  std::vector<int32_t> inns; // inode numbers
  int n_ins = fsi.get_number_inodes();

  {
    inns.push_back(fsi.create_inode());
    fs::inode_ptr inp = fsi.get_inode(inns[0]);
    inp->write(0, "hello world", 11);
  }

  {
    fs::inode_ptr inp = fsi.get_inode(inns[0]);
    std::array<char, 11> buf;
    inp->read(0, buf.data(), 11);
    REQUIRE(std::memcmp(buf.data(), "hello world", 11) == 0);
  }

}