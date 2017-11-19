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


  for(int i=0; i<100; i++) {
    inns.push_back(fsi.create_inode());
  }
  REQUIRE(fsi.get_number_inodes() == 100);

  for(int i=0; i<100; i++) {
    auto res = fsi.get_inode(inns[i]);
    std::cout << res << std::endl; // what's this return if it fails?
  }

  for(int i=0; i<100; i++) {
    fsi.remove_inode(inns[i]);
  }

  // make sure ilist is empty
  REQUIRE(fsi.get_number_inodes() == 0);

}
