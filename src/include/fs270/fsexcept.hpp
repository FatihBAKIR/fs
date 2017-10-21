//
// Created by fatih on 10/21/17.
//

#pragma once

#include <stdexcept>
namespace fs
{
struct translation_error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}