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
 * Test ignore.cpp.
 */

#include <gtest/gtest.h>

#include "../ignore.h"

/// Description of one test case
struct IgnoreFilterTestCase
{
    std::string rule;                    ///< rule to create a IgnoreFilter object
    std::vector<std::string> ignored;    ///< values that shall be ignored
    std::vector<std::string> notIgnored; ///< values that shall NOT be ignored
};

/// Test cases
static const std::vector<IgnoreFilterTestCase> testCases = {
    {
        // simplest pattern
        "a",
        {"a", "dir/a", "another_dir/subdir/a"},
        {"A", "aa", "alpha", "dir_with_a/beta", "aaaa/aa/aa"},
    },
    {
        // word pattern
        "foo",
        {"foo", "dir/foo", "another_dir/subdir/foo"},
        {"FOO", "foo.txt", "another_foo", "dir_foo/beta", "aaaa/foo_dir/aa"},
    },
    {
        // . pattern (extension)
        "linux.bak",
        {"linux.bak", "dir/linux.bak", "another_dir/subdir/linux.bak"},
        {"linux.txt", "linux-bak", ".linux.bak", "linux.bak.gz", "dir/.linux.bak"},
    },
    {
        // ? pattern (single character)
        "dish??.exe",
        {"dish00.exe", "dir/dishZZ.exe", "another_dir/subdir/dish--.exe"},
        {"dish.exe", "dish0.exe", "dish000.exe", "dish000exe"},
    },
    {
        // * pattern (multiple characters)
        "*.log",
        {"some.log", "dir/.another.file.log", "another_dir/subdir/.log"},
        {"log", "zlog"},
    },
    {
        // absolute rule
        "/cache",
        {"cache"},
        {".cache", "cache.zzz", "dir/cache"},
    },
    {
        // relative pattern with directories
        "omega/delete",
        {"omega/delete", "dir/omega/delete"},
        {"omega", "omega/bar", "bar/delete", "omega/bar/delete"},
    },
    {
        // absolute pattern with directories
        "/gamma/absolute",
        {"gamma/absolute"},
        {".gamma/absolute", "dir/gamma/absolute"},
    },
    {
        // complex pattern
        "regex.*/*/complex?.*",
        {"regex.txt/dir/complex0.zip", "dir/regex.c/dir/complexZ.x.y"},
        {"regex.txt", "regex.txt/dir/complex", "regex.txt/dir/complex.z", "regex.txt/dir/sub_dir/complex0.zip", "dir/regex/dir/complex0.zip"},
    },
};

/// Test one rule at a time
TEST(IgnoreFilterTest, one_rule)
{
    for (const auto &testCase : testCases)
    {
        std::vector<std::string> rules = {testCase.rule};
        IgnoreFilter filter{rules};

        for (const auto &testPath : testCase.ignored)
            EXPECT_TRUE(filter.isIgnored(testPath));

        for (const auto &testPath : testCase.notIgnored)
            EXPECT_FALSE(filter.isIgnored(testPath));
    }
}

/// Test all rules together
TEST(IgnoreFilterTest, all_rules)
{
    // build rules list
    std::vector<std::string> rules{};
    for (const auto &testCase : testCases)
        rules.push_back(testCase.rule);

    IgnoreFilter filter{rules};

    // run the test of each test case
    for (const auto &testCase : testCases)
    {
        for (const auto &testPath : testCase.ignored)
            EXPECT_TRUE(filter.isIgnored(testPath));

        for (const auto &testPath : testCase.notIgnored)
            EXPECT_FALSE(filter.isIgnored(testPath));
    }
}