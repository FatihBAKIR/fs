//
// Created by Chani Jindal on 11/18/17.
//

#include <fs270/disk_block_dev.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

namespace fs
{
    int disk_block_dev::write(disk_block_dev::sector_id_t id, const void *data)
    {
        auto ptr = reinterpret_cast<const char *>(data);

        if( id < 0 )
            return -1;

        int byte_offset = get_block_offset(id);
        if( ( byte_offset + disk_block_dev::m_blk_size ) >= disk_block_dev::m_capacity )
            return -1;

        lseek( fd, byte_offset, SEEK_SET );
        ::write( fd, ptr, m_blk_size );

        return 0;
    }

    int disk_block_dev::read(disk_block_dev::sector_id_t id, void *data)
    {
        auto ptr = reinterpret_cast< char *>(data);

        if( id < 0 )
            return -1;

        int byte_offset = get_block_offset(id);
        if( ( byte_offset + disk_block_dev::m_blk_size ) >= disk_block_dev::m_capacity )
            return -1;

        lseek( fd, byte_offset, SEEK_SET );
        ::read(fd, ptr, m_blk_size );

        return 0;
    }

    disk_block_dev::disk_block_dev(const std::string &path, size_t size, uint16_t block_size)
    {
        m_capacity = size;
        m_blk_size = block_size;
        fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
        //ftruncate(fd, size);

        if(fd == -1){
            perror("open: ");
            throw std::runtime_error("can't open device!");
        }

        if (lseek(fd, size, SEEK_SET) == -1){
            close(fd);
            throw std::runtime_error("can't seek device!");
        }

        char buf[4096];
        std::fill(std::begin(buf), std::end(buf), 0);

        if(::write(fd, buf, 4096) < 0){
            close(fd);
            throw std::runtime_error("can't write device!");
        }
    }

    int disk_block_dev::get_block_offset(disk_block_dev::sector_id_t sector)
    {
        return sector * m_blk_size;
    }

    uint16_t disk_block_dev::get_block_size() const
    {
        return m_blk_size;
    }

    disk_block_dev::~disk_block_dev()
    {
        //not sure what to do
        close(fd);
    }
}
