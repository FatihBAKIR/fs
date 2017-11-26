//
// Created by fatih on 10/31/17.
//

#include <fs270/fs_instance.hpp>
#include <fs270/fsexcept.hpp>
#include <fs270/bitmap_allocator.hpp>
#include <utility>
#include <fs270/fsexcept.hpp>

namespace fs {
//    inode_ptr fs_instance::find_inode(int32_t inode_id)
//    {
//        if (inode_id >= 0)
//        {
//            throw inode_not_found(inode_id);
//        }
//
//        auto inodes_per_block = m_device.get_block_size() / (sizeof(inode_data) + sizeof(cont_file));
//
//        auto file_index = inode_id / inodes_per_block;
//        auto inode_offset = inode_id % inodes_per_block;
//
//        auto blk = m_cache->load(file_index);
//        auto data = blk->data(inode_offset * (sizeof(inode_data) + sizeof(cont_file)));
//
//
//    }

    fs_instance::~fs_instance() {
        if (!m_ilist) return;
        inode::write(m_superblk.ilist_address, *m_ilist);
        m_cache->sync();
    }

    fs_instance::fs_instance(std::unique_ptr<config::block_dev_type> dev) {
        auto total_size = dev->capacity();
        auto blk_size = dev->get_block_size();
        auto total_blocks = total_size / blk_size;

        config::sector_id_t superblocks[3];

        superblocks[0] = 1;
        superblocks[1] = total_blocks / 2;
        superblocks[2] = total_blocks - 1;

        auto cache = get_cache(*dev);

        superblock replicas[3];

        replicas[0] = *(cache->load(superblocks[0])->data<const superblock>());
        replicas[1] = *(cache->load(superblocks[1])->data<const superblock>());
        replicas[2] = *(cache->load(superblocks[2])->data<const superblock>());

        superblock match{};
        if (replicas[0] == replicas[1] || replicas[0] == replicas[2]) {
            match = replicas[0];
        } else {
            match = replicas[1];
        }

        auto alloc = std::make_unique<bitmap_allocator>(bitmap_allocator::load(cache, 2));

        m_superblk = match;
        m_alloc = std::move(alloc);
        m_cache = cache;
        m_device = std::move(dev);
        m_ilist = std::make_unique<inode>(inode::read(*this, match.ilist_address));

        // get the "next inode" and pointer to free list from iblock 0 of ilist
        // m_ilist.read(0, buf, fs::inode_size); // buf contains iblock 0
    }

    fs_instance fs_instance::load(std::unique_ptr<config::block_dev_type> dev) {
        return fs_instance(std::move(dev));
    }

    std::unique_ptr<fs_instance> fs_instance::load_heap(std::unique_ptr<config::block_dev_type> dev) {
        return std::unique_ptr<fs_instance>{new fs_instance(std::move(dev))};
    }

    void fs_instance::inode_return(const inode *in) {
        // get inode number from map
        for (auto it = m_ilist_map.begin(); it != m_ilist_map.end(); ++it) {
            if (&(it->second) == in) {
                auto inum = it->first;
                // remove from map
                inode::write(m_ilist->get_physical_address(inum * fs::inode_size), *in);
                m_ilist_map.erase(it);
                return;
            }
        }
        throw fs::inode_not_found(-1);
    }

    int32_t fs_instance::create_inode() {
        int free_ptr, nin; // number of inodes
        m_ilist->read(0, &free_ptr, sizeof(int));
        m_ilist->read(sizeof(int), &nin, sizeof(int));
        uint32_t iaddr;
        if (free_ptr == 0) {
            iaddr = m_ilist->size() / fs::inode_size;
            m_ilist->truncate((iaddr + 1) * fs::inode_size);
        } else { // otherwise, rearrange the pointers
            iaddr = free_ptr;
            m_ilist->read(iaddr * fs::inode_size, &free_ptr,
                          sizeof(int)); // is this undefined if it should be null? or will free_ptr rly be 0
            char buf[16];
            m_ilist->read(iaddr * fs::inode_size + sizeof(int), buf, 16);
            if (strncmp(buf, "THISISAFREEINODE", 16) != 0)
            {
                throw std::runtime_error("ilist is broken");
            }
            // update free_ptr
            m_ilist->write(0, &free_ptr, sizeof(int));
        }
        nin++;
        m_ilist->write(sizeof(int), &nin, sizeof(int));
        auto in = inode::create(*this);
        inode::write(m_ilist->get_physical_address(iaddr * fs::inode_size), in);
        return iaddr;
    }

    inode_ptr fs_instance::get_inode(int32_t inum) {
        if (inum == 0) return nullptr;
        auto it = m_ilist_map.find(inum);
        if (it == m_ilist_map.end()) {
            // not on the map, need to get from disk
            auto in = inode::read(*this, m_ilist->get_physical_address(inum * fs::inode_size));
            // if in is null throw inode not found
            auto it = m_ilist_map.insert(std::make_pair(inum, in));
            return inode_ptr(&it.first->second);
        } else {
            return inode_ptr(&it->second);
        }
    }

    void fs_instance::remove_inode(int32_t inum) {
        {
            auto in = get_inode(inum);
            if (in->get_hardlinks() != 0)
            {
                throw alive_inode_exception{};
            }
            if (in->get_mem_refcnt() != 1)
            {
                throw alive_inode_exception{};
            }
        }
        // make sure to write a pointer or nullptr to the removed inode
        int free_ptr, nin; // number of inodes
        m_ilist->read(0, &free_ptr, sizeof(int));
        m_ilist->read(sizeof(int), &nin, sizeof(int));
        // point new block to begin of list (effectively placing it at the beginning)
        m_ilist->write(inum * fs::inode_size, &free_ptr, sizeof(int));
        m_ilist->write(inum * fs::inode_size + sizeof(int), "THISISAFREEINODE", 16);
        free_ptr = inum;
        m_ilist->write(0, &free_ptr, sizeof(int));
        nin--;
        m_ilist->write(sizeof(int), &nin, sizeof(int));
    }
}