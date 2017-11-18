//
// Created by fatih on 11/17/17.
//

#pragma once

#include <fs270/config.hpp>
#include <memory>

namespace fs
{
namespace tests
{
    std::unique_ptr<config::block_dev_type>
    get_block_dev();
}
}
