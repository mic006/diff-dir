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

#include "dispatcher_multi.h"

DispatcherMultiThread::DispatcherMultiThread(const Context &context, std::unique_ptr<Report> report)
    : Dispatcher{context, std::move(report)},
      m_fileComp{context},
      m_reportQueue{},
      m_fileCompQueue{},
      m_reportThread{},
      m_fileCompThread{&DispatcherMultiThread::taskFileComp, this}
{
    if (m_report)
        // start report thread only when a report object is provided
        m_reportThread = std::jthread{&DispatcherMultiThread::taskReport, this};
}

DispatcherMultiThread::~DispatcherMultiThread()
{
    // close queues to terminate threads
    m_reportQueue.close();
    m_fileCompQueue.close();
}

void DispatcherMultiThread::postFilledReport(ReportEntry &&entry)
{
    checkStatusMode(entry);

    // report the entry to the report thread
    std::promise<ReportEntry> entryPromise{};
    // report is already ready: publish the value
    entryPromise.set_value(std::move(entry));
    // push the associated future
    m_reportQueue.push(std::move(entryPromise.get_future()));
}

void DispatcherMultiThread::contentCompareWithPartialReport(ReportEntry &&entry, size_t fileSize)
{
    checkStatusMode(entry);

    // dispatch the comparison to the file comp queue
    FileCompParam param{std::move(entry), fileSize};
    auto entryFuture = param.entryPromise.get_future();
    m_fileCompQueue.push(std::move(param));

    // and give the future to the report thread to maintain the display order
    if (m_report)
        m_reportQueue.push(std::move(entryFuture));
}

void DispatcherMultiThread::taskReport(void)
{
    while (true)
    {
        // get next future from queue
        auto entryOpt = m_reportQueue.get();

        if (not entryOpt.has_value())
            break; // end of task

        // get value from future
        auto entry = entryOpt->get();

        if (entry.isDifferent())
            // report
            (*m_report)(std::move(entry));
    }
}

void DispatcherMultiThread::taskFileComp(void)
{
    while (true)
    {
        // get next comparison from queue
        auto paramOpt = m_fileCompQueue.get();

        if (not paramOpt.has_value())
            break; // end of task

        // perform file comparison
        auto &param = *paramOpt;
        const bool equalContent = m_fileComp(param.entry.relPath, param.fileSize);
        if (not equalContent)
        {
            param.entry.setDifference(EntryDifference::Content);
            checkStatusMode(param.entry);
        }

        // report is now ready: publish the value
        if (m_report)
            param.entryPromise.set_value(std::move(param.entry));
    }
}