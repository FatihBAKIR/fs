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

auto from_tspec(const timespec& ts)
{
    std::chrono::system_clock::time_point tp;
    tp += std::chrono::seconds(ts.tv_sec);
    tp += std::chrono::nanoseconds(ts.tv_nsec);
    return tp;
}

int create_inode(fs::fs_instance& fs, const char* p)
{
    auto v = boost::string_view(p);
    auto last = v.rfind('/');
    auto to_find = v.substr(0, last);
    auto name = v.substr(last + 1, v.size());

    auto dir_inum = fs::lookup(fs, to_find);
    if (dir_inum == 0)
    {
        return -ENOENT;
    }

    auto dir = fs::directory(fs.get_inode(dir_inum));

    if (dir.find(name) != dir.end())
    {
        return -EEXIST;
    }

    auto inum = fs.create_inode();
    dir.add_entry(name, inum);

    return inum;
}

extern "C" {
struct fs_opaque;

void *fs_init(struct fuse_conn_info *conn) {
    auto ctx = fuse_get_context();

    std::cout << ctx->private_data << '\n';

    auto fs = fs::fs_instance::load_heap(
            std::make_unique<fs::mmap_block_dev>("/tmp/fs", 1LL * 1024 * 1024 * 1024, 4096));

    auto priv = new fs_fuse_private(std::move(fs));
    priv->log = spdlog::stderr_color_st("fslog");

    return priv;
}

void fs_destroy(void* arg)
{
    auto fsp = static_cast<fs_fuse_private *>(arg);
    delete fsp;
}

int fs_open(const char *p, fuse_file_info *fi) {
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

int fs_read(const char *p, char *buf, size_t sz, off_t off, fuse_file_info *fi) {
    auto priv = get_private();
    priv->log->info("Reading {}, {} bytes from {}", p, sz, off);

    auto inode = priv->fs->get_inode(fi->fh);
    return inode->read(off, buf, sz);
}

int fs_write(const char *p, const char *buf, size_t sz, off_t off, fuse_file_info *fi) {
    auto priv = get_private();
    priv->log->info("Writing {}, {} bytes from {}", p, sz, off);
    fs::inode_ptr inode = priv->fs->get_inode(fi->fh);
    inode->write(off, buf, sz);
    return sz;
}

int fs_chmod(const char *p, mode_t m) {
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
int fs_chown(const char *p, uid_t u, gid_t g) {
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

int fs_getattr(const char *path, struct stat *stbuf) {
    auto priv = get_private();
    priv->log->info("Getattr {}", path);

    auto inum = fs::lookup(*priv->fs, path);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr inode = priv->fs->get_inode(inum);

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
    stbuf->st_ctim = to_tspec(inode->get_creation_time());
#elif BOOST_OS_MACOS
    stbuf->st_atimespec = to_tspec(inode->get_access_time());
    stbuf->st_mtimespec = to_tspec(inode->get_modification_time());
    stbuf->st_ctimespec = to_tspec(inode->get_creation_time());
#endif

    stbuf->st_blksize = priv->fs->blk_cache()->device()->get_block_size();
    stbuf->st_blocks = inode->capacity() / stbuf->st_blksize;
    stbuf->st_uid = inode->get_owner();
    stbuf->st_gid = inode->get_group();
    return 0;
}

int fs_release(const char *, fuse_file_info *fi) {
    auto priv = get_private();
    priv->log->info("Closed -> {}", fi->fh);
    return 0;
}

#if BOOST_OS_MACOS
using access_mode_t = fd_mask;
#elif BOOST_OS_LINUX
using access_mode_t = int;
#endif

int fs_access(const char* p, access_mode_t mode)
{
    return 0;
}

int fs_truncate(const char* p, off_t sz)
{
    auto priv = get_private();
    priv->log->info("Truncate {}, {}", p, sz);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr in = priv->fs->get_inode(inum);
    in->truncate(sz);
    return 0;
}


int fs_create(const char* p, mode_t m, fuse_file_info* fi)
{
    auto priv = get_private();
    priv->log->info("Create {}", p);

    auto inum = create_inode(*priv->fs, p);
    if (inum < 0)
    {
        return inum;
    }

    fs::inode_ptr inode = priv->fs->get_inode(inum);
    inode->set_owner(fuse_get_context()->uid);
    inode->set_group(fuse_get_context()->gid);
    inode->set_mode(m);
    fi->fh = inum;
    return 0;
}

int fs_settimes(const char* p, const timespec* ts)
{
    auto priv = get_private();
    priv->log->info("Update times {}, {}", p);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr inode = priv->fs->get_inode(inum);
    inode->set_times(inode->get_creation_time(), from_tspec(ts[1]), from_tspec(ts[0]));

    return 0;
}

int fs_symlink(const char* target, const char* p)
{
    auto priv = get_private();
    priv->log->info("Symlink \"{}\" -> \"{}\"", p, target);

    auto inum = create_inode(*priv->fs, p);
    if (inum < 0)
    {
        return inum;
    }

    fs::inode_ptr inode = priv->fs->get_inode(inum);
    inode->set_owner(fuse_get_context()->uid);
    inode->set_group(fuse_get_context()->gid);
    inode->set_type(fs::inode_type::symlink);
    inode->write(0, target, strlen(target) + 1);
    return 0;
}

int fs_readlink(const char* p, char* buf, size_t bufsz)
{
    auto priv = get_private();
    priv->log->info("Readlink \"{}\" -> \"{}\"", p);

    auto inum = fs::lookup(*priv->fs, p);
    if (inum == 0) {
        return -ENOENT;
    }

    fs::inode_ptr in = priv->fs->get_inode(inum);
    auto len = in->read(0, buf, bufsz);
    return 0;
}
}


auto main(int argc, char **argv) -> int {
    std::unique_ptr<fuse_operations> ops = std::make_unique<fuse_operations>();
    std::cout << "fuse version: " << fuse_version() << '\n';

    fuse_operations *p_ops = ops.get();

    /* Filesystem */
    p_ops->init     = fs_init;
    p_ops->destroy  = fs_destroy;
    p_ops->statfs   = nullptr;

    /* Open/close  */
    p_ops->open     = fs_open;
    p_ops->release  = fs_release;

    /* Attibutes */
    p_ops->getattr  = fs_getattr;
    p_ops->utimens  = fs_settimes;
    p_ops->chmod    = fs_chmod;
    p_ops->chown    = fs_chown;

    /* Read/write */
    p_ops->read     = fs_read;
    p_ops->write    = fs_write;

    /* New file / rewriting a file */
    p_ops->create   = fs_create;
    p_ops->truncate = fs_truncate;

    /* Symlink stuff */
    p_ops->symlink  = fs_symlink;
    p_ops->readlink = fs_readlink;

    /* Directory stuff */
    p_ops->mkdir    = nullptr;
    p_ops->opendir  = nullptr;
    p_ops->readdir  = nullptr;
    p_ops->rmdir    = nullptr;

    /* Hardlink stuff */
    p_ops->link     = nullptr;
    p_ops->unlink   = nullptr;

    /* Misc file stuff */
    p_ops->rename   = nullptr;
    p_ops->flush    = nullptr;

    /* Don't know if we actually need these ones */
    p_ops->access   = fs_access;
    p_ops->fsync    = nullptr;
    p_ops->fsyncdir = nullptr;
    p_ops->lock     = nullptr;
    p_ops->bmap     = nullptr;
    p_ops->ioctl    = nullptr;
    p_ops->poll     = nullptr;
    p_ops->mknod    = nullptr;

    std::cout << p_ops << '\n';
    auto fuse_stat = fuse_main(argc, argv, p_ops, p_ops);
    return fuse_stat;
}