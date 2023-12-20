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
 * Test file_comp.cpp.
 */

#include <gtest/gtest.h>

#include "../file_comp.h"
#include "../report.h"

/// Test with same content
TEST(FileCompTest, all)
{
    YAML::Node config{};
    Context ctx{{false, false, 4096 * 16}, config};
    for (int side = 0; side < 2; side++)
        ctx.root[side] = RootPath{"."}; // use current working directory
    FileCompareContent fileComp{ctx};

    const std::string relPath = "build/test-diff-dir";
    struct stat statbuff;
    ctx.root[0].lstat(relPath, statbuff);
    const size_t size = statbuff.st_size;

    EXPECT_TRUE(fileComp(relPath, size));
}