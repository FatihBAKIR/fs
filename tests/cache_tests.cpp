//
// Created by fatih on 10/21/17.
//

#include <catch.hpp>
#include <fs270/block_cache.hpp>
#include <cstring>
#include <array>

TEST_CASE("block cache", "[fs][cache]")
{
    fs::ram_block_dev dev(10 * 1024 * 1024, 4096);
    auto cache = get_cache(dev);

    std::array<uint8_t, 4096> buffer;
    buffer.fill('a');
    dev.write(3, buffer.data());

    {
        auto x = cache->load(3);

        {
            auto res = cache->load(3);

            res->data<char>()[0] = 'b';

            dev.read(3, buffer.data());
            REQUIRE(buffer[0] == 'a');
            REQUIRE(res->data()[0] == 'b');
        }

        REQUIRE(buffer[0] == 'a');
        REQUIRE(x->data<const char>()[0] == 'b');
    }

    dev.read(3, buffer.data());
    REQUIRE(buffer[0] == 'b');
}

TEST_CASE("auto flush", "[fs][cache]")
{
    fs::ram_block_dev dev(10 * 1024 * 1024, 4096);
    auto cache = get_cache(dev);

    {
        fs::block_ptr x = cache->load(0);
        std::memcpy(x->data(), "hello world", 12);
        // data is written back to the device here
    }

    {
        auto x = cache->load(0);
        REQUIRE(std::memcmp(x->data(), "hello world", 12) == 0);
    }
}