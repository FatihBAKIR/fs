//
// Created by fatih on 10/16/17.
//

#pragma once

#include <array>
#include <fs270/config.hpp>

namespace fs
{
namespace detail
{
struct contiguous_data
{
    uint32_t block_count;
    std::array<config::block_dev_type::sector_id_t, config::direct_pointers>
        direct_blocks;
    std::array<config::block_dev_type::sector_id_t, config::first_indirects>
        first_indirect_blocks;
    std::array<config::block_dev_type::sector_id_t, config::second_indirects>
        second_indirect_blocks;
    std::array<config::block_dev_type::sector_id_t, config::third_indirects>
        third_indirect_blocks;
};
}

class cont_file
{
    detail::contiguous_data m_data;
    config::block_dev_type* m_device;

    cont_file(const detail::contiguous_data& data, config::block_dev_type* dev) : m_data(data), m_device(dev) {};

    friend cont_file read(config::block_dev_type* device, config::address_t addr);
    friend void write(config::block_dev_type* device, config::address_t addr, const cont_file&);

    friend cont_file create(config::block_dev_type* device);
public:
    using virtual_block_id = int;

    config::block_dev_type::sector_id_t get_actual_block(virtual_block_id);
    int32_t get_block_count() const;

    void push_block(config::block_dev_type::sector_id_t);
    void pop_block();
};

cont_file read(config::block_dev_type* device, config::address_t addr);
void write(config::block_dev_type* device, config::address_t addr, const cont_file&);

cont_file create(config::block_dev_type* device);
}

