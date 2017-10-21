//
// Created by fatih on 10/21/17.
//

#pragma once

#include <map>
#include <fs270/config.hpp>
#include <memory>
#include <fs270/block.hpp>
#include <set>

namespace fs
{
struct id_comparator
{
    struct is_transparent{};

    bool operator()(const block& a, config::sector_id_t b) const { return a.m_id < b; }
    bool operator()(config::sector_id_t b, const block& a) const { return b < a.m_id; }
    bool operator()(const block& a, const block& b) const { return a.m_id < b.m_id; }
};

class block_cache
{
    config::block_dev_type* m_device;
    std::map<config::sector_id_t, block> m_cache;

    void flush(block*);
public:
    block_cache(config::block_dev_type& device) : m_device(&device) {}

    block_ptr load(config::sector_id_t);
    void finalize(block* b);
};
}
