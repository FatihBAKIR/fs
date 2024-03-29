//
// Created by fatih on 10/21/17.
//

#include <catch.hpp>
#include <fs270/block_cache.hpp>
#include <cstring>
#include <array>
#include "tests_common.hpp"

TEST_CASE("block cache", "[fs][cache]")
{
    auto dev = fs::tests::get_block_dev();
    auto cache = get_cache(*dev);

    std::array<uint8_t, 4096> buffer;
    buffer.fill('a');
    dev->write(3, buffer.data());

    {
        fs::block_ptr x = cache->load(3);

        {
            auto res = cache->load(3);

            res->data<char>()[0] = 'b';

            dev->read(3, buffer.data());
            REQUIRE(buffer[0] == 'a');
            REQUIRE(res->data()[0] == 'b');
        }

        REQUIRE(buffer[0] == 'a');
        REQUIRE(x->data<const char>()[0] == 'b');
    }

    dev->read(3, buffer.data());
    REQUIRE(buffer[0] == 'b');
}

TEST_CASE("auto flush", "[fs][cache]")
{
    auto dev = fs::tests::get_block_dev();
    auto cache = get_cache(*dev);

    {
        fs::block_ptr x = cache->load(1);
        std::memcpy(x->data(), "hello world", 12);
        // data is written back to the device here
    }

    {
        auto x = cache->load(1);
        REQUIRE(std::memcmp(x->data(), "hello world", 12) == 0);
    }
}

TEST_CASE("async load", "[fs][cache]")
{
    auto dev = fs::tests::get_block_dev();
    auto cache = get_cache(*dev);

    {
        auto blk = cache->load(1, true);
        std::array<char, 4096> x;
        x.fill('a');
        std::copy(x.data(), x.data() + x.size(), blk->data());
        REQUIRE(blk->data()[0] == 'a');
        x.fill('b');
        blk = cache->load(2, true);
        std::copy(x.data(), x.data() + x.size(), blk->data());
    }

    {
        auto fut = cache->load_async(1, false);
        auto fut2 = cache->load_async(2, false);
        auto blk = fut.get();
        auto blk2 = fut2.get();
        REQUIRE(blk->data<const char>()[0] == 'a');
        REQUIRE(blk2->data<const char>()[0] == 'b');
    }
}