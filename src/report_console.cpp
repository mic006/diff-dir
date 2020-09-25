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
 * Report to console.
 */

#include <iostream>

#include "context.h"
#include "report_console.h"

static constexpr char indicatorNoDiff = '-';
static constexpr const char *separatorIndicatorPath = "  ";

void ReportConsole::operator()(ReportEntry &&reportEntry)
{
    if (reportEntry.isDifferent(EntryDifference::EntryType))
    {
        std::cout << FileType::repr(reportEntry.fileType1)          //
                  << " ! " << FileType::repr(reportEntry.fileType2) //
                  << separatorIndicatorPath << reportEntry.relPath << std::endl;
    }
    else
    {

        const char contentInd = reportEntry.isDifferent(EntryDifference::Content) //
                                    ? 'c'
                                    : reportEntry.isDifferent(EntryDifference::Size) //
                                          ? 's'
                                          : reportEntry.fileType1 == FileType::Directory //
                                                ? ' '
                                                : indicatorNoDiff;
        const char ownershipInd = m_settings.checkMetadata                                  //
                                      ? reportEntry.isDifferent(EntryDifference::Ownership) //
                                            ? 'o'
                                            : indicatorNoDiff
                                      : ' ';
        const char permissionsInd = m_settings.checkMetadata and not(reportEntry.fileType1 == FileType::Symlink) //
                                        ? reportEntry.isDifferent(EntryDifference::Permissions)                  //
                                              ? 'p'
                                              : indicatorNoDiff
                                        : ' ';
        std::cout << FileType::repr(reportEntry.fileType1) << " " //
                  << contentInd << ownershipInd << permissionsInd //
                  << separatorIndicatorPath << reportEntry.relPath << std::endl;
    }
}