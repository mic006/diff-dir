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

#include <fstream>

#include "dtl.hpp"
#include "text_diff.h"

bool TextDifference::convertContent(const std::u32string &src, sequence &seq)
{
    size_t lastCopiedPos = 0;
    size_t tabPos = 0;
    std::u32string currentLine{};
    std::u32string replacement{};

    // process the string
    for (size_t pos = 0; pos < src.size(); pos++)
    {
        const char32_t c = src[pos];
        if (c == '\n')
        {
            if (lastCopiedPos < pos)
                currentLine.append(src, lastCopiedPos, pos - lastCopiedPos);
            seq.push_back(std::move(currentLine));
            currentLine.clear();
            lastCopiedPos = pos + 1;
            tabPos = pos + 1;
        }
        else if (c < 0x20)
        {
            if (c == '\r')
                replacement = ui.replaceCR;
            else if (c == '\e')
                replacement = ui.replaceEscape;
            else if (c == '\t')
            {
                // align to next tabulation
                int nbSpaces = ui.tabSize - (pos - tabPos) % ui.tabSize;
                replacement = ui.replaceTab + std::u32string(nbSpaces - ui.replaceTab.size(), ' ');
                // next character is aligned with tabulation
                tabPos = pos + 1;
            }
            else
            {
                // invalid special characters => consider content as binary, not text
                return false;
            }

            // copy content
            if (lastCopiedPos < pos)
                currentLine.append(src, lastCopiedPos, pos - lastCopiedPos);
            currentLine += replacement;
            lastCopiedPos = pos + 1;
        }
    }

    // copy final content
    if (lastCopiedPos < src.size())
        currentLine.append(src, lastCopiedPos);
    if (not currentLine.empty())
        seq.push_back(std::move(currentLine));

    return true;
}

void TextDifference::pushMessageBinaryContent(std::vector<std::u32string> &diffDetails)
{
    diffDetails.emplace_back(U"<Binary content, cannot compare>");
}

void TextDifference::operator()(const std::string &contentL, const std::string &contentR, std::vector<std::u32string> &diffDetails)
{
    // get content as UTF-32
    std::u32string c32contentL, c32contentR;
    try
    {
        c32contentL = termui::toU32String(contentL);
        c32contentR = termui::toU32String(contentR);
    }
    catch (const termui::TermUiException &)
    {
        // invalid unicode content
        return pushMessageBinaryContent(diffDetails);
    }

    // convert content for comparison
    sequence seqL{}, seqR{};
    if (not convertContent(c32contentL, seqL))
        return pushMessageBinaryContent(diffDetails); // special characters in the file, binary content
    if (not convertContent(c32contentR, seqR))
        return pushMessageBinaryContent(diffDetails); // special characters in the file, binary content

    bool diffPublished = false;
    if (!seqL.empty() and !seqR.empty())
    {
        // compare the content
        dtl::Diff<elem, sequence> diff{seqL, seqR, true};
        diff.enableTrivial();
        diff.enableHuge();
        diff.compose();

        const auto &diffSequence = diff.getSes().getSequence();
        // acceptable diff size = 50% common on the minimum size
        const size_t acceptableDiffSize =
            std::max(seqL.size(), seqR.size()) +
            std::min(seqL.size(), seqR.size()) * ui.diffCommonThreshold / 100;
        if (diffSequence.size() <= acceptableDiffSize)
        {
            // display the differences as computed
            for (const auto &[elem, info] : diffSequence)
            {
                switch (info.type)
                {
                case dtl::SES_COMMON:
                    diffDetails.emplace_back(elem);
                    break;
                case dtl::SES_DELETE:
                    diffDetails.emplace_back(formatDiffL + elem);
                    break;
                case dtl::SES_ADD:
                    diffDetails.emplace_back(formatDiffR + elem);
                    break;
                }
            }
            diffPublished = true;
        }
    }

    if (not diffPublished)
    {
        /* content is too different or only one content was provided:
         * - first display the left (as deleted)
         * - then display the right (as added)
         */
        for (const auto &line : seqL)
            diffDetails.emplace_back(formatDiffL + line);
        for (const auto &line : seqR)
            diffDetails.emplace_back(formatDiffR + line);
    }
}