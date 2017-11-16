//
// Created by fatih on 10/31/17.
//

#include <fs270/fs_instance.hpp>
#include <fs270/fsexcept.hpp>
#include <fs270/bitmap_allocator.hpp>
#include <utility>

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

    fs_instance::fs_instance(std::unique_ptr<config::block_dev_type> dev)
    {
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

        superblock match {};
        if (replicas[0] == replicas[1] || replicas[0] == replicas[2])
        {
            match = replicas[0];
        }
        else
        {
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

    void fs_instance::inode_return(inode *inode) {
    }

    config::address_t get_addr(std::unique_ptr<inode> ilist) {
      int32_t buf[fs::inode_size/sizeof(int32_t)];
      ilist->read(0, buf, fs::inode_size);
      if(buf[1] == 0) {
        // increment next inode here?
        return buf[0];
      }
      // otherwise, follow the pointers
      // update the pointers here?
      return buf[1];
    }

    int32_t fs_instance::create_inode() {
      auto in = inode::create(*this);
      m_ilist_map.insert(std::make_pair<int32_t, inode>(m_ilist_map.size(), std::move(in)));
      // come up with address addr from ilist
      config::address_t addr = get_addr(m_ilist);
      //m_ilist.add_inode(in);
      inode::write(addr, in);
      return m_ilist_map.size()-1;
    }

    inode_ptr fs_instance::get_inode(int32_t inum) {
      return NULL;
    }

    void fs_instance::remove_inode(int32_t inum) {
      return;
    }
}