//
// Created by fatih on 10/16/17.
//

#include <fs270/contiguous_data.hpp>
#include <vector>
#include <algorithm>
#include <fs270/fsexcept.hpp>
#include <fs270/block.hpp>

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

std::vector<config::sector_id_t>
detail::calc_path(config::sector_id_t id, const contiguous_data& data, uint16_t blksize)
{
    if (id >= data.block_count)
    {
        throw translation_error("Given virtual index cannot be translated!");
    }

    int ptrs_per_block = blksize / sizeof (config::sector_id_t);

    auto direct_count = data.direct_blocks.size();
    auto first_i_count = data.first_indirect_blocks.size() * ptrs_per_block;
    auto second_i_count = data.second_indirect_blocks.size() * ptrs_per_block * ptrs_per_block;
    auto third_i_count = data.third_indirect_blocks.size() * ptrs_per_block * ptrs_per_block * ptrs_per_block;

    if (id < direct_count)
    {
        return { id };
    }

    id -= direct_count;
    if (id < first_i_count)
    {
        auto first_index = id / ptrs_per_block;
        auto offset = id % ptrs_per_block;
        return { first_index, offset };
    }

    id -= first_i_count;
    if (id < second_i_count)
    {
        auto second_index = id / (ptrs_per_block * ptrs_per_block);
        auto offset = (id % (ptrs_per_block * ptrs_per_block)) / ptrs_per_block;
        auto fin_offset = (id % (ptrs_per_block * ptrs_per_block)) % ptrs_per_block;
        return { second_index, offset, fin_offset };
    }

    return {};
}

std::vector<config::sector_id_t> cont_file::calc_path(cont_file::virtual_block_id id)
{
    return detail::calc_path(id, m_data, m_device->get_block_size());
}

config::block_dev_type::sector_id_t cont_file::get_actual_block(cont_file::virtual_block_id id)
{
    auto path = calc_path(id);
    switch (path.size())
    {
    case 1:
    {
        return m_data.direct_blocks[path[0]];
    }
    case 2:
    {
        auto buf = m_cache->load(path[0]);
        return buf->data<const config::sector_id_t>()[path[1]];
    }
    case 3:
    {
        auto buf = m_cache->load(m_data.first_indirect_blocks[path[0]]);
        buf = m_cache->load(buf->data<const config::sector_id_t>()[path[1]]);
        return buf->data<const config::sector_id_t>()[path[2]];
    }
    default:
    {
        throw std::runtime_error("nope");
    }
    }
}

void cont_file::push_block(config::block_dev_type::sector_id_t physical_id)
{
    auto v_id = m_data.block_count++;
    auto path = calc_path(v_id);
    switch (path.size())
    {
    case 1:
    {
        m_data.direct_blocks[path[0]] = physical_id;
        return;
    }
    case 2:
    {
        auto buf = m_cache->load(path[0]);
        buf->data<config::sector_id_t>()[path[1]] = physical_id;
        return;
    }
    case 3:
    {
        auto buf = m_cache->load(m_data.first_indirect_blocks[path[0]]);
        buf = m_cache->load(buf->data<const config::sector_id_t>()[path[1]]);
        buf->data<config::sector_id_t>()[path[2]] = physical_id;
        return;
    }
    default:
    {
        throw std::runtime_error("nope");
    }
    }
}

void cont_file::pop_block()
{
    --m_data.block_count;
}
}
