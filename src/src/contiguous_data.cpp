//
// Created by fatih on 10/16/17.
//

#include <fs270/contiguous_data.hpp>
#include <vector>
#include <algorithm>
#include <fs270/fsexcept.hpp>
#include <fs270/block.hpp>

namespace fs {
    cont_file read_cont_file(config::block_dev_type* device, config::address_t addr)
    {
        auto blk_index = addr/device->get_block_size();
        auto offset = addr%device->get_block_size();
        auto buf = get_cache(*device)->load(blk_index);
        auto ptr = buf->data<detail::contiguous_data>(offset);
        return {*ptr, device};
    }

    void write_cont_file(config::block_dev_type* device, config::address_t addr, const cont_file& file)
    {
        auto blk_index = addr/device->get_block_size();
        auto offset = addr%device->get_block_size();
        auto buf = get_cache(*device)->load(blk_index);
        auto ptr = buf->data<detail::contiguous_data>(offset);
        *ptr = file.m_data;
    }

    cont_file create_cont_file(config::block_dev_type* device)
    {
        detail::contiguous_data data;
        data.block_count = 0;
        data.pushable_count = data.direct_blocks.size();
        std::fill(begin(data.direct_blocks), end(data.direct_blocks), config::nullsect);
        std::fill(begin(data.first_indirect_blocks), end(data.first_indirect_blocks), config::nullsect);
        std::fill(begin(data.second_indirect_blocks), end(data.second_indirect_blocks), config::nullsect);
        std::fill(begin(data.third_indirect_blocks), end(data.third_indirect_blocks), config::nullsect);
        return {data, device};
    }

    int32_t cont_file::get_block_count() const
    {
        return m_data.block_count;
    }

    std::vector<config::sector_id_t>
    detail::calc_path(config::sector_id_t id, const contiguous_data& data, uint16_t blksize)
    {
        if (id>=data.block_count) {
            throw id_translation_error("Given virtual index cannot be translated!");
        }

        int ptrs_per_block = blksize/sizeof(config::sector_id_t);

        auto direct_count = data.direct_blocks.size();
        auto first_i_count = data.first_indirect_blocks.size()*ptrs_per_block;
        auto second_i_count = data.second_indirect_blocks.size()*ptrs_per_block*ptrs_per_block;
        auto third_i_count = data.third_indirect_blocks.size()*ptrs_per_block*ptrs_per_block*ptrs_per_block;

        if (id<direct_count) {
            return {id};
        }

        id -= direct_count;
        if (id<first_i_count) {
            auto first_index = id/ptrs_per_block;
            auto offset = id%ptrs_per_block;
            return {first_index, offset};
        }

        id -= first_i_count;
        if (id<second_i_count) {
            auto second_index = id/(ptrs_per_block*ptrs_per_block);
            auto offset = (id%(ptrs_per_block*ptrs_per_block))/ptrs_per_block;
            auto fin_offset = (id%(ptrs_per_block*ptrs_per_block))%ptrs_per_block;
            return {second_index, offset, fin_offset};
        }

        return {};
    }

    std::vector<config::sector_id_t> cont_file::calc_path(cont_file::virtual_block_id id)
    {
        return detail::calc_path(id, m_data, m_cache->device()->get_block_size());
    }

    config::block_dev_type::sector_id_t cont_file::get_actual_block(cont_file::virtual_block_id id)
    {
        auto path = calc_path(id);
        switch (path.size()) {
        case 1: {
            return m_data.direct_blocks[path[0]];
        }
        case 2: {
            auto buf = m_cache->load(m_data.first_indirect_blocks[path[0]]);
            return buf->data<const config::sector_id_t>()[path[1]];
        }
        case 3: {
            auto buf = m_cache->load(m_data.second_indirect_blocks[path[0]]);
            buf = m_cache->load(buf->data<const config::sector_id_t>()[path[1]]);
            return buf->data<const config::sector_id_t>()[path[2]];
        }
        default: {
            throw std::runtime_error("nope");
        }
        }
    }

    bool cont_file::push_block(config::block_dev_type::sector_id_t physical_id)
    {
        if (m_data.pushable_count <= 0)
        {
            return false;
        }

        m_data.pushable_count--;
        auto v_id = m_data.block_count++;
        auto path = calc_path(v_id);
        switch (path.size()) {
        case 1: {
            m_data.direct_blocks[path[0]] = physical_id;
            break;
        }
        case 2: {
            if (m_data.first_indirect_blocks[path[0]]==config::nullsect) {
                m_data.block_count--;
                return false;
            }

            auto buf = m_cache->load(m_data.first_indirect_blocks[path[0]]);
            buf->data<config::sector_id_t>()[path[1]] = physical_id;
            break;
        }
        case 3: {
            if (m_data.second_indirect_blocks[path[0]]==config::nullsect) {
                m_data.block_count--;
                return false;
            }
            auto buf = m_cache->load(m_data.second_indirect_blocks[path[0]]);
            if (buf->data<const config::sector_id_t>()[path[1]]==config::nullsect) {
                m_data.block_count--;
                return false;
            }
            buf = m_cache->load(buf->data<const config::sector_id_t>()[path[1]]);
            buf->data<config::sector_id_t>()[path[2]] = physical_id;
            break;
        }
        default: {
            throw std::runtime_error("nope");
        }
        }
    }

    void cont_file::alloc_indirect_block(config::block_dev_type::sector_id_t id)
    {
        auto sector_id_per_block = m_cache->device()->get_block_size()/sizeof(config::sector_id_t);

        for (auto& sector : m_data.first_indirect_blocks) {
            if (sector==config::nullsect) {
                sector = id;
                m_data.pushable_count += sector_id_per_block;
                return;
            }
        }
        for (auto& sector : m_data.second_indirect_blocks) {
            if (sector==config::nullsect) {
                sector = id;
                return;
            }
            else {
                auto block = m_cache->load(sector);
                auto buf = block->data<const config::sector_id_t>();
                for (int i = 0; i<sector_id_per_block; ++i) {
                    if (buf[i]==config::nullsect) {
                        block->data<config::sector_id_t>()[i] = id;
                        m_data.pushable_count += sector_id_per_block;
                        return;
                    }
                }
            }
        }
    }

    void cont_file::pop_block()
    {
        --m_data.block_count;
        ++m_data.pushable_count;
    }

    int32_t cont_file::get_capacity() const
    {
        return m_data.block_count*m_cache->device()->get_block_size();
    }

    int32_t cont_file::get_pushable_count() const
    {
        return m_data.pushable_count;
    }
}
