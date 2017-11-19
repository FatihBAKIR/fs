//
// Created by fatih on 10/22/17.
//

#pragma once

#include <fs270/config.hpp>
#include <memory>
#include <fs270/fs_instance.hpp>
namespace fs
{
struct fs_parameters
{
};

fs_instance make_fs(std::unique_ptr<config::block_dev_type> dev, const fs_parameters& params);
}
