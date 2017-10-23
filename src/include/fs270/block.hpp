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

/**
 * This class represents a buffered block from disk in memory
 * Any change made into the buffer owned by this object is
 * reflected to disk upon the destruction of the last pointer
 * to a block object
 *
 * Read only access is done by getting const object pointers
 */
class block
{
public:

    /**
     * Returns a pointer to an object at the beginning of the block
     * Using this method marks the block as modified and triggers a writeback before deletion
     * If you need read-only access to the buffer, use `const T` instead of `T`
     * @tparam T Type of the returned pointer
     * @return Pointer to the object
     */
    template <class T = char> T* data();

    /**
     * Returns a pointer to an object at the beginning of the block
     * This overload does not mark the block as modified
     * @tparam T Type of the returned pointer
     * @return Pointer to the object
     */
    template <class T = char> const T* data() const;

private:
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
    static_assert(std::is_trivially_copyable<T>{}, "T Must be a trivially copyable type!");
    return data_impl<T>(std::is_const<T>{});
}

template<class T = char> const T *block::data() const
{
    static_assert(std::is_trivially_copyable<T>{}, "T Must be a trivially copyable type!");
    return data_impl<T>(std::is_const<T>{});
}

class block;
using block_ptr = boost::intrusive_ptr<block>;
}

