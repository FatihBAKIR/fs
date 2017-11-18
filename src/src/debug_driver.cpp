//
// Created by fatih on 11/18/17.
//

#include <fs270/fs_instance.hpp>
#include <iostream>

int main()
{
    auto dev = std::make_unique<fs::mmap_block_dev>("/tmp/fs", 10LL * 1024 * 1024 * 1024, 4096);
    auto fs = fs::fs_instance::load(std::move(dev));

    std::cout << fs.blk_cache()->device()->capacity() << '\n';
    std::cout << fs.get_total_blocks() << '\n';
    std::cout << "free blocks: " << fs.allocator()->get_num_free_blocks() << '\n';

    auto in = fs::inode::read(fs, 100000);
    std::string msg;
    msg.resize(12);
    in.read(0, &msg[0], 12);

    std::cout << msg << '\n';
}
