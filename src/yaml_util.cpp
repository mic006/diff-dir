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

#include "yaml_util.h"

YAML::Node yaml_merge(const YAML::Node base, const YAML::Node upper)
{
    if (not upper.IsDefined() or upper.IsNull())
        return base;

    if (base.IsMap())
    {
        YAML::Node result{YAML::NodeType::Map};
        // keep all keys from base, merge values
        for (const auto &entry : base)
        {
            const auto &key = entry.first.as<std::string>();
            result[key] = yaml_merge(entry.second, upper[key]);
        }
        // get all extra keys from upper
        for (const auto &entry : upper)
        {
            const auto &key = entry.first.as<std::string>();
            if (not result[key])
                result[key] = entry.second;
        }
        return result;
    }

    return upper;
}
