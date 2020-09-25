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
 * Multithread dispatcher.
 */

#pragma once

#include <future>
#include <thread>

#include "concurrent.h"
#include "dispatcher.h"
#include "file_comp.h"

/// Parameters to manage file comparison in a dedicated thread
struct FileCompParam
{
    FileCompParam(ReportEntry &&_entry, size_t _fileSize)
        : entry{std::move(_entry)}, fileSize{_fileSize} {}

    ~FileCompParam() = default;

    // not copyable (detect unwanted copies)
    FileCompParam(const FileCompParam &) = delete;
    FileCompParam &operator=(const FileCompParam &) = delete;

    // movable
    FileCompParam(FileCompParam &&) noexcept = default;
    FileCompParam &operator=(FileCompParam &&) noexcept = default;

    std::promise<ReportEntry> entryPromise; ///< promise for the report entry
    ReportEntry entry;                      ///< report entry pre-filled by diff_dir
    size_t fileSize;                        ///< common size of both files
};

/// Multi-threads version of the Dispatcher
class DispatcherMultiThread : public Dispatcher
{
public:
    DispatcherMultiThread(const Context &context, Report &report);
    ~DispatcherMultiThread() override;

    void postFilledReport(ReportEntry &&entry) override;
    void contentCompareWithPartialReport(ReportEntry &&entry, size_t fileSize) override;

private:
    /// Threaded task managing reports
    void taskReport(void);
    /// Threaded task managing file comparison
    void taskFileComp(void);

    FileCompareContent m_fileComp;                           ///< file content comparison
    ConcurrentQueue<std::future<ReportEntry>> m_reportQueue; ///< queue for reports
    ConcurrentQueue<FileCompParam> m_fileCompQueue;          ///< queue for file comparison
    std::jthread m_reportThread;                             ///< reports handling thread
    std::jthread m_fileCompThread;                           ///< file comparison handling thread
};