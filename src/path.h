/*
Copyright 2020 Michel Palleau

This file is part of diff-dir.

diff-dir is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

diff-dir is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with diff-dir. If not, see <https://www.gnu.org/licenses/>.
*/

/** @file
 *
 * Manage paths.
 */

#pragma once

#include <fcntl.h>
#include <limits.h>
#include <map>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"

/// type of files
struct FileType
{
    enum EnumType
    {
        NoFile = 0, ///< file does not exist
        Regular,    ///< regular file
        Directory,  ///< directory
        Block,      ///< block device
        Character,  ///< character device
        Fifo,       ///< named pipe
        Symlink,    ///< symbolic link
        Socket,     ///< UNIX domain socket
        Unknown,    ///< unspecified
        NbElem      ///< LAST: number of elements
    };

    /// Get FileType representation as a char
    static inline constexpr char repr(EnumType fileType)
    {
        return lut[fileType];
    }

private:
    /// LUT giving a char representation
    static constexpr char lut[NbElem] = {
        '-', // NoFile
        'f', // Regular
        'd', // Directory
        'b', // Block
        'c', // Character
        'F', // Fifo
        'l', // Symlink
        's', // Socket
        '?', // Unknown
    };
};

/// Encapsulate a file handle to ensure closing
struct ScopedFd
{
    static ScopedFd open(const std::string &path, int flags)
    {
        const int fd = ::open(path.c_str(), flags);
        if (fd < 0)
            log_errno("open", path);
        return ScopedFd{fd};
    }

    static ScopedFd openat(int rootFd, const std::string &relPath, int flags)
    {
        const int fd = ::openat(rootFd, relPath.c_str(), flags);
        if (fd < 0)
            log_errno("openat", relPath);
        return ScopedFd{fd};
    }

    ScopedFd()
        : fd{-1} {}
    ScopedFd(int _fd)
        : fd{_fd} {}
    ~ScopedFd()
    {
        if (fd >= 0)
            ::close(fd);
    }

    // not copyable
    ScopedFd(const ScopedFd &) = delete;
    ScopedFd &operator=(const ScopedFd &) = delete;

    // movable
    ScopedFd(ScopedFd &&other) noexcept
        : fd{std::exchange(other.fd, -1)}
    {
    }
    ScopedFd &operator=(ScopedFd &&other) noexcept
    {
        std::swap(fd, other.fd);
        return *this;
    }

    bool isValid() const
    {
        return fd >= 0;
    }

    /// Get file content as string
    std::string getContent();

    int fd; ///< file handle
};

/// Directory content
struct DirEntry;
typedef std::vector<DirEntry> dir_content_type;

/// Path and file manipulation for the purpose of DiffDir
struct DirEntry
{
    explicit DirEntry(
        const std::string &_filename,
        FileType::EnumType _fileType = FileType::Unknown)
        : filename{_filename},
          fileType{_fileType}
    {
    }

    std::string filename;        ///< filename
    FileType::EnumType fileType; ///< type of the file

    // DirEntry comparisons
    auto operator<=>(const DirEntry &) const = default;
};

struct RootPath : public ScopedFd
{
    RootPath() = default;
    RootPath(const std::string rootPath)
        : ScopedFd(ScopedFd::open(rootPath, O_RDONLY | O_DIRECTORY | O_PATH)), path{rootPath}
    {
    }
    ~RootPath() = default;

    // not copyable
    RootPath(const RootPath &) = delete;
    RootPath &operator=(const RootPath &) = delete;

    // movable
    RootPath(RootPath &&) noexcept = default;
    RootPath &operator=(RootPath &&) noexcept = default;

    /** Get a (sub-)directory content.
     */
    void getSortedDirContent(const std::string &relPath, dir_content_type &result) const;

    /** Get file lstat.
     */
    void lstat(const std::string &relPath, struct stat &statbuf) const
    {
        if (::fstatat(fd, relPath.c_str(), &statbuf, AT_NO_AUTOMOUNT | AT_SYMLINK_NOFOLLOW) < 0)
            log_errno("fstatat", relPath);
    }

    /** Get file symlink target.
     */
    std::string readSymlink(const std::string &relPath, size_t size = 0) const
    {
        if (size == 0)
            size = PATH_MAX;
        else
            size++; // add room for a final \0
        char buff[size];
        ssize_t res = ::readlinkat(fd, relPath.c_str(), buff, size);
        if (res < 0)
        {
            log_errno("readlinkat", relPath);
            return {};
        }
        // add the terminating \0, not done by readlinkat
        if ((size_t)res == size)
            res--;
        buff[res] = '\0';
        return std::string(buff);
    }

    std::string path; ///< filesystem path
};

// Timespec comparisons
inline bool operator==(const struct timespec &lhs, const struct timespec &rhs)
{
    return lhs.tv_sec == rhs.tv_sec and lhs.tv_nsec == rhs.tv_nsec;
}
inline bool operator!=(const struct timespec &lhs, const struct timespec &rhs)
{
    return !operator==(lhs, rhs);
}

/// Convert uid to gid to name
class UidGidNameReader
{
public:
    /// Get the name of the given uid
    const std::string &getUidName(uid_t uid);

    /// Get the name of the given gid
    const std::string &getGidName(gid_t gid);

private:
    std::map<uid_t, std::string> m_uidNames; ///< map uid to name
    std::map<gid_t, std::string> m_gidNames; ///< map gid to name
};