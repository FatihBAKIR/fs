//
// Created by fatih on 10/16/17.
//

#include <fs270/contiguous_data.hpp>
#include <vector>
#include <algorithm>

namespace fs
{
cont_file read(config::block_dev_type *device, config::address_t addr)
{
    auto blk_index = addr / device->get_block_size();
    auto offset = addr % device->get_block_size();
    std::vector<char> buffer(device->get_block_size());
    device->read(blk_index, buffer.data());

    auto ptr = reinterpret_cast<detail::contiguous_data*>(buffer.data() + offset);
    return { *ptr, device };
}

void write(config::block_dev_type *device, config::address_t addr, const cont_file & file)
{
    auto blk_index = addr / device->get_block_size();
    auto offset = addr % device->get_block_size();
    std::vector<char> buffer(device->get_block_size());
    device->read(blk_index, buffer.data());

    auto ptr = reinterpret_cast<detail::contiguous_data*>(buffer.data() + offset);
    *ptr = file.m_data;

    device->write(blk_index, buffer.data());
}

cont_file create(config::block_dev_type* device)
{
    detail::contiguous_data data;
    data.block_count = 0;
    std::fill(begin(data.direct_blocks), end(data.direct_blocks), config::nullsect);
    std::fill(begin(data.first_indirect_blocks), end(data.first_indirect_blocks), config::nullsect);
    std::fill(begin(data.second_indirect_blocks), end(data.second_indirect_blocks), config::nullsect);
    std::fill(begin(data.third_indirect_blocks), end(data.third_indirect_blocks), config::nullsect);
    return { data, device };
}

int32_t cont_file::get_block_count() const
{
    return m_data.block_count;
}

config::block_dev_type::sector_id_t cont_file::get_actual_block(cont_file::virtual_block_id id)
{
    auto ptrs_per_block = m_device->get_block_size() / sizeof (config::sector_id_t);

    auto direct_count = m_data.direct_blocks.size();
    auto first_i_count = m_data.first_indirect_blocks.size() * ptrs_per_block;
    auto second_i_count = m_data.second_indirect_blocks.size() * ptrs_per_block * ptrs_per_block;
    auto third_i_count = m_data.third_indirect_blocks.size() * ptrs_per_block * ptrs_per_block * ptrs_per_block;

    if (id < direct_count)
    {
        return m_data.direct_blocks[id];
    }

    id -= direct_count;
    if (id < first_i_count)
    {
        auto first_index = id / ptrs_per_block;
        auto offset = id % ptrs_per_block;
        std::vector<config::sector_id_t> buffer(m_device->get_block_size());
        m_device->read(m_data.first_indirect_blocks[first_index], buffer.data());
        return buffer[offset];
    }

    id -= first_i_count;
    if (id < second_i_count)
    {
        auto second_index = id / (ptrs_per_block * ptrs_per_block);
        auto offset = (id % (ptrs_per_block * ptrs_per_block)) / ptrs_per_block;
        auto fin_offset = id % ptrs_per_block;
        std::vector<config::sector_id_t> buffer(m_device->get_block_size());
        m_device->read(m_data.first_indirect_blocks[second_index], buffer.data());
        m_device->read(buffer[offset], buffer.data());
        return buffer[fin_offset];
    }

    // third indirect
}

void cont_file::push_block(config::block_dev_type::sector_id_t)
{

}

void cont_file::pop_block()
{

}
}
