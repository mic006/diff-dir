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
 * Context definition.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <yaml-cpp/yaml.h>

#include "dispatcher.h"
#include "ignore.h"
#include "path.h"

/// Constant settings of the diff
struct Settings
{
    bool debug;               ///< output debug information on stderr
    bool checkMetadata;       ///< whether metadata shall be checked for differences
    size_t contentBufferSize; ///< size to be used for buffering file content
};

// forward reference
class Dispatcher;

/// type for function check if 2 files have the same content
typedef std::function<bool(const std::string &relPath, size_t fileSize)> comp_file_fct_type;

/// Side
enum class Side : int
{
    Left = 0,
    Right,
};

/// Context with handlers
class Context
{
public:
    Context(const Settings &_settings, const YAML::Node &config)
        : settings{_settings},
          cfg{config},
          dispatcher{},
          ignoreFilter{}
    {
    }

    const Settings settings;                  ///< settings of the diff
    const YAML::Node &cfg;                    ///< user configuration
    RootPath root[2];                         ///< root on left and right sides
    std::unique_ptr<Dispatcher> dispatcher;   ///< dispatcher for report and file comparison
    std::optional<IgnoreFilter> ignoreFilter; ///< filter to ignore some paths during the diff
};

/** Get the yaml configuration.
 * The returned configuration is built from:
 * - the default settings embedded in the source code
 * - overriden by the system settings
 * - overriden by the user settings
 */
YAML::Node getConfig();