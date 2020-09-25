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

#include <algorithm>
#include <dirent.h>

#include "path.h"

namespace dt_lut
{
    // let the compiler build the LookUp Table
    constexpr uint32_t dtValues[] = {DT_FIFO, DT_CHR, DT_DIR, DT_BLK, DT_REG, DT_LNK, DT_SOCK};
    constexpr uint32_t maxDt = *std::max_element(std::begin(dtValues), std::end(dtValues));
    typedef std::array<FileType::EnumType, maxDt + 1> dt_lut_type;

    constexpr dt_lut_type getLut()
    {
        dt_lut_type lut{};
        lut[DT_FIFO] = FileType::Fifo;
        lut[DT_CHR] = FileType::Character;
        lut[DT_DIR] = FileType::Directory;
        lut[DT_BLK] = FileType::Block;
        lut[DT_REG] = FileType::Regular;
        lut[DT_LNK] = FileType::Symlink;
        lut[DT_SOCK] = FileType::Socket;
        return lut;
    };
} // namespace dt_lut

/// Convert DT to FileType
static FileType::EnumType filetype_from_dt(uint8_t dt)
{
    static constexpr dt_lut::dt_lut_type lut = dt_lut::getLut();
    if (dt <= dt_lut::maxDt)
        return lut[dt];
    return FileType::Unknown;
}

void RootPath::getSortedDirContent(const std::string &relPath, dir_content_type &result) const
{
    result.clear();
    const int dirFd = ::openat(fd, relPath.c_str(), O_RDONLY | O_DIRECTORY);
    if (dirFd < 0)
    {
        log_errno("openat", relPath);
        return;
    }
    DIR *dirHandle = ::fdopendir(dirFd);
    if (dirHandle == nullptr)
    {
        log_errno("fdopendir", relPath);
        ::close(dirFd);
        return;
    }

    const struct dirent *dirEntry;
    while ((dirEntry = ::readdir(dirHandle)) != nullptr)
    {
        std::string filename = dirEntry->d_name;
        if (filename != "." and filename != "..")
        {
            result.emplace_back(
                dirEntry->d_name,
                filetype_from_dt(dirEntry->d_type));
        }
    }

    ::closedir(dirHandle);

    // sort by filename
    std::sort(result.begin(), result.end());
}
