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

#include "ignore.h"

/** Replace occurrences in a string
 * https://stackoverflow.com/a/29752943
 */
static void replaceAll(std::string &source, const std::string &from, const std::string &to)
{
    std::string newString{};
    newString.reserve(source.length()); // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while (std::string::npos != (findPos = source.find(from, lastPos)))
    {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    // care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}

IgnoreFilter::IgnoreFilter(const std::vector<std::string> &ignoreRules)
{
    // build a single regex matching all the ignore rules
    std::string unifiedRegex{};

    for (const auto &rule : ignoreRules)
    {
        // transform the rule to a regex
        std::string ruleRegex = rule;
        // . -> \\. (matches a real dot)
        replaceAll(ruleRegex, ".", "\\.");
        // ? -> [^/] (matches any single character, except /)
        // TODO: do not replace escaped-? (\\?)
        replaceAll(ruleRegex, "?", "[^/]");
        // * -> [^/]* (matches any number of any characters, except /)
        // TODO: do not replace escaped-* (\\*)
        replaceAll(ruleRegex, "*", "[^/]*");
        // all other regex elements are kept as is. User shall escape them if needed.

        /* handle absolute / relative rules
         * if a rule starts with a /, the rule is absolute and shall match from the beginning
         * otherwise, the rule is relative and shall match between /
         */
        if (ruleRegex.size() > 0 and ruleRegex[0] == '/')
        {
            // absolute rule: remove the leading slash
            ruleRegex = ruleRegex.substr(1);
        }
        else
        {
            // relative rule: allow any path before the rule
            ruleRegex = "(.*/)?" + ruleRegex;
        }

        // add it to the unified regex
        if (unifiedRegex.size() > 0)
            unifiedRegex += "|";
        unifiedRegex += "(" + ruleRegex + ")";
    }

    m_regex = std::regex(unifiedRegex, std::regex::nosubs | std::regex::optimize);
}