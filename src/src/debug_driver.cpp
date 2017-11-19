//
// Created by fatih on 11/18/17.
//

#include <fs270/fs_instance.hpp>
#include <iostream>
#include <fs270/directory.hpp>

int main()
{
    auto dev = std::make_unique<fs::mmap_block_dev>("/tmp/fs", 10LL * 1024 * 1024 * 1024, 4096);
    auto fs = fs::fs_instance::load(std::move(dev));

    std::cout << fs.blk_cache()->device()->capacity() << '\n';
    std::cout << fs.get_total_blocks() << '\n';
    std::cout << "free blocks: " << fs.allocator()->get_num_free_blocks() << '\n';
    auto root_dir = fs::directory(fs.get_inode(1));
    auto hello = fs.get_inode((*root_dir.find("hello")).second);
    char buf[256];
    hello->read(0, buf, hello->size());
    std::cout << buf << '\n';
}
