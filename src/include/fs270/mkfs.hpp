//
// Created by fatih on 10/22/17.
//

#pragma once

#include <fs270/config.hpp>

namespace fs
{
struct fs_parameters
{
    int max_inodes;
};

void make_fs(config::block_dev_type& dev, const fs_parameters& params);
}
