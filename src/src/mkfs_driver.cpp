//
// Created by fatih on 10/22/17.
//

#include <fs270/mkfs.hpp>
#include <iostream>
#include <fs270/directory.hpp>

int main(int argc, char** argv)
{
    auto blk_dev = std::make_unique<fs::config::block_dev_type>("/media/fatih/D9E0-4BC6/fs", 1LL * 1024 * 1024 * 1024, 4096);
    auto fs = fs::make_fs(std::move(blk_dev), {});

    std::cout << "free blocks: " << fs.allocator()->get_num_free_blocks() << '\n';
    auto root_dir = fs::directory(fs.get_inode(1));
    auto hello_file = fs.create_inode();
    root_dir.add_entry("hello", hello_file);
    auto hello_f = fs.get_inode(hello_file);
    auto wm = "Welcome to your file system";
    hello_f->write(0, wm, strlen(wm) + 1);
    std::cout << "free blocks: " << fs.allocator()->get_num_free_blocks() << '\n';

    auto inner_dir = fs.create_inode();
    root_dir.add_entry("opt", inner_dir);
    auto inner_in = fs.get_inode(inner_dir);
    inner_in->set_type(fs::inode_type::directory);
    inner_in->set_mode(0755);
}