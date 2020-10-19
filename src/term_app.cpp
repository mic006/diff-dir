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
 * Terminal application.
 */

#include "term_app.h"

void TermAppWindow::setWinPos(int _origY, int _origX, int _height, int _width)
{
    origX = _origX;
    origY = _origY;
    height = _height;
    width = _width;
}

void TermAppWindow::draw()
{
    // content
    const int innerHeight = height - 2; // exclude header and footer
    const int contentSize = getContentSize();
    determineDisplayContent(innerHeight, contentSize);
    const int remaining = contentSize - firstDisplayedIndex;
    const int displayed = std::min(remaining, innerHeight);
    for (int i = 0; i < displayed; i++)
    {
        drawContentLine(origY + 1 + i, firstDisplayedIndex + i);
    }

    // header
    ctx.tmui.addStringN(origY, origX, header, width, termui::TextAlignment::kCentered, ctx.ui.header);

    // footer
    ctx.tmui.addStringsN(origY + height - 1, origX, footer.textLeft, footer.textMiddle, footer.textRight, width, ctx.ui.footer);
}

void TermAppWindow::determineDisplayContent(int innerHeight, int contentSize)
{
    // adjust firstDisplayedIndex based on screen size
    firstDisplayedIndex = std::min(firstDisplayedIndex, contentSize - innerHeight);
    firstDisplayedIndex = std::max(firstDisplayedIndex, 0);

    if (contentSize > 0)
    {
        // indicate which part of the content is on display
        const int beginPerc = 100 * firstDisplayedIndex / contentSize;
        const int displayEnd = std::min(firstDisplayedIndex + innerHeight, contentSize);
        const int endPerc = 100 * displayEnd / contentSize;
        footer.textRight = std::to_string(beginPerc) + "%-" + std::to_string(endPerc) + "%";

        // update scrollbar indicator positions
        // have a constant scroll bar size (for a given screen width)
        int scrollBarSize = contentSize == 0 ? innerHeight : (innerHeight * innerHeight + contentSize / 2) / contentSize;
        scrollBarSize = std::max(scrollBarSize, 1); // at least 1 char

        // determine the start position of the scrollbar
        scrollIndStart = (innerHeight * firstDisplayedIndex + contentSize / 2) / contentSize; // round
        // scroll bar shall be at top or bottom only if we really are at top or bottom
        if (firstDisplayedIndex > 0)
            scrollIndStart = std::max(scrollIndStart, 1);
        if (displayEnd < contentSize)
            scrollIndStart = std::min(scrollIndStart, innerHeight - 1 - scrollBarSize);
        scrollIndEnd = scrollIndStart + scrollBarSize;
    }
    else
    {
        footer.textRight = "";
        scrollIndStart = 0;
        scrollIndEnd = innerHeight;
    }
}

void TermAppListWindow::drawContentLine(int y, int contentIndex)
{
    const ReportEntry &reportEntry = ctx.diffs[contentIndex].reportEntry;

    const termui::Color bg = ctx.ui.normal.colorBg;

    if (reportEntry.isDifferent(EntryDifference::EntryType))
    {
        ctx.tmui.addGlyph(y, origX, FileType::repr(reportEntry.file[0].type), ctx.ui.differenceLFg, bg);
        ctx.tmui.addGlyph(y, origX + 2, U'!');
        ctx.tmui.addGlyph(y, origX + 4, FileType::repr(reportEntry.file[1].type), ctx.ui.differenceLFg, bg);
    }
    else
    {
        const FileType::EnumType fileType = reportEntry.file[0].type;
        ctx.tmui.addGlyph(y, origX, FileType::repr(fileType), ctx.ui.fileType, bg);

        const char32_t indicatorNoDiff = U'-';
        const int contentIndX = 2;
        if (reportEntry.isDifferent(EntryDifference::Content))
            ctx.tmui.addGlyph(y, contentIndX, U'c', ctx.ui.differenceLFg, bg);
        else if (reportEntry.isDifferent(EntryDifference::Size))
            ctx.tmui.addGlyph(y, contentIndX, U's', ctx.ui.differenceLFg, bg);
        else if (fileType != FileType::Directory)
            ctx.tmui.addGlyph(y, contentIndX, indicatorNoDiff);

        if (ctx.diffDirCtx.settings.checkMetadata)
        {
            const int ownershipIndX = 3;
            if (reportEntry.isDifferent(EntryDifference::Ownership))
                ctx.tmui.addGlyph(y, ownershipIndX, U'o', ctx.ui.differenceLFg, bg);
            else
                ctx.tmui.addGlyph(y, ownershipIndX, indicatorNoDiff);

            const int permissionsIndX = 4;
            if (reportEntry.isDifferent(EntryDifference::Permissions))
                ctx.tmui.addGlyph(y, permissionsIndX, U'p', ctx.ui.differenceLFg, bg);
            else if (fileType != FileType::Symlink)
                ctx.tmui.addGlyph(y, permissionsIndX, indicatorNoDiff);
        }
    }
    ctx.tmui.addStringN(y, origX + 7, reportEntry.relPath, width - 7, termui::TextAlignment::kLeft | termui::TextAlignment::kClipStart);

    // highlight selection
    if (contentIndex == ctx.selectedIndex)
        ctx.tmui.setColors(y, origX, width, ctx.ui.selectedFg, ctx.ui.selectedBg);
}

void TermAppListWindow::determineDisplayContent(int innerHeight, int contentSize)
{
    // adjust firstDisplayedIndex based on screen size and selectedIndex
    // keep selected index in the screen, and not first / last line on the screen if possible
    firstDisplayedIndex = std::max(firstDisplayedIndex, ctx.selectedIndex + 2 - innerHeight);
    firstDisplayedIndex = std::min(firstDisplayedIndex, ctx.selectedIndex - 1);

    TermAppWindow::determineDisplayContent(innerHeight, contentSize);

    // indicate the index of selection to user
    footer.textMiddle = std::to_string(ctx.selectedIndex + 1) + "/" + std::to_string(ctx.diffs.size());
}

void TermAppListWindow::moveSelection(MoveKind mv)
{
    const int pageSize = height - 2 - 1; // header, footer, and one line kept from previous screen
    switch (mv)
    {
    case MoveKind::Top:
        ctx.selectedIndex = 0;
        break;
    case MoveKind::PageUp:
        firstDisplayedIndex -= pageSize;
        ctx.selectedIndex -= pageSize;
        break;
    case MoveKind::LineUp:
        ctx.selectedIndex--;
        break;
    case MoveKind::LineDown:
        ctx.selectedIndex++;
        break;
    case MoveKind::PageDown:
        firstDisplayedIndex += pageSize;
        ctx.selectedIndex += pageSize;
        break;
    case MoveKind::Bottom:
        ctx.selectedIndex = ctx.diffs.size() - 1;
        break;
    }
    ctx.selectedIndex = std::max(ctx.selectedIndex, 0);
    ctx.selectedIndex = std::min(ctx.selectedIndex, (int)ctx.diffs.size() - 1);
    // clipping of firstDisplayedIndex is done in determineDisplayContent() to handle screen resize
}

TermAppDetailWindow::TermAppDetailWindow(TermAppContext &_ctx)
    : TermAppWindow{_ctx},
      metadataBg{termui::U32Format::buildColorBg(ctx.ui.metadataBg)},
      titleStart{termui::U32Format::buildEffect(termui::Effect::kUnderline)},
      titleEnd{termui::U32Format::buildEffect(0)},
      differenceL{termui::U32Format::buildColorFg(ctx.ui.differenceLFg)},
      differenceR{termui::U32Format::buildColorFg(ctx.ui.differenceRFg)},
      normal{termui::U32Format::buildColorFg(ctx.ui.normal.colorFg)},
      warningStr{termui::U32Format::buildColorFg(ctx.ui.warningFg), termui::U32Format::buildColorBg(ctx.ui.warningBg)},
      metadataStr{metadataBg, normal},
      fieldsTitle{},
      fieldsLeft{},
      fieldsRight{}
{
}

static constexpr const char *fileTypeStr[FileType::NbElem] = {
    "None",              // NoFile
    "Regular file",      // Regular
    "Directory",         // Directory
    "Block",             // Block
    "Character",         // Character
    "Fifo / Named pipe", // Fifo
    "Symbolic link",     // Symlink
    "Socket",            // Socket
    "Unknown",           // Unknown
};

int TermAppDetailWindow::maxDisplayLength(const std::vector<FormattedString> &v)
{
    int result = 0;
    for (const auto &fs : v)
        result = std::max(result, fs.displayLength);
    return result;
}

void TermAppDetailWindow::addMetadataSingleFile(const FileEntry &file, Side side)
{
    // swap fields if file exists on right
    bool swap = side == Side::Right;
    const std::u32string notAvailable = U"-";

    addMetadataSimpleLineDiffers(U"Type",
                                 termui::toU32String(fileTypeStr[file.type]),
                                 termui::toU32String(fileTypeStr[FileType::NoFile]),
                                 swap);

    if (file.type == FileType::Regular)
    {
        addMetadataSimpleLineDiffers(U"Size",
                                     termui::toU32String(file.size()),
                                     notAvailable,
                                     swap);
    }

    if (file.type == FileType::Regular or file.type == FileType::Symlink)
    {
        addMetadataSimpleLineDiffers(U"Mtime",
                                     termui::toU32String(file.mtime()),
                                     notAvailable,
                                     swap);
    }

    const std::string ownership =
        ctx.uidgidReader.getUidName(file.lstat.st_uid) +
        ':' + ctx.uidgidReader.getGidName(file.lstat.st_gid);
    addMetadataSimpleLineDiffers(U"Ownership",
                                 termui::toU32String(ownership),
                                 notAvailable,
                                 swap);

    if (file.type != FileType::Symlink)
    {
        addMetadataSimpleLineDiffers(U"Permissions",
                                     termui::toU32String(file.permissions()),
                                     notAvailable,
                                     swap);
    }
}

void TermAppDetailWindow::addMetadataSimpleLineCommon(const std::u32string &title, const std::u32string &common)
{
    fieldsTitle.emplace_back(title, title.size());
    fieldsLeft.emplace_back(common, common.size());
    fieldsRight.emplace_back(); // empty field
}

void TermAppDetailWindow::addMetadataSimpleLineDiffers(const std::u32string &title, const std::u32string &left, const std::u32string &right)
{
    fieldsTitle.emplace_back(title, title.size());
    fieldsLeft.emplace_back(differenceL + left + normal, left.size());
    fieldsRight.emplace_back(differenceR + right + normal, right.size());
}

void TermAppDetailWindow::addMetadataSimpleLineDiffers(const std::u32string &title, const std::u32string &left, const std::u32string &right, bool swap)
{
    if (swap)
        addMetadataSimpleLineDiffers(title, right, left);
    else
        addMetadataSimpleLineDiffers(title, left, right);
}

void TermAppDetailWindow::addMetadataSimpleLineWarning(const std::u32string &title, const std::u32string &left, const std::u32string &right)
{
    fieldsTitle.emplace_back(title, title.size());
    fieldsLeft.emplace_back(warningStr + left + metadataStr, left.size());
    fieldsRight.emplace_back(warningStr + right + metadataStr, right.size());
}

void TermAppDetailWindow::updateSelection()
{
    DiffEntry *entry = ctx.getSelected();
    header = entry ? entry->reportEntry.relPath : "";
    firstDisplayedIndex = 0;

    if (entry != nullptr and entry->details.empty())
    {
        const ReportEntry &reportEntry = entry->reportEntry;
        auto &details = entry->details;

        fieldsTitle.clear();
        fieldsLeft.clear();
        fieldsRight.clear();

        // prepare details for display
        const FileType::EnumType fileTypeL = reportEntry.file[0].type;
        const FileType::EnumType fileTypeR = reportEntry.file[1].type;

        if (fileTypeR == FileType::NoFile)
        {
            // only left file
            addMetadataSingleFile(reportEntry.file[0], Side::Left);
        }
        else if (fileTypeL == FileType::NoFile)
        {
            // only right file
            addMetadataSingleFile(reportEntry.file[1], Side::Right);
        }
        else if (fileTypeL != fileTypeR)
        {
            // the 2 files exist with different types
            addMetadataSimpleLineWarning(U"Type",
                                         termui::toU32String(fileTypeStr[fileTypeL]),
                                         termui::toU32String(fileTypeStr[fileTypeR]));
        }
        else
        {
            // two files of same type
            addMetadataSimpleLineCommon(U"Type",
                                        termui::toU32String(fileTypeStr[fileTypeL]));

            if (fileTypeL == FileType::Regular)
            {
                // file size
                if (reportEntry.file[0].lstat.st_size == reportEntry.file[1].lstat.st_size)
                    addMetadataSimpleLineCommon(U"Size",
                                                termui::toU32String(reportEntry.file[0].size()));
                else
                    addMetadataSimpleLineDiffers(U"Size",
                                                 termui::toU32String(reportEntry.file[0].size()),
                                                 termui::toU32String(reportEntry.file[1].size()));
            }

            if (fileTypeL == FileType::Regular or fileTypeL == FileType::Symlink)
            {
                // file modification time
                if (reportEntry.file[0].lstat.st_mtim.tv_sec == reportEntry.file[1].lstat.st_mtim.tv_sec)
                    addMetadataSimpleLineCommon(U"Mtime",
                                                termui::toU32String(reportEntry.file[0].mtime()));
                else if (reportEntry.file[0].lstat.st_mtim.tv_sec < reportEntry.file[1].lstat.st_mtim.tv_sec)
                    addMetadataSimpleLineDiffers(U"Mtime",
                                                 termui::toU32String(reportEntry.file[0].mtime()),
                                                 termui::toU32String(reportEntry.file[1].mtime()));
                else
                    addMetadataSimpleLineWarning(U"Mtime",
                                                 termui::toU32String(reportEntry.file[0].mtime()),
                                                 termui::toU32String(reportEntry.file[1].mtime()));
            }

            // file ownership
            if (reportEntry.file[0].lstat.st_uid == reportEntry.file[1].lstat.st_uid and
                reportEntry.file[0].lstat.st_gid == reportEntry.file[1].lstat.st_gid)
            {
                const std::string ownership =
                    ctx.uidgidReader.getUidName(reportEntry.file[0].lstat.st_uid) +
                    ':' + ctx.uidgidReader.getGidName(reportEntry.file[0].lstat.st_gid);
                addMetadataSimpleLineCommon(U"Ownership",
                                            termui::toU32String(ownership));
            }
            else
            {
                std::u32string title = U"Ownership";
                fieldsTitle.emplace_back(std::move(title), title.size());
                auto &left = fieldsLeft.emplace_back();
                auto &right = fieldsRight.emplace_back();

                const std::u32string ownerL = termui::toU32String(ctx.uidgidReader.getUidName(reportEntry.file[0].lstat.st_uid));
                if (reportEntry.file[0].lstat.st_uid == reportEntry.file[1].lstat.st_uid)
                {
                    left.str = ownerL;
                    left.displayLength = ownerL.size();
                    right.str = ownerL;
                    right.displayLength = ownerL.size();
                }
                else
                {
                    left.str = differenceL + ownerL + normal;
                    left.displayLength = ownerL.size();
                    const std::u32string ownerR = termui::toU32String(ctx.uidgidReader.getUidName(reportEntry.file[1].lstat.st_uid));
                    right.str = differenceR + ownerR + normal;
                    right.displayLength = ownerR.size();
                }

                left.str += U':';
                left.displayLength++;
                right.str += U':';
                right.displayLength++;

                const std::u32string groupL = termui::toU32String(ctx.uidgidReader.getGidName(reportEntry.file[0].lstat.st_gid));
                if (reportEntry.file[0].lstat.st_gid == reportEntry.file[1].lstat.st_gid)
                {
                    left.str += groupL;
                    left.displayLength += groupL.size();
                    right.str += groupL;
                    right.displayLength += groupL.size();
                }
                else
                {
                    left.str += differenceL + groupL + normal;
                    left.displayLength += groupL.size();
                    const std::u32string groupR = termui::toU32String(ctx.uidgidReader.getGidName(reportEntry.file[1].lstat.st_gid));
                    right.str += differenceR + groupR + normal;
                    right.displayLength += groupR.size();
                }
            }

            // file permissions
            if (fileTypeL != FileType::Symlink)
            {
                if (reportEntry.file[0].lstat.st_mode == reportEntry.file[1].lstat.st_mode)
                {
                    addMetadataSimpleLineCommon(U"Permissions",
                                                termui::toU32String(reportEntry.file[0].permissions()));
                }
                else
                {
                    std::u32string title = U"Permissions";
                    fieldsTitle.emplace_back(std::move(title), title.size());
                    const std::string filePermL = reportEntry.file[0].permissions();
                    const std::string filePermR = reportEntry.file[1].permissions();
                    auto &left = fieldsLeft.emplace_back(U"", filePermL.size());
                    auto &right = fieldsRight.emplace_back(U"", filePermL.size());
                    bool modeDifference = false;
                    for (int i = 0; i < (int)filePermL.size(); i++)
                    {
                        const bool wantedMode = filePermL[i] != filePermR[i];
                        if (modeDifference and not wantedMode)
                        {
                            left.str += normal;
                            right.str += normal;
                            modeDifference = false;
                        }
                        else if (not modeDifference and wantedMode)
                        {
                            left.str += differenceL;
                            right.str += differenceR;
                            modeDifference = true;
                        }
                        left.str += filePermL[i];
                        right.str += filePermR[i];
                    }
                    if (modeDifference)
                    {
                        left.str += normal;
                        right.str += normal;
                    }
                }
            }
        }

        // build details
        const int maxTitle = maxDisplayLength(fieldsTitle);
        const int maxLeft = maxDisplayLength(fieldsLeft);
        for (int i = 0; i < (int)fieldsTitle.size(); i++)
        {
            std::u32string &line = details.emplace_back();
            line += metadataBg;
            line += titleStart;
            line += fieldsTitle[i].str;
            line += titleEnd;
            line += U": ";
            line.resize(line.size() + maxTitle - fieldsTitle[i].displayLength, U' ');
            line += fieldsLeft[i].str;
            if (not fieldsRight[i].str.empty())
            {
                line.resize(line.size() + maxLeft - fieldsLeft[i].displayLength, U' ');
                line += U" <-> ";
                line += fieldsRight[i].str;
            }
        }

        // compare content
        if (fileTypeL != FileType::NoFile and
            fileTypeR != FileType::NoFile and
            fileTypeL != fileTypeR)
        {
            // 2 files with different types
            details.emplace_back(U"<Different file types, cannot compare>");
        }
        else
        {
            // 1 file vs None ou 2 files of same type
            const FileType::EnumType fileType = fileTypeL != FileType::NoFile ? fileTypeL : fileTypeR;
            if (fileType == FileType::Regular)
            {
                // perform file comparison
                std::string content[2];
                for (int side = 0; side < 2; side++)
                {
                    if (reportEntry.file[side].type == FileType::Regular and reportEntry.file[side].lstat.st_size != 0)
                        content[side] = ScopedFd::openat(ctx.diffDirCtx.root[side].fd, reportEntry.relPath, O_RDONLY).getContent();
                }
                ctx.textDiff(content[0], content[1], details);
            }
            else if (fileType == FileType::Symlink)
            {
                // perform link target comparison
                ctx.textDiff(fileTypeL == FileType::Symlink ? reportEntry.file[0].symlinkTarget : "",
                             fileTypeR == FileType::Symlink ? reportEntry.file[1].symlinkTarget : "",
                             details);
            }
            else
            {
                // 2 files with different types
                details.emplace_back(U"<No content display for this file type>");
            }
        }
    }
}

void TermAppDetailWindow::drawContentLine(int y, int contentIndex)
{
    const DiffEntry *entry = ctx.getSelected();
    if (entry != nullptr and contentIndex < (int)entry->details.size())
    {
        ctx.tmui.addFString(y, origX, entry->details[contentIndex], width);
    }
}

void TermAppDetailWindow::move(MoveKind mv)
{
    const int pageSize = height - 2 - 1; // header, footer, and one line kept from previous screen
    switch (mv)
    {
    case MoveKind::Top:
        firstDisplayedIndex = 0;
        break;
    case MoveKind::PageUp:
        firstDisplayedIndex -= pageSize;
        break;
    case MoveKind::LineUp:
        firstDisplayedIndex--;
        break;
    case MoveKind::LineDown:
        firstDisplayedIndex++;
        break;
    case MoveKind::PageDown:
        firstDisplayedIndex += pageSize;
        break;
    case MoveKind::Bottom:
        firstDisplayedIndex = getContentSize();
        break;
    }
    // clipping of firstDisplayedIndex is done in determineDisplayContent() to handle screen resize
}

TermApp::TermApp(const Context &_diffDirCtx, const std::string &title)
    : diffDirCtx{_diffDirCtx}, ctx{_diffDirCtx}, winList{ctx}, winDetail{ctx}, reportQueue{},
      spinnerIndex{0}, spinnerStepCountdown{0}, appThread{}
{
    ctx.tmui.setDefaultColors(ctx.ui.normal.colorFg, ctx.ui.normal.colorBg);
    winList.header = title;

    appThread = std::jthread{&TermApp::run, this};
}

TermApp::~TermApp()
{
    // close the queue to indicate the end of the diff-dir scan
    reportQueue.close();
}

/// element for the separator with scroll bars
struct SeparatorElem
{
    char32_t glyph;
    termui::Effect effect;
};

/// Separator to used depending on left / right scrollbar presence
static const SeparatorElem separators[2][2] = {
    {
        {U'┃', 0},
        {U'▎', termui::Effect::kReverseVideo},
    },
    {
        {U'▊', 0},
        {U'█', 0},
    },
};

void TermApp::redraw()
{
    ctx.tmui.reset();
    const int width = ctx.tmui.width();
    const int height = ctx.tmui.height();

    if (width >= ctx.ui.minWidthForLeftRightView)
    {
        // large terminal, use left / right display

        // place each window
        const int windowWidth = (width - 1) / 2;
        winList.setWinPos(0, 0, height, windowWidth);
        winList.draw();
        winDetail.setWinPos(0, windowWidth + 1, height, width - (windowWidth + 1));
        winDetail.draw();

        // add the vertical separator
        ctx.tmui.addGlyph(0, windowWidth, ' ', ctx.ui.header);
        for (int y = 0; y < height - 2; y++)
        {
            const auto &sepElem = separators[winList.showScrollbar(y)][winDetail.showScrollbar(y)];
            ctx.tmui.addGlyph(1 + y, windowWidth, sepElem.glyph, ctx.ui.separator, ctx.ui.normal.colorBg, sepElem.effect);
        }
        ctx.tmui.addGlyph(height - 1, windowWidth, ' ', ctx.ui.footer);
    }
    else
    {
        // small width terminal: use top / bottom display
        const int windowHeight = height / 2;
        winList.setWinPos(0, 0, windowHeight, width);
        winList.draw();
        winDetail.setWinPos(windowHeight, 0, height - windowHeight, width);
        winDetail.draw();
    }
}

void TermApp::moveSelection(MoveKind mv)
{
    const int prevSelection = ctx.selectedIndex;
    winList.moveSelection(mv);
    if (ctx.selectedIndex != prevSelection)
        winDetail.updateSelection();
}

void TermApp::run()
{
    bool exit = false;
    bool needRedraw = true;
    bool pollQueue = true;
    while (not exit)
    {
        if (pollQueue)
        {
            // retrieve newly available report entries
            while (true)
            {
                auto entryOpt = reportQueue.get(false);
                if (not entryOpt.has_value())
                    break; // no element in queue

                // store it in diffs list
                ctx.diffs.emplace_back(std::move(*entryOpt));
                if (ctx.diffs.size() == 1)
                    // first element added; select it
                    winDetail.updateSelection();
                needRedraw = true;
            }

            // check if we still need to poll the queue
            if (reportQueue.isExhausted())
            {
                pollQueue = false;
                // clear spinner
                winList.footer.textLeft = "";
                needRedraw = true;
            }
            else
            {
                // update spinner
                if (--spinnerStepCountdown <= 0)
                {
                    winList.footer.textLeft = ctx.ui.spinnerStrings[spinnerIndex];
                    if (++spinnerIndex >= (int)ctx.ui.spinnerStrings.size())
                        spinnerIndex = 0;
                    spinnerStepCountdown = ctx.ui.spinnerStepCount;
                    needRedraw = true;
                }
            }
        }

        if (needRedraw)
        {
            redraw();
            needRedraw = false;
        }

        const termui::Event event = ctx.tmui.waitForEvent(ctx.ui.cycleTimeMs);
        switch (event.value())
        {
        case termui::Event::kTermResize:
            needRedraw = true;
            break;

        case termui::Event::kSigInt:
        case termui::Event::kSigTerm:
        case termui::Event::kCtrlC:
        case termui::Event::kEscape:
        case 'q':
        case 'Q':
            exit = true;
            break;

        case termui::Event::kArrowUp:
        case '4':
        case 'u':
            moveSelection(MoveKind::LineUp);
            needRedraw = true;
            break;

        case termui::Event::kArrowDown:
        case '1':
        case 'j':
            moveSelection(MoveKind::LineDown);
            needRedraw = true;
            break;

        case termui::Event::kPageUp:
        case 'i':
            moveSelection(MoveKind::PageUp);
            needRedraw = true;
            break;

        case termui::Event::kPageDown:
        case 'k':
            moveSelection(MoveKind::PageDown);
            needRedraw = true;
            break;

        case termui::Event::kHome:
            moveSelection(MoveKind::Top);
            needRedraw = true;
            break;

        case termui::Event::kEnd:
            moveSelection(MoveKind::Bottom);
            needRedraw = true;
            break;

        case '5':
        case 'o':
            winDetail.move(MoveKind::LineUp);
            needRedraw = true;
            break;

        case '2':
        case 'l':
            winDetail.move(MoveKind::LineDown);
            needRedraw = true;
            break;

        case '6':
        case 'p':
            winDetail.move(MoveKind::PageUp);
            needRedraw = true;
            break;

        case '3':
        case ';':
        case 'm':
            winDetail.move(MoveKind::PageDown);
            needRedraw = true;
            break;

        default:
            // unknown or no key: ignore
            break;
        }
    }
}