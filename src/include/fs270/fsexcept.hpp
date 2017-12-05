//
// Created by fatih on 10/21/17.
//

#pragma once

#include <stdexcept>
#include <string>

namespace fs {
    struct fs_error : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    struct operation_error : fs_error {
        using fs_error::fs_error;
    };

    struct integrity_error : fs_error {
        using fs_error::fs_error;
    };

    struct alive_inode_exception : integrity_error {
        alive_inode_exception() : integrity_error("trying to delete an inode with live hardlinks") {}
    };

    struct null_block_exception : integrity_error {
        null_block_exception() : integrity_error("accessed the null block!") {}
    };

    struct double_free : integrity_error {
        double_free() : integrity_error("a block is being freed twice!") {}
    };

    struct id_translation_error : operation_error {
        using operation_error::operation_error;
    };

    struct inode_not_found : operation_error {
        using operation_error::operation_error;
        int32_t not_found_id;

        inode_not_found(int32_t id) : operation_error("inode with id " + std::to_string(id) + " was not found!") {
            not_found_id = id;
        }
    };

    struct out_of_space_error : operation_error {
        out_of_space_error() : operation_error("device is out of space") {}
    };

    struct file_too_big_error : operation_error {
        file_too_big_error() : operation_error("requested file size is greater than possible") {}
    };

    struct not_a_directory : operation_error {
        not_a_directory() : operation_error("opened inode was not a directory") {}
    };

    struct name_too_long : operation_error {
        name_too_long() : operation_error("given path was too long") {}
    };
}