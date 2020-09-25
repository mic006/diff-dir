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
 * Filter capability to ignore some paths.
 */

#pragma once

#include <regex>

/// ignore filter object
class IgnoreFilter
{
public:
    IgnoreFilter(const std::vector<std::string> &ignoreRules);

    /// Indicate if the provided path shall be ignored
    bool isIgnored(const std::string &path) const
    {
        return std::regex_match(path, m_regex);
    }

private:
    std::regex m_regex; ///< regex built from the rules
};