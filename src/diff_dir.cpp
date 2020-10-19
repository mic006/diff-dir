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
 * Directory difference algorithm.
 */

#include <algorithm>
#include <iostream>
#include <stack>
#include <sys/stat.h>
#include <unistd.h>

#include "diff_dir.h"
#include "file_comp.h"
#include "report.h"

/// Determine directories differences
struct DiffDir
{
    DiffDir(const Context &_ctx)
        : ctx{_ctx},
          dirContent{},
          dirStack{},
          currDirStack{}
    {
    }

    /** Run the comparison.
     */
    void operator()();

    /** Handle an element existing only on one side.
     * 
     * @param[in] relPath   relative path to entry
     * @param[in] fileType  type of file
     * @param[in] side      which side the file is present
     */
    inline void handle_single_side_entry(const std::string &relPath,
                                         FileType::EnumType fileType,
                                         Side side);

    /** Get the (sorted) content of both directories.
     * 
     * @param[in] dirPath relative path to roots
     */
    void get_dirs_content(const std::string &dirPath);

    /** Compare the directories content.
     *
     * @param[in] dirPath relative path to roots
     */
    void compare_dirs(const std::string &dirPath);

    const Context &ctx;                   ///< context for the comparison
    dir_content_type dirContent[2];       ///< content of the current directories on both sides
    std::stack<std::string> dirStack;     ///< relPath of directories to compare
    std::stack<std::string> currDirStack; ///< stack of sub-directories of the current directory
};

static inline std::string make_path(const std::string &dirPath, const std::string &filename)
{
    if (dirPath == ".")
        return filename; // avoid ./ ahead of the relative path for ignore filter
    return dirPath + "/" + filename;
}

void DiffDir::handle_single_side_entry(const std::string &relPath, FileType::EnumType fileType, Side side)
{
    if (ctx.ignoreFilter.has_value() and ctx.ignoreFilter->isIgnored(relPath))
    {
        if (ctx.settings.debug)
        {
            std::cerr << "Ignoring on one side: " << relPath << std::endl;
        }
    }
    else
    {
        ReportEntry reportEntry{relPath};
        reportEntry.setDifference(EntryDifference::EntryType);
        FileEntry &file = reportEntry.file[int(side)];
        file.set(ctx.root[int(side)], relPath, fileType);
        ctx.dispatcher->postFilledReport(std::move(reportEntry));
    }
}

void DiffDir::get_dirs_content(const std::string &dirPath)
{
    // get directories content
    for (int side = 0; side < 2; side++)
        ctx.root[side].getSortedDirContent(dirPath, dirContent[side]);

    if (ctx.settings.debug)
    {
        std::cerr << "Dir: '" << dirPath << "' "
                  << dirContent[0].size() << " elem <-> " << dirContent[1].size() << " elem"
                  << std::endl;
    }
}

void DiffDir::compare_dirs(const std::string &dirPath)
{
    // go through the 2 sorted directory entries
    auto itDirL = dirContent[0].cbegin();
    auto itDirR = dirContent[1].cbegin();

    while (itDirL != dirContent[0].cend() and itDirR != dirContent[1].cend())
    {
        const std::string &nameL = itDirL->filename;
        const std::string &nameR = itDirR->filename;

        if (nameL < nameR)
        {
            handle_single_side_entry(make_path(dirPath, nameL), itDirL->fileType, Side::Left);
            itDirL++;
        }
        else if (nameL > nameR)
        {
            handle_single_side_entry(make_path(dirPath, nameR), itDirR->fileType, Side::Right);
            itDirR++;
        }
        else // nameL == nameR
        {
            const std::string relPath = make_path(dirPath, nameL);
            if (ctx.ignoreFilter.has_value() and ctx.ignoreFilter->isIgnored(relPath))
            {
                if (ctx.settings.debug)
                {
                    std::cerr << "Ignoring on both sides: " << relPath << std::endl;
                }
            }
            else
            {
                const FileType::EnumType fileTypeL = itDirL->fileType;
                const FileType::EnumType fileTypeR = itDirR->fileType;
                ReportEntry reportEntry{relPath};
                reportEntry.file[0].set(ctx.root[0], relPath, fileTypeL);
                reportEntry.file[1].set(ctx.root[1], relPath, fileTypeR);

                if (fileTypeL != fileTypeR)
                {
                    // type mismatch
                    reportEntry.setDifference(EntryDifference::EntryType);
                    ctx.dispatcher->postFilledReport(std::move(reportEntry));
                }
                else
                {
                    // same filetype
                    // compare metadata
                    if (ctx.settings.checkMetadata)
                    {
                        // check owership
                        if (reportEntry.file[0].lstat.st_uid != reportEntry.file[1].lstat.st_uid or
                            reportEntry.file[0].lstat.st_gid != reportEntry.file[1].lstat.st_gid)
                        {
                            reportEntry.setDifference(EntryDifference::Ownership);
                        }

                        // check permissions
                        if (reportEntry.file[0].lstat.st_mode != reportEntry.file[1].lstat.st_mode)
                        {
                            reportEntry.setDifference(EntryDifference::Permissions);
                        }
                    }

                    // comparison based on filetype
                    switch (fileTypeL)
                    {
                    case FileType::Directory:
                        // directories: add to queue for comparison
                        currDirStack.emplace(relPath);
                        break;

                    case FileType::Regular:
                        // regular files: compare size, m_time then content
                        if (reportEntry.file[0].lstat.st_size != reportEntry.file[1].lstat.st_size)
                        {
                            // size is different
                            reportEntry.setDifference(EntryDifference::Size);
                        }
                        else if (reportEntry.file[0].lstat.st_size > 0 and
                                 reportEntry.file[0].lstat.st_mtim != reportEntry.file[1].lstat.st_mtim)
                        {
                            /* m_time are different, but files have the same size (> 0, with real content)
                             * => check file content to see whether they are really different
                             */
                            if (ctx.settings.debug)
                            {
                                std::cerr << "File with same size but different m_time, checking content: " << relPath << std::endl;
                            }
                            ctx.dispatcher->contentCompareWithPartialReport(std::move(reportEntry), reportEntry.file[0].lstat.st_size);
                            // report has been done, clear so that it is not done twice
                            reportEntry.clear();
                        }
                        break;

                    case FileType::Symlink:
                    {
                        // symlinks: compare the target names
                        if (reportEntry.file[0].symlinkTarget != reportEntry.file[1].symlinkTarget)
                        {
                            reportEntry.setDifference(EntryDifference::Content);
                        }
                        break;
                    }

                    default:
                        // TODO: additional checks for other types ?
                        break;
                    }

                    if (reportEntry.isDifferent())
                    {
                        ctx.dispatcher->postFilledReport(std::move(reportEntry));
                    }
                }
            }
            itDirL++;
            itDirR++;
        }
    }

    // process remaining items on one side or the other
    while (itDirL != dirContent[0].cend())
    {
        handle_single_side_entry(make_path(dirPath, itDirL->filename), itDirL->fileType, Side::Left);
        itDirL++;
    }
    while (itDirR != dirContent[1].cend())
    {
        handle_single_side_entry(make_path(dirPath, itDirR->filename), itDirR->fileType, Side::Right);
        itDirR++;
    }

    // add sub directories to the stack, in the proper order
    while (not currDirStack.empty())
    {
        dirStack.emplace(std::move(currDirStack.top()));
        currDirStack.pop();
    }
}

void DiffDir::operator()()
{
    dirStack.emplace("."); // start with empty relPath = root

    while (not dirStack.empty())
    {
        const std::string dirPath = std::move(dirStack.top());
        dirStack.pop();

        get_dirs_content(dirPath);
        compare_dirs(dirPath);
    }
}

void diff_dirs(const Context &ctx)
{
    DiffDir(ctx).operator()();
}