//
// Created by Chani Jindal on 11/18/17.
//

#include <fs270/block_dev.hpp>
#include <fcntl.h>
#include <unistd.h>

namespace fs
{
    int block_dev::write(block_dev::sector_id_t id, const void *data)
    {
        auto ptr = reinterpret_cast<const char *>(data);

        if( id < 0 )
            return -1;

        int byte_offset = get_block_offset(id);
        if( ( byte_offset + block_dev::m_blk_size ) >= block_dev::m_capacity )
            return -1;

        lseek( fd, byte_offset, SEEK_SET );
        ::write( fd, ptr, m_blk_size );

        return 0;
    }

    int block_dev::read(block_dev::sector_id_t id, void *data)
    {
        auto ptr = reinterpret_cast< char *>(data);

        if( id < 0 )
            return -1;

        int byte_offset = get_block_offset(id);
        if( ( byte_offset + block_dev::m_blk_size ) >= block_dev::m_capacity )
            return -1;

        lseek( fd, byte_offset, SEEK_SET );
        ::read(fd, ptr, m_blk_size );

        return 0;
    }

    block_dev::block_dev(const std::string &path, size_t size, uint16_t block_size)
    {
        m_capacity = size;
        m_blk_size = block_size;
        fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
        ftruncate(fd, size);

        if(fd == -1){
            _exit(1);
        }
        if (lseek(fd, size, SEEK_SET) == -1){
            close(block_dev::fd);
            _exit(1);
        }
        if(::write(fd, "", 1) < 0){
            close(fd);
            _exit(1);
        }
    }

    int block_dev::get_block_offset(block_dev::sector_id_t sector)
    {
        return sector * m_blk_size;
    }

    uint16_t block_dev::get_block_size() const
    {
        return m_blk_size;
    }

    block_dev::~block_dev()
    {
        //not sure what to do
        close(fd);
    }

}
