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
 * Monothread dispatcher.
 */

#pragma once

#include "dispatcher.h"
#include "file_comp.h"

/// Single thread version of the Dispatcher
class DispatcherMonoThread : public Dispatcher
{
public:
    DispatcherMonoThread(const Context &context, std::unique_ptr<Report> report)
        : Dispatcher{context, std::move(report)}, m_fileComp{context} {};

    void postFilledReport(ReportEntry &&entry) override;
    void contentCompareWithPartialReport(ReportEntry &&entry, size_t fileSize) override;

private:
    FileCompareContent m_fileComp; ///< file content comparison
};