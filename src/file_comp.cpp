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
 * File content comparison.
 */

#include <algorithm>
#include <fstream>

#include "file_comp.h"
#include "log.h"

bool FileCompareContent::operator()(const std::string &relPath, size_t fileSize)
{
    ScopedFd fdL = ScopedFd::openat(ctx.root[0].fd, relPath, O_RDONLY);
    ScopedFd fdR = ScopedFd::openat(ctx.root[1].fd, relPath, O_RDONLY);

    if (!fdL.isValid() or !fdR.isValid())
        return false; // cannot compare files => consider them different

    while (fileSize > 0)
    {
        ssize_t dataReadL = ::read(fdL.fd, m_contentBuffL.get(), ctx.settings.contentBufferSize);
        ssize_t dataReadR = ::read(fdR.fd, m_contentBuffR.get(), dataReadL);
        if (dataReadL < 0 or dataReadR != dataReadL)
        {
            log_errno("read", relPath);
            return false; // cannot compare files => consider them different
        }
        if (::memcmp(m_contentBuffL.get(), m_contentBuffR.get(), dataReadL) != 0)
            return false; // exit on first diff
        fileSize -= dataReadL;
    }
    return true;
}