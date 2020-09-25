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
    ScopedFd fd1 = ScopedFd::openat(ctx.root1.fd, relPath, O_RDONLY);
    ScopedFd fd2 = ScopedFd::openat(ctx.root2.fd, relPath, O_RDONLY);

    if (!fd1.isValid() or !fd2.isValid())
        return false; // cannot compare files => consider them different

    while (fileSize > 0)
    {
        ssize_t dataRead = ::read(fd1.fd, m_contentBuff1.get(), ctx.settings.contentBufferSize);
        ssize_t dataRead2 = ::read(fd2.fd, m_contentBuff2.get(), dataRead);
        if (dataRead < 0 or dataRead2 != dataRead)
        {
            log_errno("read", relPath);
            return false; // cannot compare files => consider them different
        }
        if (::memcmp(m_contentBuff1.get(), m_contentBuff2.get(), dataRead) != 0)
            return false; // exit on first diff
        fileSize -= dataRead;
    }
    return true;
}