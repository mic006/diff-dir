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
 * Generic report object.
 */

#include "report.h"

std::string FileEntry::permissions() const
{
    // from https://stackoverflow.com/a/10323131
    static const char *rwx[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};

    std::string result{};
    result.reserve(9);
    const mode_t mode = lstat.st_mode;

    result += rwx[(mode >> 6) & 7]; // user
    result += rwx[(mode >> 3) & 7]; // group
    result += rwx[mode & 7];        // other
    if ((mode & S_ISUID) != 0)
        result[2] = (mode & S_IXUSR) != 0 ? 's' : 'S';
    if ((mode & S_ISGID) != 0)
        result[5] = (mode & S_IXGRP) != 0 ? 's' : 'l';
    if ((mode & S_ISVTX) != 0)
        result[8] = (mode & S_IXOTH) != 0 ? 't' : 'T';

    return result;
}

std::string FileEntry::size() const
{
    static constexpr char unitChar[] = {'K', 'M', 'G', 'T', 'P', 'E'};

    const off_t size = lstat.st_size;

    if (size < 1024)
        return std::to_string(size) + " o";

    // convert large numbers to human readable format
    float mantissa = size;
    int unit = -1;
    while (mantissa >= 1024)
    {
        mantissa /= 1024;
        unit++;
    }

    // build the result
    std::string result{};
    result.reserve(9);
    result.resize(5);
    int precision = mantissa >= 99.5 ? 0 : mantissa >= 9.95 ? 1 : mantissa >= 0.995 ? 2 : 3;
    int written = snprintf(result.data(), result.size(), "%.*f", precision, mantissa);
    result.resize(written);
    // add unit
    result += " ";
    result += unitChar[unit];
    result += "io";

    return result;
}

std::string FileEntry::mtime() const
{
    std::string result{};
    result.resize(30);
    int written = strftime(result.data(), result.size(), "%F %T %Z", std::localtime(&lstat.st_mtim.tv_sec));
    result.resize(written);
    return result;
}