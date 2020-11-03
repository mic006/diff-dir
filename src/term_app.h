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
 * 
 * The report display is heavily inspired from `tig` (https://github.com/jonas/tig):
 * - 2 half screens
 *   - side to side when the width is sufficient
 *   - top / bottom otherwise
 *   - each has a header and a footer
 * - first half contains the list of differences, one per line
 * - second half displays details on the currently selected difference of the first list
 *   - details on common or difference for size, mtime, ownership and permissions
 *   - output of diff for files
 * - different keys to scroll on each half screen
 */

#pragma once

#include <thread>
#include <vector>

#include "concurrent.h"
#include "report.h"
#include "term_app_settings.h"
#include "text_diff.h"

/// Difference entry, enhancing the ReportEntry
struct DiffEntry
{
    DiffEntry(ReportEntry &&_reportEntry)
        : reportEntry{std::move(_reportEntry)}, details{} {}

    ReportEntry reportEntry;
    std::vector<std::u32string> details;
};

/// Movement of the content inside windows
enum class MoveKind
{
    Top,
    PageUp,
    LineUp,
    LineDown,
    PageDown,
    Bottom,
};

/// Shared application context
struct TermAppContext
{
    TermAppContext(const Context &_diffDirCtx)
        : diffDirCtx{_diffDirCtx}, tmui{}, ui{_diffDirCtx}, diffs{}, selectedIndex{0}, uidgidReader{}, textDiff{ui} {}

    // not copyable (detect unwanted copies)
    TermAppContext(const TermAppContext &) = delete;
    TermAppContext &operator=(const TermAppContext &) = delete;

    DiffEntry *getSelected()
    {
        return (selectedIndex >= 0 and selectedIndex < (int)diffs.size()) ? &diffs[selectedIndex] : nullptr;
    }

    const Context &diffDirCtx; ///< diff dir context
    termui::TermUi tmui;       ///< TermUi instance to manage the terminal
    TermAppSettings ui;        ///< ui settings

    // app internal data
    std::vector<DiffEntry> diffs;  ///< difference list
    int selectedIndex;             ///< index of item currently selected
    UidGidNameReader uidgidReader; ///< get uid / gid names
    TextDifference textDiff;       ///< handler to compute difference on text files
};

/// Multiple fields on a single line
struct TermAppMultiFieldsLine
{
    std::string textLeft;   ///< text on left side
    std::string textMiddle; ///< text centered on the line
    std::string textRight;  ///< text on the right side
};

/// One window (half of screen) of the application
struct TermAppWindow
{
    TermAppWindow(TermAppContext &_ctx)
        : ctx{_ctx}, header{}, footer{},
          origY{0}, origX{0}, height{0}, width{0},
          firstDisplayedIndex{0},
          scrollIndStart{0}, scrollIndEnd{0} {}
    virtual ~TermAppWindow() = default;

    // not copyable (detect unwanted copies)
    TermAppWindow(const TermAppWindow &) = delete;
    TermAppWindow &operator=(const TermAppWindow &) = delete;

    /// Get size of the window content
    virtual int getContentSize() const = 0;

    /// Define the window position on the screen
    void setWinPos(int _origY, int _origX, int _height, int _width);

    /// Draw the window
    void draw();

    /// Draw one line of content
    virtual void drawContentLine(int y, int contentIndex) = 0;

    /// Determine the content to be displayed
    virtual void determineDisplayContent(int innerHeight, int contentSize);

    /// Whether scrollbar shall be shown at the given index
    bool showScrollbar(int y) const
    {
        return y >= scrollIndStart and y < scrollIndEnd;
    }

    TermAppContext &ctx;           ///< shared context to application
    std::string header;            ///< header content
    TermAppMultiFieldsLine footer; ///< footer content
    int origY;                     ///< start of window
    int origX;                     ///< start of window
    int height;                    ///< height of window
    int width;                     ///< width of window
    int firstDisplayedIndex;       ///< index of first item displayed
    int scrollIndStart;            ///< start index for vertical scrollbar indication
    int scrollIndEnd;              ///< end index for vertical scrollbar indication
};

/// List of differences window
struct TermAppListWindow : public TermAppWindow
{
    TermAppListWindow(TermAppContext &_ctx)
        : TermAppWindow{_ctx} {}
    ~TermAppListWindow() override = default;

    int getContentSize() const override
    {
        return ctx.diffs.size();
    }

    void drawContentLine(int y, int contentIndex) override;

    void determineDisplayContent(int innerHeight, int contentSize) override;

    /// Move the selection
    void moveSelection(MoveKind mv);
};

/// Details of one difference window
struct TermAppDetailWindow : public TermAppWindow
{
    TermAppDetailWindow(TermAppContext &_ctx);
    ~TermAppDetailWindow() override = default;

    void updateSelection();

    int getContentSize() const override
    {
        const DiffEntry *entry = ctx.getSelected();
        return entry == nullptr ? 0 : entry->details.size();
    }

    void drawContentLine(int y, int contentIndex) override;

    /// Move the window for the displayed content
    void move(MoveKind mv);

private:
    /// Formatted string with display size
    struct FormattedString
    {
        std::u32string str;
        int displayLength;
    };
    /// Get max display length from a list
    static int maxDisplayLength(const std::vector<FormattedString> &v);

    /// Add metadata information when file exists only on one side
    void addMetadataSingleFile(const FileEntry &file, Side side);

    /// Add a line to metadata
    void addMetadataSimpleLineCommon(const std::u32string &title,
                                     const std::u32string &common);
    void addMetadataSimpleLineDiffers(const std::u32string &title,
                                      const std::u32string &left,
                                      const std::u32string &right);
    void addMetadataSimpleLineDiffers(const std::u32string &title,
                                      const std::u32string &left,
                                      const std::u32string &right,
                                      bool swap);
    void addMetadataSimpleLineWarning(const std::u32string &title,
                                      const std::u32string &left,
                                      const std::u32string &right);

    // effects to create strings
    const char32_t metadataBg;
    const char32_t titleStart;
    const char32_t titleEnd;
    const char32_t differenceL;
    const char32_t differenceR;
    const char32_t normal;
    const std::u32string warningStr;
    const std::u32string metadataStr;

    // fields for metadata comparison
    std::vector<FormattedString> fieldsTitle;
    std::vector<FormattedString> fieldsLeft;
    std::vector<FormattedString> fieldsRight;
};

/// Terminal application
struct TermApp
{
    TermApp(Context &_diffDirCtx, const std::string &title);
    ~TermApp();

    // not copyable (detect unwanted copies)
    TermApp(const TermApp &) = delete;
    TermApp &operator=(const TermApp &) = delete;

    /// Redraw completely the application
    void redraw();

    /// Move the selection
    void moveSelection(MoveKind mv);

    /// Thread loop
    void run();

    Context &diffDirCtx;                      ///< context for the comparison
    TermAppContext ctx;                       ///< context to perform drawing via TermUi
    TermAppListWindow winList;                ///< window handling the list of differences
    TermAppDetailWindow winDetail;            ///< window handling the details of one difference
    ConcurrentQueue<ReportEntry> reportQueue; ///< queue giving report entry from diff-dir algo
    int spinnerIndex;                         ///< index of current string for spinner
    int spinnerStepCountdown;                 ///< countdown to keep current string of spinner on display
    std::jthread appThread;                   ///< thread running the event loop
};