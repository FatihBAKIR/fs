//
// Created by fatih on 10/21/17.
//

#pragma once

#include <memory>
#include <boost/intrusive_ptr.hpp>
#include <fs270/config.hpp>

namespace fs
{
class block_cache;
class block
{
    config::sector_id_t m_id;
    block_cache* m_cache;
    bool m_writeback;
    int8_t m_refcnt;
    std::unique_ptr<uint8_t[]> m_data;

    friend class block_cache;
    friend class id_comparator;
    friend void intrusive_ptr_add_ref(block* b);
    friend void intrusive_ptr_release(block* b);

public:

    template <class T = uint8_t>
    T* data();

    template <class T = uint8_t>
    const T* data() const;
};

template<class T = uint8_t>
T *block::data()
{
    m_writeback = true;
    return reinterpret_cast<T*>(m_data.get());
}

template<class T = uint8_t>
const T *block::data() const
{
    return reinterpret_cast<const T*>(m_data.get());
}

class block;
using block_ptr = boost::intrusive_ptr<block>;
}

