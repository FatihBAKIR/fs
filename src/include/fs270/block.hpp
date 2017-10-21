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

    template <class T>
    T* data_impl(std::false_type);

    template <class T>
    const T* data_impl(std::true_type) const;

public:

    template <class T = char> T* data();
    template <class T = char> const T* data() const;
};

template<class T>
T *block::data_impl(std::false_type)
{
    m_writeback = true;
    return reinterpret_cast<T*>(m_data.get());
}

template<class T>
const T *block::data_impl(std::true_type) const
{
    return reinterpret_cast<const T*>(m_data.get());
}

template<class T = char> T *block::data()
{
    return data_impl<T>(std::is_const<T>{});
}

template<class T = char> const T *block::data() const
{
    return data_impl<T>(std::is_const<T>{});
}

class block;
using block_ptr = boost::intrusive_ptr<block>;
}

