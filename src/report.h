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

#pragma once

#include "context.h"
#include "path.h"

/// kind of difference for an entry
enum class EntryDifference : int
{
    EntryType, ///< file types are different
    // below differences are only applicable if the files have the same type
    Ownership,   ///< owner or group
    Permissions, ///< rwx for owner, group or other
    Content,     ///< same size but different content (regular), different target (symlink)
    Size,        ///< different size
};

/// Entry to report a difference
struct ReportEntry
{
    /// Constructor for a file with same type on both sides
    ReportEntry(const std::string &_relPath,
                const FileType::EnumType &_fileType)
        : relPath{_relPath},
          fileType1{_fileType},
          fileType2{FileType::NoFile},
          diffBitmap{0}
    {
    }

    /// Constructor for a file with different types (or existing on a single side)
    ReportEntry(const std::string &_relPath,
                const FileType::EnumType &_fileType1,
                const FileType::EnumType &_fileType2)
        : relPath{_relPath},
          fileType1{_fileType1},
          fileType2{_fileType2},
          diffBitmap{0}
    {
        setDifference(EntryDifference::EntryType);
    }

    ~ReportEntry() = default;

    // not copyable (detect unwanted copies)
    ReportEntry(const ReportEntry &) = delete;
    ReportEntry &operator=(const ReportEntry &) = delete;

    // movable
    ReportEntry(ReportEntry &&) noexcept = default;
    ReportEntry &operator=(ReportEntry &&) noexcept = default;

    /// Get whether any difference have been recorded for the current file.
    bool isDifferent() const
    {
        return diffBitmap != 0;
    }

    bool isDifferent(EntryDifference entryDiff) const
    {
        return (diffBitmap & (1 << int(entryDiff))) != 0;
    }

    void setDifference(EntryDifference entryDiff)
    {
        diffBitmap |= 1 << int(entryDiff);
    }

    void clear()
    {
        diffBitmap = 0;
    }

    std::string relPath;                     ///< relative path of files being compared
    FileType::EnumType fileType1, fileType2; ///< type of the files being compared
    uint32_t diffBitmap;                     ///< bitmap of EntryDifferences, indicating all the differences between left/right sides
};

// forward reference
class Settings;

class Report
{
public:
    Report(const Settings &settings)
        : m_settings{settings} {}

    /** Report a difference.
     */
    virtual void operator()(ReportEntry &&reportEntry) = 0;

protected:
    const Settings &m_settings;
};

/// Build a ReportCompact: compact report to stdout/console
std::unique_ptr<Report> makeReportCompact(const Settings &settings);