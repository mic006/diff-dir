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

#include <locale>

#include "context.h"
#include "term_app_settings.h"

TermAppSettings::TermAppSettings(const Context &diffDirCtx)
    : spinnerStrings{}
{
    // need locale for UTF-8 parsing
    std::locale::global(std::locale(""));

    // read configuration
    const YAML::Node &appCfg = diffDirCtx.cfg["interactive"];
    normal.colorFg = termui::Color::rgbMask | appCfg["ui"]["normal"]["fg"].as<uint32_t>();
    normal.colorBg = termui::Color::rgbMask | appCfg["ui"]["normal"]["bg"].as<uint32_t>();
    header.colorFg = termui::Color::rgbMask | appCfg["ui"]["header"]["fg"].as<uint32_t>();
    header.colorBg = termui::Color::rgbMask | appCfg["ui"]["header"]["bg"].as<uint32_t>();
    header.effect = appCfg["ui"]["header"]["effect"].as<uint32_t>();
    footer.colorFg = termui::Color::rgbMask | appCfg["ui"]["footer"]["fg"].as<uint32_t>();
    footer.colorBg = termui::Color::rgbMask | appCfg["ui"]["footer"]["bg"].as<uint32_t>();
    footer.effect = appCfg["ui"]["footer"]["effect"].as<uint32_t>();
    selectedFg = termui::Color::rgbMask | appCfg["ui"]["selectedFg"].as<uint32_t>();
    selectedBg = termui::Color::rgbMask | appCfg["ui"]["selectedBg"].as<uint32_t>();
    separator = termui::Color::rgbMask | appCfg["ui"]["separator"].as<uint32_t>();
    fileType = termui::Color::rgbMask | appCfg["ui"]["fileType"].as<uint32_t>();
    differenceLFg = termui::Color::rgbMask | appCfg["ui"]["differenceL"]["fg"].as<uint32_t>();
    differenceLBg = termui::Color::rgbMask | appCfg["ui"]["differenceL"]["bg"].as<uint32_t>();
    differenceRFg = termui::Color::rgbMask | appCfg["ui"]["differenceR"]["fg"].as<uint32_t>();
    differenceRBg = termui::Color::rgbMask | appCfg["ui"]["differenceR"]["bg"].as<uint32_t>();
    warningFg = termui::Color::rgbMask | appCfg["ui"]["warning"]["fg"].as<uint32_t>();
    warningBg = termui::Color::rgbMask | appCfg["ui"]["warning"]["bg"].as<uint32_t>();
    metadataBg = termui::Color::rgbMask | appCfg["ui"]["metadataBg"].as<uint32_t>();
    minWidthForLeftRightView = appCfg["minWidthForLeftRightView"].as<uint32_t>();
    cycleTimeMs = appCfg["cycleTimeMs"].as<uint32_t>();
    for (const auto &entry : appCfg["spinner"]["strings"])
        spinnerStrings.emplace_back(entry.as<std::string>());
    spinnerStepCount = appCfg["spinner"]["stepTimeMs"].as<uint32_t>() / cycleTimeMs;
    diffCommonThreshold = appCfg["text"]["diffCommonThreshold"].as<uint32_t>();
    tabSize = appCfg["text"]["tabSize"].as<uint32_t>();
    replaceCR = termui::toU32String(appCfg["text"]["replacement"]["carriageReturn"].as<std::string>());
    replaceEscape = termui::toU32String(appCfg["text"]["replacement"]["escape"].as<std::string>());
    replaceTab = termui::toU32String(appCfg["text"]["replacement"]["tab"].as<std::string>());
}