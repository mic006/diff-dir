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

#include "dispatcher_mono.h"

void DispatcherMonoThread::postFilledReport(ReportEntry &&entry)
{
    checkStatusMode(entry);

    m_report(std::move(entry));
}

void DispatcherMonoThread::contentCompareWithPartialReport(ReportEntry &&entry, size_t fileSize)
{
    checkStatusMode(entry);

    // perform content comparison
    const bool equalContent = m_fileComp(entry.relPath, fileSize);
    if (not equalContent)
        entry.setDifference(EntryDifference::Content);

    // post report
    if (entry.isDifferent())
        postFilledReport(std::move(entry));
}