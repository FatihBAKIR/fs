//
// Created by fatih on 11/4/17.
//

#include <catch.hpp>
#include <fs270/bitmap_allocator.hpp>
#include "tests_common.hpp"

TEST_CASE("bitmap allocator", "[fs][bitmap_alloc]")
{
    auto dev = fs::tests::get_block_dev();
    auto cache = get_cache(*dev);
    auto alloc = fs::bitmap_allocator::create(cache, 1);
    alloc.mark_used(0);

    auto begin = 1;

    REQUIRE(alloc.alloc(1) == begin + 1);
    REQUIRE(alloc.alloc(1) == begin + 2);
    REQUIRE(alloc.alloc(2) == begin + 3);
    REQUIRE(alloc.alloc(1) == begin + 5);

    alloc.free(begin + 3, 2);

    REQUIRE(alloc.alloc(2) == begin + 3);
}

TEST_CASE("bitmap alloc large", "[fs][bitmap_alloc]")
{

}