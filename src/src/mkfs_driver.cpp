//
// Created by fatih on 10/22/17.
//

#include <fs270/mkfs.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    auto blk_dev = std::make_unique<fs::config::block_dev_type>("/tmp/fs", 10LL * 1024 * 1024 * 1024, 4096);
    auto fs = fs::make_fs(std::move(blk_dev), {});

    std::cout << "free blocks: " << fs.allocator()->get_num_free_blocks() << '\n';
    auto in = fs::inode::create(fs);
    in.write(0, "hello world", 12);
    fs::inode::write(100000, in);
    std::cout << "free blocks: " << fs.allocator()->get_num_free_blocks() << '\n';
}