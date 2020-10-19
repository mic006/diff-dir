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
 * YAML utilities.
 */

#pragma once

#include <yaml-cpp/yaml.h>

/** Merge 2 nodes recursively.
 * The result will have the same schema as base, with content:
 * - from upper when a value exists
 * - from base otherwise.
 * 
 * @param[in] base   yaml tree, provides schema and default values for the merge
 * @param[in] upper  yaml tree, provides values to override base
 * @return yaml tree, same schema as base, values from upper or base by default
 */
YAML::Node yaml_merge(const YAML::Node base, const YAML::Node upper);
