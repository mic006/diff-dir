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

#include "context.h"
#include "yaml_util.h"

static constexpr const char *system_config_file_path = "/etc/diff-dir.conf.yaml";
static constexpr const char *user_config_file_path = "~/.diff-dir.conf.yaml";

// symbols of the resource file
extern const char _binary_diff_dir_conf_yaml_start[];
extern const char _binary_diff_dir_conf_yaml_end[];

YAML::Node getConfig()
{
    // start with the embedded default configuration
    YAML::Node config = YAML::Load(
        std::string(
            _binary_diff_dir_conf_yaml_start,
            _binary_diff_dir_conf_yaml_end - _binary_diff_dir_conf_yaml_start));

    // load system configuration if available
    try
    {
        const auto system_config = YAML::LoadFile(system_config_file_path);
        config = yaml_merge(config, system_config);
    }
    catch (...)
    {
    }

    // load user configuration if available
    try
    {
        const auto user_config = YAML::LoadFile(user_config_file_path);
        config = yaml_merge(config, user_config);
    }
    catch (...)
    {
    }

    return config;
}
