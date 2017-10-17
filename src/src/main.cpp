#include <iostream>

#include <fs270/ram_block_dev.hpp>
#include <array>
#include <cassert>
#include <fs270/mmap_block_dev.hpp>

int main() {
    fs::mmap_block_dev data("/tmp/test", 1024 * 1024, 16);

    std::array<char, 16> buffer;
    buffer.fill('a');
    //data.write(3, buffer.data());

    std::array<char, 16> other_buffer;
    other_buffer.fill('b');
    data.read(3, other_buffer.data());

    assert(buffer == other_buffer);

    data.read(4, other_buffer.data());

    assert(buffer != other_buffer);
}
