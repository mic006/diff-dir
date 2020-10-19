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
 * Text difference algorithm.
 */

#pragma once

#include "term_app_settings.h"

/// Compute differences on text
class TextDifference
{
public:
    TextDifference(const TermAppSettings &_ui)
        : ui{_ui},
          formatDiffL{termui::U32Format::buildColorFg(ui.differenceLFg), termui::U32Format::buildColorBg(ui.differenceLBg)},
          formatDiffR{termui::U32Format::buildColorFg(ui.differenceRFg), termui::U32Format::buildColorBg(ui.differenceRBg)}
    {
    }

    /** Compute the differences between the 2 contents.
     * @param[in]      contentL  content for left side
     * @param[in]      contentR  content for right side
     * @param[in,out]  lines to be displayed, with formatting
     */
    void operator()(const std::string &contentL, const std::string &contentR, std::vector<std::u32string> &diffDetails);

private:
    // template types for dtl
    using elem = std::u32string;
    using sequence = std::vector<elem>;

    /** Convert content for comparison.
     * - check for special characters, replace them or stop if binary content
     * - split by lines
     * @param[in]  src  content to prepare
     * @param[out] seq  content split by lines, and with special characters replaced
     * @return success
     */
    bool convertContent(const std::u32string &src, sequence &seq);

    /// Push message that comparison cannot be done as content is binary
    void pushMessageBinaryContent(std::vector<std::u32string> &diffDetails);

    const TermAppSettings &ui;
    const std::u32string formatDiffL; ///< format string for left side only display
    const std::u32string formatDiffR; ///< format string for right side only display
};