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
          dirContent1{},
          dirContent2{},
          dirStack{},
          currDirStack{}
    {
    }

    /** Run the comparison.
     */
    void operator()();

    /** Handle an element existing only on one side.
     * 
     * @param[in] relPath relative path to entry
     * @param[in] fileType1 type of file on left side
     * @param[in] fileType2 type of file on right side
     */
    inline void handle_single_side_entry(const std::string &relPath, const FileType::EnumType &fileType1, const FileType::EnumType &fileType2);

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

    const Context &ctx;                        ///< context for the comparison
    dir_content_type dirContent1, dirContent2; ///< content of the current directories on both sides
    std::stack<std::string> dirStack;          ///< relPath of directories to compare
    std::stack<std::string> currDirStack;      ///< stack of sub-directories of the current directory
};

static inline std::string make_path(const std::string &dirPath, const std::string &filename)
{
    if (dirPath == ".")
        return filename; // avoid ./ ahead of the relative path for ignore filter
    return dirPath + "/" + filename;
}

void DiffDir::handle_single_side_entry(const std::string &relPath, const FileType::EnumType &fileType1, const FileType::EnumType &fileType2)
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
        ReportEntry reportEntry{relPath, fileType1, fileType2};
        ctx.dispatcher->postFilledReport(std::move(reportEntry));
    }
}

void DiffDir::get_dirs_content(const std::string &dirPath)
{
    // get directories content
    ctx.root1.getSortedDirContent(dirPath, dirContent1);
    ctx.root2.getSortedDirContent(dirPath, dirContent2);

    if (ctx.settings.debug)
    {
        std::cerr << "Dir: '" << dirPath << "' "
                  << dirContent1.size() << " elem <-> " << dirContent2.size() << " elem"
                  << std::endl;
    }
}

void DiffDir::compare_dirs(const std::string &dirPath)
{
    // go through the 2 sorted directory entries
    auto itDir1 = dirContent1.cbegin();
    auto itDir2 = dirContent2.cbegin();

    while (itDir1 != dirContent1.cend() and itDir2 != dirContent2.cend())
    {
        const std::string &name1 = itDir1->filename;
        const std::string &name2 = itDir2->filename;

        if (name1 < name2)
        {
            handle_single_side_entry(make_path(dirPath, name1), itDir1->fileType, FileType::NoFile);
            itDir1++;
        }
        else if (name1 > name2)
        {
            handle_single_side_entry(make_path(dirPath, name2), FileType::NoFile, itDir2->fileType);
            itDir2++;
        }
        else // name1 == name2
        {
            const std::string relPath = make_path(dirPath, name1);
            if (ctx.ignoreFilter.has_value() and ctx.ignoreFilter->isIgnored(relPath))
            {
                if (ctx.settings.debug)
                {
                    std::cerr << "Ignoring on both sides: " << relPath << std::endl;
                }
            }
            else
            {
                const FileType::EnumType &fileType1 = itDir1->fileType;
                const FileType::EnumType &fileType2 = itDir2->fileType;
                if (fileType1 != fileType2)
                {
                    // type mismatch
                    ReportEntry reportEntry{relPath, fileType1, fileType2};
                    ctx.dispatcher->postFilledReport(std::move(reportEntry));
                }
                else
                {
                    // same filetype
                    ReportEntry reportEntry{relPath, fileType1};

                    struct stat lstat1;
                    struct stat lstat2;
                    ctx.root1.lstat(relPath, lstat1);
                    ctx.root2.lstat(relPath, lstat2);

                    // compare metadata
                    if (ctx.settings.checkMetadata)
                    {
                        // check owership
                        if (lstat1.st_uid != lstat2.st_uid or lstat1.st_gid != lstat2.st_gid)
                        {
                            reportEntry.setDifference(EntryDifference::Ownership);
                        }

                        // check permissions
                        if (lstat1.st_mode != lstat2.st_mode)
                        {
                            reportEntry.setDifference(EntryDifference::Permissions);
                        }
                    }

                    // comparison based on filetype
                    switch (fileType1)
                    {
                    case FileType::Directory:
                        // directories: add to queue for comparison
                        currDirStack.emplace(relPath);
                        break;

                    case FileType::Regular:
                        // regular files: compare size, m_time then content
                        if (lstat1.st_size != lstat2.st_size)
                        {
                            // size is different
                            reportEntry.setDifference(EntryDifference::Size);
                        }
                        else if (lstat1.st_size > 0 and lstat1.st_mtim != lstat2.st_mtim)
                        {
                            /* m_time are different, but files have the same size (> 0, with real content)
                             * => check file content to see whether they are really different
                             */
                            if (ctx.settings.debug)
                            {
                                std::cerr << "File with same size but different m_time, checking content: " << relPath << std::endl;
                            }
                            ctx.dispatcher->contentCompareWithPartialReport(std::move(reportEntry), lstat1.st_size);
                            // report has been done, clear so that it is not done twice
                            reportEntry.clear();
                        }
                        break;

                    case FileType::Symlink:
                    {
                        // symlinks: compare the target names
                        const std::string target1 = ctx.root1.readSymlink(relPath, lstat1.st_size);
                        const std::string target2 = ctx.root2.readSymlink(relPath, lstat2.st_size);
                        if (target1 != target2)
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
            itDir1++;
            itDir2++;
        }
    }

    // process remaining items on one side or the other
    while (itDir1 != dirContent1.cend())
    {
        handle_single_side_entry(make_path(dirPath, itDir1->filename), itDir1->fileType, FileType::NoFile);
        itDir1++;
    }
    while (itDir2 != dirContent2.cend())
    {
        handle_single_side_entry(make_path(dirPath, itDir2->filename), FileType::NoFile, itDir2->fileType);
        itDir2++;
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