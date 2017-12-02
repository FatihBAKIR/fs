//
// Created by fatih on 11/17/17.
//
#include <fuse.h>
#include <string>
#include <memory>
#include <iostream>
#include <fs270/fs_instance.hpp>
#include <spdlog/spdlog.h>
#include <fs270/directory.hpp>
#include <boost/predef.h>
#include <chrono>
#include <boost/type_index.hpp>
#include <spdlog/sinks/null_sink.h>
#include <fs270/fsexcept.hpp>

struct fs_fuse_private {
    std::unique_ptr<fs::fs_instance> fs;
    std::shared_ptr<spdlog::logger> log;
    fs::directory rootdir;

    fs_fuse_private(std::unique_ptr<fs::fs_instance> f)
            : fs(std::move(f)), rootdir(fs::directory(fs->get_inode(1))) {}
};

fs_fuse_private *get_private() {
    return static_cast<fs_fuse_private *>(fuse_get_context()->private_data);
}

timespec to_tspec(std::chrono::system_clock::time_point tp) {
    timespec ts;
    auto dur = tp.time_since_epoch();
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(dur);
    ts.tv_sec = secs.count();
    dur -= secs;
    ts.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    return ts;
}

auto from_tspec(const timespec &ts) {
    std::chrono::system_clock::time_point tp;
    tp += std::chrono::seconds(ts.tv_sec);
    //tp += std::chrono::nanoseconds(ts.tv_nsec);
    return tp;
}

boost::string_view get_name(boost::string_view v) {
    auto last = v.rfind('/');
    auto name = v.substr(last + 1, v.size());
    return name;
}

int parent_dir(fs::fs_instance &fs, const char *p) {
    auto v = boost::string_view(p);
    auto last = v.rfind('/');
    auto to_find = v.substr(0, last);
    auto name = v.substr(last + 1, v.size());

    auto dir_inum = fs::lookup(fs, to_find);
    return dir_inum;
}

int create_inode(fs::fs_instance &fs, const char *p) {
    auto v = boost::string_view(p);
    auto last = v.rfind('/');
    auto to_find = v.substr(0, last);
    auto name = v.substr(last + 1, v.size());

    if (name.size() > 255) {
        return -ENAMETOOLONG;
    }

    auto dir_inum = fs::lookup(fs, to_find);
    if (dir_inum == 0) {
        return -ENOENT;
    }

    auto dir = fs::directory(fs.get_inode(dir_inum));

    if (dir.find(name) != dir.end()) {
        return -EEXIST;
    }

    auto inum = fs.create_inode();
    try {
        dir.add_entry(name, inum);
    }
    catch (fs::fs_error &) {
        fs.remove_inode(inum);
        throw;
    }

    return inum;
}

void fill_stats(fs::inode_ptr inode, struct stat *stbuf, int32_t inum) {
    auto priv = get_private();
    memset(stbuf, 0, sizeof *stbuf);
    stbuf->st_mode = inode->get_mode();

    switch (inode->get_type()) {
        case fs::inode_type::regular:
            stbuf->st_mode |= S_IFREG;
            break;
        case fs::inode_type::directory:
            stbuf->st_mode |= S_IFDIR;
            break;
        case fs::inode_type::symlink:
            stbuf->st_mode |= S_IFLNK;
            break;
    }

    stbuf->st_nlink = std::max<int>(1, inode->get_hardlinks());
    stbuf->st_size = inode->size();

#if BOOST_OS_LINUX
    stbuf->st_atim = to_tspec(inode->get_access_time());
    stbuf->st_mtim = to_tspec(inode->get_modification_time());
    stbuf->st_ctim = to_tspec(inode->get_change_time());
#elif BOOST_OS_MACOS
    stbuf->st_atimespec = to_tspec(inode->get_access_time());
    stbuf->st_mtimespec = to_tspec(inode->get_modification_time());
    stbuf->st_ctimespec = to_tspec(inode->get_creation_time());
#endif

    stbuf->st_blksize = priv->fs->blk_cache()->device()->get_block_size();
    stbuf->st_blocks = inode->capacity() / stbuf->st_blksize;
    stbuf->st_uid = inode->get_owner();
    stbuf->st_gid = inode->get_group();
    stbuf->st_ino = inum;
}


extern "C" {
struct fs_opaque;

void *fs_init(struct fuse_conn_info *conn) {
    auto fs = fs::fs_instance::load_heap(
            std::make_unique<fs::config::block_dev_type>("/dev/sdb", 10LL * 1024 * 1024 * 1024, 4096));

    auto priv = new fs_fuse_private(std::move(fs));
    spdlog::set_async_mode(8192 * 4);
    priv->log =
            spdlog::stderr_color_st("fslog");
    //std::make_shared<spdlog::logger>("null_log",std::make_shared<spdlog::sinks::null_sink_st>());

    priv->log->info("Initiated filesystem");
    priv->log->info("Using \"{}\" as the block device",
                    boost::typeindex::type_id<fs::config::block_dev_type>().pretty_name());
    priv->log->info("Fuse version: {}", fuse_version());

    return priv;
}

void fs_destroy(void *arg) {
    auto fsp = static_cast<fs_fuse_private *>(arg);
    delete fsp;
}

int fs_statfs(const char *, struct statvfs *stat) {
    auto priv = get_private();
    priv->log->info("Statfs");
    fs::fs_instance *inst = priv->fs.get();
    stat->f_blocks = inst->get_total_blocks();
    stat->f_bsize = inst->blk_cache()->device()->get_block_size();
    stat->f_frsize = stat->f_bsize;
    stat->f_bfree = inst->allocator()->get_num_free_blocks();
    stat->f_bavail = inst->allocator()->get_num_free_blocks();

#if BOOST_OS_MACOS
    stat->f_files = 100000000;
#elif BOOST_OS_LINUX
    stat->f_files = inst->get_number_inodes();
#endif
    stat->f_ffree = 1000000;
    stat->f_favail = 1000000;

    stat->f_fsid = 0xF5270;
    stat->f_flag = 0;
    stat->f_namemax = 255;
    return 0;
}

int fs_open(const char *p, fuse_file_info *fi) try {
    auto priv = get_private();
    priv->log->info("Opening {}", p);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fi->fh = inum;
    priv->log->info("Opened \"{}\" -> {}", p, fi->fh);

    return 0;
}
catch (fs::not_a_directory &) {
    return -ENOTDIR;
}
catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_read(const char *p, char *buf, size_t sz, off_t off, fuse_file_info *fi) {
    auto priv = get_private();
    priv->log->info("Reading {}, {} bytes from {}", p, sz, off);

    off = std::min<off_t>(off, 0x7ffff000);

    auto inode = priv->fs->get_inode(fi->fh);
    return inode->read(off, buf, sz);
}

int fs_write(const char *p, const char *buf, size_t sz, off_t off, fuse_file_info *fi) try {
    auto priv = get_private();
    priv->log->info("Writing {}, {} bytes from {}", p, sz, off);
    fs::inode_ptr inode = priv->fs->get_inode(fi->fh);
    inode->write(off, buf, sz);
    return sz;
}
catch (fs::file_too_big_error &) {
    return -EFBIG;
}
catch (fs::out_of_space_error &) {
    return -ENOSPC;
}

int fs_chmod(const char *p, mode_t m) try {
    auto priv = get_private();
    priv->log->info("CHMOD {}, {:o}", p, m);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr in = priv->fs->get_inode(inum);
    in->set_mode(m);
    return 0;
}
catch (fs::not_a_directory &) {
    return -ENOTDIR;
}
catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_chown(const char *p, uid_t u, gid_t g) try {
    auto priv = get_private();
    priv->log->info("CHOWN {}, {}, {}", p, u, g);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr in = priv->fs->get_inode(inum);
    in->set_owner(u);
    in->set_group(g);
    return 0;
}
catch (fs::not_a_directory &) {
    return -ENOTDIR;
}
catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_getattr(const char *path, struct stat *stbuf) try {
    auto priv = get_private();
    priv->log->info("Getattr {}", path);

    auto inum = fs::lookup(*priv->fs, path);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr inode = priv->fs->get_inode(inum);

    fill_stats(inode, stbuf, inum);

    return 0;
}
catch (fs::not_a_directory &) {
    return -ENOTDIR;
}
catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_release(const char *, fuse_file_info *fi) {
    auto priv = get_private();
    priv->log->info("Closed -> {}", fi->fh);
    return 0;
}

#if BOOST_OS_MACOS
using access_mode_t = fd_mask;
#elif BOOST_OS_LINUX
using access_mode_t = int ;
#endif

int fs_access(const char *p, access_mode_t mode) {
    return 0;
}

int fs_truncate(const char *p, off_t sz) try {
    auto priv = get_private();
    priv->log->info("Truncate {}, {}", p, sz);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr in = priv->fs->get_inode(inum);
    in->truncate(sz);
    return 0;
} catch (fs::out_of_space_error) {
    return -ENOSPC;
} catch (fs::file_too_big_error) {
    return -EFBIG;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_create(const char *p, mode_t m, fuse_file_info *fi) try {
    auto priv = get_private();
    priv->log->info("Create {}", p);

    auto inum = create_inode(*priv->fs, p);
    if (inum < 0) {
        return inum;
    }

    fs::inode_ptr inode = priv->fs->get_inode(inum);
    inode->set_owner(fuse_get_context()->uid);
    inode->set_group(fuse_get_context()->gid);
    inode->set_mode(m);
    fi->fh = inum;
    return 0;
} catch (fs::out_of_space_error &) {
    return -ENOSPC;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_settimes(const char *p, const timespec *ts) try {
    auto priv = get_private();
    priv->log->info("Update times {}, {}", p);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr inode = priv->fs->get_inode(inum);
    inode->set_times(inode->get_change_time(), from_tspec(ts[1]), from_tspec(ts[0]));

    return 0;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_symlink(const char *target, const char *p) try {
    auto priv = get_private();
    priv->log->info("Symlink \"{}\" -> \"{}\"", p, target);

    auto inum = create_inode(*priv->fs, p);
    if (inum < 0) {
        return inum;
    }

    if (strlen(target) > 4096) {
        return -EFBIG;
    }

    fs::inode_ptr inode = priv->fs->get_inode(inum);
    inode->set_owner(fuse_get_context()->uid);
    inode->set_group(fuse_get_context()->gid);
    inode->set_type(fs::inode_type::symlink);
    inode->write(0, target, strlen(target) + 1);
    return 0;
}
catch (fs::out_of_space_error &) {
    return -ENOSPC;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_readlink(const char *p, char *buf, size_t bufsz) try {
    auto priv = get_private();
    priv->log->info("Readlink \"{}\" -> \"{}\"", p);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr in = priv->fs->get_inode(inum);
    auto len = in->read(0, buf, bufsz);
    return 0;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_link(const char *to, const char *p) try {
    auto priv = get_private();
    priv->log->info("Link \"{}\" -> \"{}\"", p, to);

    auto dir_inum = parent_dir(*priv->fs, p);
    if (dir_inum <= 0) {
        return -ENOENT;
    }

    auto to_inum = fs::lookup(*priv->fs, to);

    if (to_inum <= 0) {
        return -ENOENT;
    }

    auto name = boost::string_view(p);
    name = name.substr(name.rfind('/') + 1, name.size());

    auto dir = fs::directory(priv->fs->get_inode(dir_inum));
    dir.add_entry(name, to_inum);

    return 0;
} catch (fs::out_of_space_error &) {
    return -ENOSPC;
} catch (fs::file_too_big_error &) {
    return -EFBIG;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_unlink(const char *p) try {
    auto priv = get_private();
    priv->log->info("Unlink \"{}\"", p);

    auto parent_dirnum = parent_dir(*priv->fs, p);

    if (parent_dirnum == 0) {
        return -ENOENT;
    }

    auto parent_dir = fs::directory(priv->fs->get_inode(parent_dirnum));

    auto name = boost::string_view(p);
    name = name.substr(name.rfind('/') + 1, name.size());

    auto it = parent_dir.find(name);
    if (it == parent_dir.end()) {
        return -ENOENT;
    }

    auto inum = it.get_inumber();
    fs::inode_ptr inode = priv->fs->get_inode(inum);

    parent_dir.del_entry(name);

    if (inode->get_hardlinks() == 0) {
        priv->log->info("Deleting \"{}\"", p);
        inode->truncate(0);
        inode.reset();
        priv->fs->remove_inode(inum);
    }

    return 0;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
}

int fs_rmdir(const char *p) try {
    auto priv = get_private();
    priv->log->info("Rmdir \"{}\"", p);

    auto parent_dir_inum = parent_dir(*priv->fs, p);
    if (parent_dir_inum == 0) {
        return -ENOENT;
    }

    auto name = get_name(p);

    auto parent_dir = fs::directory(priv->fs->get_inode(parent_dir_inum));

    auto it = parent_dir.find(name);

    if (it == parent_dir.end()) {
        return -ENOENT;
    }

    auto inum = it.get_inumber();
    fs::inode_ptr child_inode = priv->fs->get_inode(inum);

    {
        auto child_dir = fs::directory(child_inode);

        if (child_dir.begin() != child_dir.end()) {
            // folder is not empty
            return -EPERM;
        }
    }

    parent_dir.del_entry(name);

    if (child_inode->get_hardlinks() == 0) {
        priv->log->info("Deleting \"{}\"", p);
        child_inode->truncate(0);
        child_inode.reset();
        priv->fs->remove_inode(inum);
    }

    return 0;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
}

int fs_mkdir(const char *p, mode_t m) try {

    auto priv = get_private();
    priv->log->info("Mkdir \"{}\"", p);

    auto parent_dirnum = parent_dir(*priv->fs, p);

    auto v = boost::string_view(p);
    auto last = v.rfind('/');
    auto to_find = v.substr(0, last);
    auto name = v.substr(last + 1, v.size());

    if (name.size() > 255) {
        return -ENAMETOOLONG;
    }

    auto root_dir = fs::directory(priv->fs->get_inode(parent_dirnum));
    auto new_dir = priv->fs->create_inode();
    try
    {
        root_dir.add_entry(name, new_dir);
    }
    catch (fs::fs_error&)
    {
        priv->fs->remove_inode(new_dir);
        throw;
    }
    auto new_in = priv->fs->get_inode(new_dir);
    new_in->set_type(fs::inode_type::directory);
    new_in->set_owner(fuse_get_context()->uid);
    new_in->set_group(fuse_get_context()->gid);
    new_in->set_mode(m);

    return 0;
} catch (fs::out_of_space_error) {
    return -ENOSPC;
} catch (fs::file_too_big_error) {
    return -EFBIG;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_readdir(const char *p, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) try {

    auto priv = get_private();
    priv->log->info("Readdir \"{}\"", p);

    auto inum = fs::lookup(*priv->fs, p);
    auto dir = fs::directory(priv->fs->get_inode(inum));

    filler(buf, ".", nullptr, 0);
    filler(buf, "..", nullptr, 0);

    for (auto entry : dir) {
        struct stat s;
        auto in = priv->fs->get_inode(entry.second);
        fill_stats(in, &s, entry.second);
        filler(buf, entry.first.c_str(), &s, 0);
    }

    return 0;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
}

int fs_rename(const char *p, const char *to) try {
    auto priv = get_private();
    priv->log->info("Rename \"{}\" -> \"{}\"", p, to);

    auto old_parent = parent_dir(*priv->fs, p);
    auto new_parent = parent_dir(*priv->fs, to);

    if (old_parent == 0 || new_parent == 0) {
        return -ENOENT;
    }

    auto old_name = get_name(p);
    auto new_name = get_name(to);

    auto old_dir = fs::directory(priv->fs->get_inode(old_parent));
    auto new_dir = fs::directory(priv->fs->get_inode(new_parent));

    auto old_it = old_dir.find(old_name);
    if (old_it == old_dir.end()) {
        return -ENOENT;
    }

    auto old_inum = old_it.get_inumber();
    auto new_it = new_dir.find(new_name);

    if (new_it != new_dir.end()) {
        auto exist = new_it.get_inumber();
        auto exist_inode = priv->fs->get_inode(exist);
        new_dir.del_entry(new_name);
        if (exist_inode->get_hardlinks() == 0) {
            exist_inode->truncate(0);
            exist_inode.reset();
            priv->fs->remove_inode(exist);
        }
    }

    new_dir.add_entry(new_name, old_inum);

    auto old_inode = priv->fs->get_inode(old_inum);

    old_dir.del_entry(old_name);

    if (old_inode->get_hardlinks() == 0) {
        old_inode->truncate(0);
        old_inode.reset();
        priv->fs->remove_inode(old_inum);
    }

    return 0;
} catch (fs::not_a_directory &) {
    return -ENOTDIR;
} catch (fs::name_too_long &) {
    return -ENAMETOOLONG;
} catch (fs::out_of_space_error&) {
    return -ENOSPC;
} catch (fs::file_too_big_error&) {
    return -EFBIG;
}
}


auto main(int argc, char **argv) -> int {
    std::unique_ptr<fuse_operations> ops = std::make_unique<fuse_operations>();

    fuse_operations *p_ops = ops.get();

    /* Filesystem */
    p_ops->init = fs_init;
    p_ops->destroy = fs_destroy;
    p_ops->statfs = fs_statfs;

    /* Open/close  */
    p_ops->open = fs_open;
    p_ops->release = fs_release;

    /* Attibutes */
    p_ops->getattr = fs_getattr;
    p_ops->utimens = fs_settimes;
    p_ops->chmod = fs_chmod;
    p_ops->chown = fs_chown;

    /* Read/write */
    p_ops->read = fs_read;
    p_ops->write = fs_write;

    /* New file / rewriting a file */
    p_ops->create = fs_create;
    p_ops->truncate = fs_truncate;

    /* Symlink stuff */
    p_ops->symlink = fs_symlink;
    p_ops->readlink = fs_readlink;

    /* Directory stuff */
    p_ops->mkdir = fs_mkdir;
    p_ops->opendir = nullptr;
    p_ops->readdir = fs_readdir;
    p_ops->rmdir = fs_rmdir;

    /* Hardlink stuff */
    p_ops->link = fs_link;
    p_ops->unlink = fs_unlink;

    /* Misc file stuff */
    p_ops->rename = fs_rename;
    p_ops->flush = nullptr;

    /* Don't know if we actually need these ones */
    p_ops->access = nullptr;
    p_ops->fsync = nullptr;
    p_ops->fsyncdir = nullptr;
    p_ops->lock = nullptr;
    p_ops->bmap = nullptr;
    p_ops->mknod = nullptr;
#if BOOST_OS_LINUX
    p_ops->ioctl = nullptr;
    p_ops->poll = nullptr;
#endif

    std::set_terminate([] {
        auto priv = get_private();
        priv->log->error("Exception thrown, trying to save everything and abort!");
        delete priv;
        std::abort();
    });

    auto fuse_stat = fuse_main(argc, argv, p_ops, nullptr);
    return fuse_stat;
}