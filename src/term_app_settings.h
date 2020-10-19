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
 * Terminal application settings.
 */

#pragma once

#include "termui.h"

// forward reference
class Context;

// TermApp settings
struct TermAppSettings
{
    TermAppSettings(const Context &diffDirCtx);

    termui::RenderCtx normal;                ///< normal text
    termui::RenderCtx header;                ///< header
    termui::RenderCtx footer;                ///< footer
    termui::Color selectedFg;                ///< selected item foreground color
    termui::Color selectedBg;                ///< selected item background color
    termui::Color separator;                 ///< separator foreground color
    termui::Color fileType;                  ///< filetype foreground color
    termui::Color differenceLFg;             ///< difference foreground color, left side
    termui::Color differenceLBg;             ///< difference background color, left side
    termui::Color differenceRFg;             ///< difference foreground color, right side
    termui::Color differenceRBg;             ///< difference background color, right side
    termui::Color warningFg;                 ///< alert user when modification time is more recent on left
    termui::Color warningBg;                 ///< alert user when modification time is more recent on left
    termui::Color metadataBg;                ///< metadata details background color
    int minWidthForLeftRightView;            ///< minimal window width to use left / right view
    int cycleTimeMs;                         ///< cycle time for the terminal application, in ms
    std::vector<std::string> spinnerStrings; ///< strings for the spinner, displayed cyclically
    int spinnerStepCount;                    ///< number of cycleTimeMs each spinner string is displayed
    int diffCommonThreshold;                 ///< percentage of difference between files for different display
    int tabSize;                             ///< size to expand tabs
    std::u32string replaceCR;                ///< string replace for carriage return
    std::u32string replaceEscape;            ///< string replace for escape
    std::u32string replaceTab;               ///< string replace for tabulation
};