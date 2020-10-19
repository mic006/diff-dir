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
 * Interactive report using terminal.
 */

#include "report.h"
#include "term_app.h"

/// Interactive report using terminal
struct ReportInteractive : public Report
{
    ReportInteractive(const Context &_ctx);
    ~ReportInteractive() override = default;

    void operator()(ReportEntry &&reportEntry) override;

    TermApp app;
};

ReportInteractive::ReportInteractive(const Context &_ctx)
    : Report{_ctx}, app{_ctx, "DiffDir: " + ctx.root[0].path + " <-> " + ctx.root[1].path}
{
}

void ReportInteractive::operator()(ReportEntry &&reportEntry)
{
    app.reportQueue.push(std::move(reportEntry));
}

std::unique_ptr<Report> makeReportInteractive(const Context &ctx)
{
    return std::make_unique<ReportInteractive>(ctx);
}