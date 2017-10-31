//
// Created by fatih on 10/21/17.
//

#pragma once

#include <stdexcept>
namespace fs
{
struct id_translation_error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct inode_not_found : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}