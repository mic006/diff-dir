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

#pragma once

#include <memory.h>

#include "context.h"

class FileCompareContent
{
public:
    FileCompareContent(const Context &context)
        : ctx{context},
          // TODO: use std::make_unique_for_overwrite when available
          m_contentBuffL{new uint8_t[context.settings.contentBufferSize]},
          m_contentBuffR{new uint8_t[context.settings.contentBufferSize]}
    {
    }

    ~FileCompareContent() = default;

    // copyable
    FileCompareContent(const FileCompareContent &other)
        : FileCompareContent{other.ctx}
    {
    }
    // not assign copyable (no default constructor)
    FileCompareContent &operator=(const FileCompareContent &) = delete;

    // movable
    FileCompareContent(FileCompareContent &&other) noexcept
        : ctx{other.ctx},
          m_contentBuffL{std::exchange(other.m_contentBuffL, nullptr)},
          m_contentBuffR{std::exchange(other.m_contentBuffR, nullptr)}
    {
    }
    // not assign movable (no default constructor)
    FileCompareContent &operator=(FileCompareContent &&) noexcept = delete;

    /** Compare the content of 2 files.
     * @param[in] relPath relative path to the files to be compared
     * @param[in] fileSize size of both files
     * @returns whether the files contents match
     */
    bool operator()(const std::string &relPath, size_t fileSize);

private:
    const Context &ctx;
    std::unique_ptr<uint8_t[]> m_contentBuffL, m_contentBuffR; ///< buffer for file content on both sides
};