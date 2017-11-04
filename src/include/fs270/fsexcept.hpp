//
// Created by fatih on 10/21/17.
//

#pragma once

#include <stdexcept>
#include <string>
namespace fs
{
struct id_translation_error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct inode_not_found : std::runtime_error
{
    using std::runtime_error::runtime_error;

    int32_t not_found_id;

    inode_not_found(int32_t id) : runtime_error("inode with id " + std::to_string(id) + " was not found!") {
        not_found_id = id;
    }
};

struct null_block_exception
{
};
}