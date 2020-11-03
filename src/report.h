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

/// One file in the report entry
struct FileEntry
{
    FileEntry()
        : type(FileType::NoFile), symlinkTarget{} {}

    void set(const RootPath &root, const std::string &relPath, FileType::EnumType fileType)
    {
        type = fileType;
        root.lstat(relPath, lstat);
        if (fileType == FileType::Symlink)
            symlinkTarget = root.readSymlink(relPath, lstat.st_size);
    }

    /// Permissions to string
    std::string permissions() const;

    /// File size to string
    std::string size() const;

    /// Time to string
    std::string mtime() const;

    FileType::EnumType type;   ///< type of the file
    struct stat lstat;         ///< lstat of the file
    std::string symlinkTarget; ///< symlink target when type == Symlink
};

/// Entry to report a difference
struct ReportEntry
{
    ReportEntry(const std::string &_relPath)
        : relPath{_relPath},
          diffBitmap{0},
          file{}
    {
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

    std::string relPath; ///< relative path of files being compared
    uint32_t diffBitmap; ///< bitmap of EntryDifferences, indicating all the differences between left/right sides
    FileEntry file[2];   ///< information of the file on each side
};

// forward reference
class Context;

class Report
{
public:
    Report(const Context &_ctx)
        : ctx{_ctx} {}

    virtual ~Report() = default;

    // not copyable
    Report(const Report &) = delete;
    Report &operator=(const Report &) = delete;

    // not movable
    Report(Report &&) noexcept = delete;
    Report &operator=(Report &&) noexcept = delete;

    /** Report a difference.
     */
    virtual void operator()(ReportEntry &&reportEntry) = 0;

protected:
    const Context &ctx; ///< context for the comparison
};

/// Build a ReportCompact: compact report to stdout/console
std::unique_ptr<Report> makeReportCompact(const Context &ctx);

/// Build a ReportInteractive: interactive terminal application
std::unique_ptr<Report> makeReportInteractive(Context &ctx);