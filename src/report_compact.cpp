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
 * Compact report to stdout/console.
 */

#include <iostream>

#include "report.h"

/// Compact report to stdout/console.
class ReportCompact : public Report
{
public:
    ReportCompact(const Context &_ctx)
        : Report{_ctx} {}

    void operator()(ReportEntry &&reportEntry) override;
};

static constexpr char indicatorNoDiff = '-';
static constexpr const char *separatorIndicatorPath = "  ";

void ReportCompact::operator()(ReportEntry &&reportEntry)
{
    if (reportEntry.isDifferent(EntryDifference::EntryType))
    {
        std::cout << FileType::repr(reportEntry.file[0].type)          //
                  << " ! " << FileType::repr(reportEntry.file[1].type) //
                  << separatorIndicatorPath << reportEntry.relPath << std::endl;
    }
    else
    {

        const char contentInd = reportEntry.isDifferent(EntryDifference::Content) //
                                    ? 'c'
                                    : reportEntry.isDifferent(EntryDifference::Size) //
                                          ? 's'
                                          : reportEntry.file[0].type == FileType::Directory //
                                                ? ' '
                                                : indicatorNoDiff;
        const char ownershipInd = ctx.settings.checkMetadata                                //
                                      ? reportEntry.isDifferent(EntryDifference::Ownership) //
                                            ? 'o'
                                            : indicatorNoDiff
                                      : ' ';
        const char permissionsInd = ctx.settings.checkMetadata and not(reportEntry.file[0].type == FileType::Symlink) //
                                        ? reportEntry.isDifferent(EntryDifference::Permissions)                       //
                                              ? 'p'
                                              : indicatorNoDiff
                                        : ' ';
        std::cout << FileType::repr(reportEntry.file[0].type) << " " //
                  << contentInd << ownershipInd << permissionsInd    //
                  << separatorIndicatorPath << reportEntry.relPath << std::endl;
    }
}

std::unique_ptr<Report> makeReportCompact(const Context &ctx)
{
    return std::make_unique<ReportCompact>(ctx);
}