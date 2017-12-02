//
// Created by fatih on 10/22/17.
//

#include <fs270/inode.hpp>
#include <fs270/fs_instance.hpp>
#include <fs270/bitmap_allocator.hpp>
#include <cmath>
#include <cstring>
#include <fs270/fsexcept.hpp>

namespace fs {
    void inode::update_mod_time() {
        auto time = std::chrono::system_clock::now();
        auto u_time = std::chrono::system_clock::to_time_t(time);
        m_data.mod_time = u_time;
    }

    void inode::update_chg_time() {
        auto time = std::chrono::system_clock::now();
        auto u_time = std::chrono::system_clock::to_time_t(time);
        m_data.chg_time = u_time;
    }

    void inode::update_access_time() const {
        auto time = std::chrono::system_clock::now();
        auto u_time = std::chrono::system_clock::to_time_t(time);
        m_data.access_time = u_time;
    }

    void inode::set_times(clock::time_point change, clock::time_point mod, clock::time_point access) {
        m_data.chg_time = clock::to_time_t(change);
        m_data.mod_time = clock::to_time_t(mod);
        m_data.access_time = clock::to_time_t(access);
    }

    auto inode::get_modification_time() const -> clock::time_point {
        return clock::from_time_t(m_data.mod_time);
    }

    auto inode::get_change_time() const -> clock::time_point {
        return clock::from_time_t(m_data.chg_time);
    }

    auto inode::get_access_time() const -> clock::time_point {
        return clock::from_time_t(m_data.access_time);
    }

    void inode::set_owner(int32_t user_id) {
        update_chg_time();
        m_data.owner = user_id;
    }

    int32_t inode::get_owner() const {
        return m_data.owner;
    }

    void inode::set_group(int32_t group_id) {
        update_chg_time();
        m_data.group = group_id;
    }

    int32_t inode::get_group() const {
        return m_data.group;
    }

    void intrusive_ptr_add_ref(const inode *n) {
        n->m_refcnt++;
    }

    void intrusive_ptr_release(const inode *n) {
        n->m_refcnt--;
        if (n->m_refcnt == 0) {
            n->m_fs->inode_return(n);
        }
    }

    int32_t inode::read(uint64_t from, void *buf, int32_t len) const {
        if (from + len >= size()) {
            len = size() - from;
        }

        auto read_res = len;

        if (len <= 0) {
            return 0;
        }

        auto buffer = reinterpret_cast<char *>(buf);
        auto offset = from;

        auto cache = m_fs->blk_cache();
        auto dev = cache->device();

        while (len > 0) {
            auto blk_id = offset / dev->get_block_size();
            auto blk_offset = offset % dev->get_block_size();

            auto blk = cache->load(m_blocks.get_actual_block(blk_id));
            auto blk_buf = blk->data<const char>();

            auto copy_bytes = std::min<int32_t>(len, int32_t(dev->get_block_size()) - blk_offset);

            std::copy(blk_buf + blk_offset, blk_buf + blk_offset + copy_bytes, buffer);
            buffer += copy_bytes;
            offset += copy_bytes;
            len -= copy_bytes;
        }

        update_access_time();
        return read_res;
    }

    void inode::write(uint64_t from, const void *buf, int32_t len) {

        if (from + len > size())
        {
            truncate(from + len);
        }

        if (len <= 0) {
            return;
        }

        auto buffer = reinterpret_cast<const char *>(buf);
        auto offset = from;

        auto cache = m_fs->blk_cache();
        auto dev = cache->device();

        while (len > 0) {
            auto blk_id = offset / dev->get_block_size();
            auto blk_offset = offset % dev->get_block_size();
            auto copy_bytes = std::min<int32_t>(len, int32_t(dev->get_block_size()) - blk_offset);

            auto a_blk_id = m_blocks.get_actual_block(blk_id);
            if (a_blk_id == config::nullsect)
            {
                a_blk_id = m_blocks.get_actual_block(blk_id);
                throw null_block_exception{};
            }
            auto blk = cache->load(a_blk_id, copy_bytes == dev->get_block_size());
            auto blk_buf = blk->data<char>();

            std::copy(buffer, buffer + copy_bytes, blk_buf + blk_offset);
            buffer += copy_bytes;
            offset += copy_bytes;
            len -= copy_bytes;
        }
        update_mod_time();
    }

    void inode::truncate(int64_t new_size) {
        if (new_size < m_data.file_size) {
            m_data.file_size = new_size;

            auto needed_blk_count = int32_t(std::ceil(double(new_size) / m_fs->blk_cache()->device()->get_block_size()));

            for (int i = m_blocks.get_block_count();
                 i > needed_blk_count;
                 --i) {
                m_fs->allocator()->free(m_blocks.get_actual_block(i - 1), 1);
                m_blocks.pop_block();
            }

            config::sector_id_t free;
            while ((free = m_blocks.pop_indirect_block()) != config::nullsect)
            {
                m_fs->allocator()->free(free, 1);
            }
        }
        else if (new_size > m_data.file_size)
        {
            if (new_size > m_fs->max_inode_size())
            {
                throw file_too_big_error{};
            }
            m_data.file_size = new_size;

            auto needed_blk_count = int32_t(std::ceil(double(new_size) / m_fs->blk_cache()->device()->get_block_size()));

            auto diff = needed_blk_count - m_blocks.get_block_count();
            while (m_blocks.get_pushable_count() < diff)
            {
                auto blk = m_fs->allocator()->alloc(1);
                if (blk == config::nullsect)
                {
                    throw out_of_space_error{};
                }
                m_blocks.push_indirect_block(blk);
            }

            for (int i = m_blocks.get_block_count(); i < needed_blk_count; ++i)
            {
                auto blk = m_fs->allocator()->alloc(1);
                if (blk == config::nullsect)
                {
                    throw out_of_space_error{};
                }
                m_blocks.push_block(blk);
                auto b = m_fs->blk_cache()->load(blk, true);
                auto buf = b->data<uint64_t>();
                std::fill(buf, buf + (m_fs->blk_cache()->device()->get_block_size() / sizeof(uint64_t)), 0);
            }
        }
        if (m_blocks.get_capacity() < size())
        {
            throw integrity_error("truncate fail");
        }
        update_chg_time();
        update_mod_time();
    }

    inode::inode(fs_instance *inst) : m_fs(inst), m_blocks(fs::create_cont_file(inst->blk_cache())) {
    }

    inode inode::create(fs_instance &fs) {
        auto in = inode(&fs);
        std::memset(&in.m_data, 0, sizeof in.m_data);
        in.m_data.file_size = 0;
        in.set_times(clock::now(), clock::now(), clock::now());
        in.m_data.ref_cnt = 0;
        in.set_mode(0644);
        return in;
    }

    config::address_t inode::get_physical_address(uint32_t ioptr) const
    {
        auto blk_sz = m_fs->blk_cache()->device()->get_block_size();
        auto blk_id = ioptr / blk_sz;
        auto offset = ioptr % blk_sz;

        auto physical_blk_id = m_blocks.get_actual_block(blk_id);
        auto address = config::address_t(physical_blk_id) * blk_sz + offset;
        return address;
    }

    template <class T>
    void write_raw(block_cache* cache, config::address_t at, const T& t)
    {
        static_assert(std::is_trivially_copyable<T>{}, "");
        auto blk_id = at / cache->device()->get_block_size();
        auto off = at % cache->device()->get_block_size();
        if (off + sizeof t > cache->device()->get_block_size())
        {
            throw std::runtime_error("nope");
        }
        auto blk = cache->load(blk_id);
        std::memcpy(blk->data(off), &t, sizeof t);
    }

    template <class T>
    void read_raw(block_cache* cache, config::address_t from, T& t)
    {
        static_assert(std::is_trivially_copyable<T>{}, "");
        auto blk_id = from / cache->device()->get_block_size();
        auto off = from % cache->device()->get_block_size();
        if (off + sizeof t > cache->device()->get_block_size())
        {
            throw std::runtime_error("nope");
        }
        auto blk = cache->load(blk_id);
        std::memcpy(&t, blk->data<const char>(off), sizeof t);
    }

    void inode::write(config::address_t at, const inode &inode) {
        auto cache = inode.m_fs->blk_cache();
        auto blk_id = at / cache->device()->get_block_size();
        auto blk = cache->load(blk_id);
        write_raw(cache, at, inode.m_data);
        fs::write_cont_file(cache, at + sizeof inode.m_data, inode.m_blocks);
    }

    inode inode::read(fs_instance &fs, config::address_t at) {
        auto cache = fs.blk_cache();
        auto blk_id = at / cache->device()->get_block_size();
        auto blk = cache->load(blk_id);
        inode in(&fs);
        read_raw(cache, at, in.m_data);
        in.m_blocks = read_cont_file(cache, at + sizeof in.m_data);
        return in;
    }

    void ::fs::detail::create_raw(block_cache *cache, config::address_t at) {
        inode_data data {};
        std::memset(&data, 0, sizeof data);
        auto blk_id = at / cache->device()->get_block_size();
        auto blk = cache->load(blk_id);
        write_raw(cache, at, data);
        fs::write_cont_file(cache, at + sizeof data, fs::create_cont_file(cache));
    }
}