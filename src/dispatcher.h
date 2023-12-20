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
 * Dispatcher for reports and file comparison.
 */

#pragma once

#include "report.h"

// forward reference
class Context;
class Report;
struct ReportEntry;

/** Dispatcher for reports and file comparison.
 * Base class to limit the different processing between single thread
 * and multithread operations.
 */
class Dispatcher
{
public:
    Dispatcher(const Context &context, std::unique_ptr<Report> report) : ctx{context}, m_report{std::move(report)} {};

    virtual ~Dispatcher() = default;

    // not copyable
    Dispatcher(const Dispatcher &) = delete;
    Dispatcher &operator=(const Dispatcher &) = delete;

    // not movable
    Dispatcher(Dispatcher &&) noexcept = delete;
    Dispatcher &operator=(Dispatcher &&) noexcept = delete;

    /// Post report that is already filled
    virtual void postFilledReport(ReportEntry &&entry) = 0;

    /** Request comparison of the content of 2 files, and post report
     * @param[in] entry report entry partially filled
     * @param[in] fileSize size of both files
     */
    virtual void contentCompareWithPartialReport(ReportEntry &&entry, size_t fileSize) = 0;

protected:
    void checkStatusMode(const ReportEntry &entry) const;

    const Context &ctx;
    std::unique_ptr<Report> m_report; ///< report handler
};

/// Build a monothread dispatcher
std::unique_ptr<Dispatcher> makeDispatcherMono(const Context &context, std::unique_ptr<Report> report);

/// Build a multithread dispatcher
std::unique_ptr<Dispatcher> makeDispatcherMulti(const Context &context, std::unique_ptr<Report> report);