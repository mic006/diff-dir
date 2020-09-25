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
 * main() function and argument parsing.
 */

#include <iostream>

#include "cxxopts.hpp"

#include "context.h"
#include "diff_dir.h"
#include "dispatcher_mono.h"
#include "dispatcher_multi.h"
#include "file_comp.h"
#include "path.h"
#include "report_console.h"

#ifndef VERSION
#define VERSION "testbuild"
#endif // VERSION

/// Error message prefix
constexpr const char *error_prefix = "diff-dir error: ";

/** Parse arguments and catch exception nicely.
 * 
 * Workaround to https://github.com/jarro2783/cxxopts/issues/146 :
 * use a dedicated function to catch an exception.
 */
static cxxopts::ParseResult parseArgs(cxxopts::Options &options, int &argc, char **&argv)
try
{
    cxxopts::ParseResult result = options.parse(argc, argv);
    return result;
}
catch (const cxxopts::OptionParseException &exc)
{
    std::cerr << error_prefix << exc.what() << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    // parse options
    cxxopts::Options options("diff-dir", "Difference of 2 directories");

    options.add_options()                                                                                                         //
        ("h,help", "help message", cxxopts::value<bool>())                                                                        //
        ("v,version", "print version", cxxopts::value<bool>())                                                                    //
        ("s,status", "give no output, return 1 on first identified difference, 0 if no difference found")                         //
        ("i,ignore", "ignore paths matching the given pattern(s)", cxxopts::value<std::vector<std::string>>(), "path_pattern")    //
        ("m,metadata", "check and report metadata differences (ownership, permissions)", cxxopts::value<bool>())                  //
        ("t,thread", "use multiple threads to speed-up the comparison", cxxopts::value<bool>())                                   //
        ("B,buffer", "size of the buffers used for content comparison", cxxopts::value<size_t>()->default_value("65536"), "size") //
        ("d,debug", "print debug information during the diff", cxxopts::value<bool>())                                            //
        ("dir1", "first directory", cxxopts::value<std::string>())                                                                //
        ("dir2", "second directory", cxxopts::value<std::string>())                                                               //
        ;

    options.parse_positional({"dir1", "dir2"});
    options.positional_help("dir1 dir2");

    const auto result = parseArgs(options, argc, argv);

    // check parameters
    if (result["help"].as<bool>())
    {
        std::cerr << options.help();
        return EXIT_SUCCESS;
    }

    if (result["version"].as<bool>())
    {
        std::cerr << "diff-dir " << VERSION << "\n";
        return EXIT_SUCCESS;
    }

    if (result["dir1"].count() == 0 or result["dir2"].count() == 0)
    {
        std::cerr << error_prefix << "missing mandatory arguments" << std::endl;
        exit(EXIT_FAILURE);
    }

    RootPath root1{result["dir1"].as<std::string>()};
    RootPath root2{result["dir2"].as<std::string>()};
    if (!root1.isValid() or !root2.isValid())
    {
        std::cerr << error_prefix << "invalid paths, need 2 directories" << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t buffSize = result["buffer"].as<size_t>();
    if (buffSize == 0)
    {
        std::cerr << error_prefix << "invalid buffer size" << std::endl;
        exit(EXIT_FAILURE);
    }

    // prepare diff context
    Context ctx{{result["debug"].as<bool>(),
                 result["status"].as<bool>(),
                 result["metadata"].as<bool>(),
                 buffSize}};
    ctx.root1 = std::move(root1);
    ctx.root2 = std::move(root2);
    ReportConsole reportConsole{ctx.settings};
    if (result["thread"].as<bool>())
        ctx.dispatcher = std::make_unique<DispatcherMultiThread>(ctx, reportConsole);
    else
        ctx.dispatcher = std::make_unique<DispatcherMonoThread>(ctx, reportConsole);
    // handle ignore rules
    if (result["ignore"].count() > 0)
    {
        ctx.ignoreFilter = std::move(result["ignore"].as<std::vector<std::string>>());
    }

    // perform the diff
    diff_dirs(ctx);

    return EXIT_SUCCESS;
}