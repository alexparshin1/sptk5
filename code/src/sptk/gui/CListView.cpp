/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/cutils>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <cmath>

#include <sptk5/CSmallPixmapIDs.h>
#include <sptk5/db/Query.h>
#include <sptk5/gui/CListView.h>
#include <sptk5/gui/CThemes.h>

using namespace std;
using namespace sptk;

// default header size
constexpr int HEADER_HEIGHT = 20;

void CListView::scrollbar_callback(Fl_Widget* scrollbar, void*)
{
    ((CListView*) (scrollbar->parent()))->top(((CScrollBar*) scrollbar)->value());
}

void CListView::hscrollbar_callback(Fl_Widget* scrollbar, void*)
{
    ((CListView*) (scrollbar->parent()))->hposition(((CScrollBar*) scrollbar)->value());
}

constexpr int defaultScrollBarWidth = 16;
int CListView::m_scrollbarWidth = defaultScrollBarWidth;

void CListView::scrollbar_width(int width)
{
    m_scrollbarWidth = width;
}

int CListView::scrollbar_width()
{
    return m_scrollbarWidth;
}

// Cause minimal update to redraw the given item:

void CListView::redraw_line(unsigned line)
{
    if (!m_redraw1 || m_redraw1 == (int) line)
    {
        m_redraw1 = (int) line;
        damage(FL_DAMAGE_EXPOSE);
    }
    else if (!m_redraw2 || m_redraw2 == (int) line)
    {
        m_redraw2 = (int) line;
        damage(FL_DAMAGE_EXPOSE);
    }
    else
    {
        damage(FL_DAMAGE_SCROLL);
    }
}

void CListView::hposition(int xPosition)
{
    if (xPosition < 0)
    {
        xPosition = 0;
    }
    if (xPosition == m_horizPosition)
    {
        return;
    }
    m_horizPosition = xPosition;
    if (xPosition != m_realHorizPosition)
    {
        redraw_lines();
    }
}

void CListView::ctor_init()
{
    m_dataMode = CListViewDataMode::LV_DATA_UNDEFINED;
    m_activeRow = 0;
    m_capitalizeColumnNames = false;
    m_fillTerminated = false;
    m_fillInProgress = false;
    m_multipleSelection = true;
    box(FL_NO_BOX);
    m_horizPosition = m_realHorizPosition = 0;
    m_top = 0;
    m_headerHeight = HEADER_HEIGHT;
    when(FL_WHEN_RELEASE_ALWAYS);
    selection_color(FL_SELECTION_COLOR);
    scrollbar.callback(scrollbar_callback);
    hscrollbar.callback(hscrollbar_callback);
    scrollbar.visible_focus(false);
    hscrollbar.visible_focus(false);
    hscrollbar.type(FL_HORIZONTAL);
    m_maxWidth = 0;
    m_redraw1 = m_redraw2 = 0;
    end();

    m_control = this;

    align(Fl_Align(FL_ALIGN_LEFT | FL_ALIGN_TOP));

    m_draggingColumn = -1;
    m_showGrid = false;
    m_showStripes = false;
    m_showSelection = true;
    m_autoRowHeight = false;
    m_textFont = FL_HELVETICA;
    m_textSize = (uchar) FL_NORMAL_SIZE;
    m_textColor = FL_FOREGROUND_COLOR;
    m_currentTextFont = m_textFont;
    m_currentTextSize = m_textSize;
    m_iconNames = smallPixmapIDs;
}

CListView::CListView(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
    : CControl(label, layoutSize, layoutAlignment)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__

CListView::CListView(int x, int y, int w, int h, const char* l)
    : CControl(x, y, w, h, l)
    , scrollbar(0, 0, 0, 0)
    , hscrollbar(0, 0, 0, 0)
{
    ctor_init();
}
#endif

// the scrollbars are resized & placed by draw(), since each one's size
// depends on whether the other is visible or not.  This skips over
// Fl_Group::resize since it moves the scrollbars uselessly.

void CListView::resize(int xPosition, int yPosition, int width, int height)
{
    Fl_Widget::resize(xPosition, yPosition, width, height);

    // Determine the optimal scroll bar width
    int sbh = height;
    m_scrollbarWidth = 0;
    scrollbar.preferredSize(m_scrollbarWidth, sbh);

    scrollbar.resize(xPosition + width - m_scrollbarWidth, yPosition, m_scrollbarWidth, height - m_scrollbarWidth);
    hscrollbar.resize(xPosition, yPosition + height - m_scrollbarWidth, width - m_scrollbarWidth, m_scrollbarWidth);

    bbox(xPosition, yPosition, width, height);

    if (m_menuButton)
    {
        m_menuButton->resize(xPosition, yPosition, width, height);
    }
}

void CListView::displayRow(unsigned index)
{
    if (index >= m_rows.size())
    {
        return;
    }
    if (displayed(index))
    {
        return;
    }

    int xPos {0};
    int yPos {0};
    int width {0};
    int height {0};
    bbox(xPos, yPos, width, height);

    // find the bottom
    int hh = 0;
    bool lastPage = true;
    for (unsigned i = index; i < m_rows.size(); i++)
    {
        int rh = m_rows[i]->height;
        hh += rh;
        if (hh > height)
        {
            lastPage = false;
            break;
        }
    }

    if (lastPage)
    { // Now find top
        for (int i = (int) index - 1; i >= 0; i--)
        {
            int rh = m_rows[i]->height;
            hh += rh;
            if (hh > height)
            {
                m_top = unsigned(i);
                break;
            }
        }
    }
    else
    {
        hh = 0;
        height /= 2;
        m_top = index;
        for (int i = (int) index; i >= 0; i--)
        {
            int rh = m_rows[i]->height;
            hh += rh;
            if (hh > height)
            {
                m_top = unsigned(i);
                break;
            }
        }
    }

    redraw();
}

// redraw, has side effect of updating top and setting scrollbar:

void CListView::draw()
{
    if (m_currentTextFont != m_textFont || m_currentTextSize != m_textSize)
    {
        m_currentTextFont = m_textFont;
        m_currentTextSize = m_textSize;
        item_compute_height_all();
    }
    const Fl_Color bgColor = FL_BACKGROUND2_COLOR;
    int drawsquare = 0;
    if (damage() & FL_DAMAGE_ALL)
    { // redraw the box if full redraw
        Fl_Boxtype b = box() ? box() : FL_DOWN_BOX;
        draw_box(b, x(), y(), w(), h(), bgColor);
        drawsquare = 1;
    }

    scrollbar.clear_visible();
    hscrollbar.clear_visible();

    int full_width_ = fullWidth();
    int full_height_ = fullHeight();

    int xPos {0};
    int yPos {0};
    int width {0};
    int height {0};
    bbox(xPos, yPos, width, height);

    if (m_top >= m_rows.size())
    {
        m_top = 0;
    }

    // see if scrollbar needs to be switched on/off:
    for (int iteration = 0; iteration < 2; iteration++)
    {
        if (full_height_ > height)
        {
            if (!scrollbar.visible())
            {
                scrollbar.set_visible();
                width -= m_scrollbarWidth;
                drawsquare = 1;
            }
        }
        else
        {
            m_top = 0;
            if (scrollbar.visible())
            {
                scrollbar.clear_visible();
                clear_damage(uchar(damage() | FL_DAMAGE_SCROLL));
            }
        }

        if (full_width_ > width)
        {
            if (!hscrollbar.visible())
            {
                hscrollbar.set_visible();
                height -= m_scrollbarWidth;
                drawsquare = 1;
            }
        }
        else
        {
            m_realHorizPosition = 0;
            if (hscrollbar.visible())
            {
                hscrollbar.clear_visible();
                clear_damage(uchar(damage() | FL_DAMAGE_SCROLL));
            }
        }
    }

    if (m_headerHeight)
    {
        header_draw(xPos - m_horizPosition, yPos - (int) m_headerHeight, m_headerHeight);
    }

    fl_push_clip(xPos, yPos, width, height);

    int l = top();

    int yy = 0;
    m_maxWidth = item_width(uint32_t(l));

    draw_box(FL_FLAT_BOX, xPos, yPos, width, height, bgColor);

    int maxl = size();
    for (; l < maxl && yy < height; l++)
    {
        int hh = item_height(uint32_t(l));
        if (hh <= 0)
        {
            continue;
        }
        if ((damage() & (FL_DAMAGE_SCROLL | FL_DAMAGE_ALL)) || l == m_redraw1 || l == m_redraw2)
        {
            fl_push_clip(xPos, yPos + yy, width, hh);
            int textAlign = FL_ALIGN_TOP;
            if (m_autoRowHeight)
            {
                textAlign |= FL_ALIGN_WRAP;
            }
            item_draw(uint32_t(l), nullptr, xPos - m_horizPosition, yy + yPos, width + m_horizPosition, hh, Fl::focus() == this,
                      textAlign,
                      false);
            fl_pop_clip();
        }
        yy += hh;
    }

    fl_pop_clip();

    m_redraw1 = m_redraw2 = -1;

    // update the scrollbars and redraw them:
    int dy = m_top ? item_height(m_top) : 0;
    if (dy < 10)
    {
        dy = 10;
    }
    if (scrollbar.visible())
    {
        int visibleRows = 0;
        int hh = 0;
        for (unsigned i = m_top; i < m_rows.size(); i++)
        {
            hh += m_rows[i]->height;
            if (hh < height)
            {
                visibleRows++;
            }
            else
            {
                break;
            }
        }
        scrollbar.damage_resize(
            scrollbar.align() & FL_ALIGN_LEFT ? xPos - m_scrollbarWidth : xPos + width,
            yPos, m_scrollbarWidth, height);
        scrollbar.value(m_top, visibleRows, 0, m_rows.size());
        scrollbar.linesize(1);
        if (drawsquare)
        {
            draw_child(scrollbar);
        }
        else
        {
            update_child(scrollbar);
        }
    }
    if (hscrollbar.visible())
    {
        hscrollbar.damage_resize(
            xPos, scrollbar.align() & FL_ALIGN_TOP ? yPos - m_scrollbarWidth : yPos + height,
            width, m_scrollbarWidth);
        hscrollbar.value(m_horizPosition, width, 0, full_width_);
        hscrollbar.linesize(dy);
        if (drawsquare)
        {
            draw_child(hscrollbar);
        }
        else
        {
            update_child(hscrollbar);
        }
    }

    // draw that little square between the scrollbars:
    if (drawsquare && scrollbar.visible() && hscrollbar.visible())
    {
        fl_color(bgColor);
        fl_rectf(scrollbar.x(), hscrollbar.y(), m_scrollbarWidth, m_scrollbarWidth);
    }

    m_realHorizPosition = m_horizPosition;
}

bool CListView::item_selected(unsigned index) const
{
    return (m_rows[index]->flags & CLV_SELECTED) != 0;
}

void CListView::item_select(unsigned index, bool v)
{
    if (index >= m_rows.size())
    {
        return;
    }
    CPackedStrings* item = m_rows[index];
    if (v)
    {
        m_selection.select(item);
    }
    else
    {
        m_selection.deselect(item);
    }
}

void CListView::removeRow(unsigned line)
{
    if (line > m_rows.size())
    {
        return;
    }
    m_selection.deselect(m_rows[line]);
    if (displayed(line))
    {
        redraw_lines();
    }
    m_rows.remove(line);
}

int CListView::item_height(unsigned index) const
{
    try
    {
        const auto* row = m_rows[index];
        if (row->flags & CLV_NOTDISPLAYED)
        {
            return 0;
        }
        int h = row->height;
        if (h < 6)
        {
            h = 6;
        }
        return h;
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }

    return 0;
}

int CListView::item_compute_height_all()
{
    unsigned count = m_rows.size();
    unsigned totalHeight = 0;
    for (unsigned i = 0; i < count; i++)
    {
        totalHeight += item_compute_height(m_rows[i]);
    }
    m_rows.m_fullHeight = totalHeight;
    return (int) totalHeight;
}

int CListView::item_compute_height(CPackedStrings* l)
{
    if (l->flags & CLV_NOTDISPLAYED)
    {
        return 0;
    }

    if (fl_font() != m_textFont || fl_size() != m_textSize)
    {
        fl_font(m_textFont, m_textSize);
    }

    const auto& cells = *l;

    // For blank lines set the height to exactly 1 line!
    unsigned hmax = m_textSize;
    unsigned borderWidth = cellBorderWidth();

    if (cells.size())
    {
        short colmax = cells.size();
        if (colmax > (short) m_columnList.size())
        {
            colmax = (short) m_columnList.size();
        }
        for (short c = 0; c < colmax; c++)
        {
            CColumn& column = m_columnList[c];
            if (column.visible())
            {
                const char* str = cells[c];
                int ch = 0;
                int cw = column.width() - (int) borderWidth;

                if (column.autoWidth())
                {
                    constexpr int maxColumnWidth = 400;
                    cw = maxColumnWidth;
                }

                if (column.autoWidth() || m_autoRowHeight)
                {
                    fl_measure(str, cw, ch, 0);
                }
                else
                {
                    ch = m_textSize + 2;
                }

                cw += (int) borderWidth;
                if (column.autoWidth())
                {
                    if (((int) kind() &
                         ((int) CControlKind::CHECKBUTTONS | (int) CControlKind::RADIOBUTTONS)) &&
                        c == 0)
                    {
                        cw += (m_textSize - 2) / 3 * 3 + 3;
                    }
                    if (cw > column.width())
                    {
                        column.width(int16_t(cw));
                    } // adjust column width for auto-width column
                }
                if (ch > (int) hmax)
                {
                    hmax = (unsigned) ch;
                }
            }
        }
    }

    hmax += borderWidth;

    if (constexpr int maxHeight = 254;
        hmax > maxHeight)
    {
        hmax = maxHeight;
    }
    l->height = (unsigned char) hmax;
    return (int) hmax;
}

int CListView::item_width(unsigned) const
{
    size_t width = 6;
    size_t count = m_columnList.size();
    for (size_t i = 0; i < count; i++)
    {
        if (m_columnList[i].visible())
        {
            width += m_columnList[i].width();
        }
    }

    return (int) width;
}

void CListView::sortColumn(int column, bool sortNow)
{
    if ((unsigned) column >= m_columnList.size())
    {
        return;
    }
    const auto& columnInfo = m_columnList[column];
    VariantDataType ctype = columnInfo.type();
    CPackedStrings* row = nullptr;
    if (m_activeRow < size())
    {
        row = m_rows[m_activeRow];
    }
    m_rows.sortColumn(column, ctype, sortNow);
    if (row && sortNow)
    {
        m_activeRow = (uint32_t) m_rows.indexOf(row);
        if (!displayed(m_activeRow))
        {
            displayRow(m_activeRow);
        }
        else
        {
            redraw();
        }
    }
}

int CListView::sortColumn() const
{
    return m_rows.sortColumn();
}

String CListView::sortColumnName() const
{
    const int ndx = sortColumn();
    if (ndx < 0)
    {
        return "";
    }
    return m_columnList[ndx].name();
}

void CListView::sortAscending(bool ascending, bool sortNow)
{
    CPackedStrings* row = nullptr;
    if (m_activeRow < size())
    {
        row = m_rows[m_activeRow];
    }
    m_rows.sortAscending(ascending, sortNow);
    if (row && sortNow)
    {
        m_activeRow = (uint32_t) m_rows.indexOf(row);
        if (!displayed(m_activeRow))
        {
            displayRow(m_activeRow);
        }
        else
        {
            redraw();
        }
    }
}

void CListView::header_draw(int xPosition, int yPosition, int headerHeight) const
{
    fl_push_clip(x() + 2, yPosition, w() - 3, headerHeight);
    fl_font(m_textFont, m_textSize);
    const size_t count = m_columnList.size();
    for (size_t i = 0; i < count; i++)
    {
        const CColumn& column = m_columnList[i];
        const bool sortColumn = (int(i) == m_rows.sortColumn());
        if (column.visible())
        {
            int width = column.width();
            int markSize = headerHeight - 8;
            if (!CThemes::drawButton(THM_BUTTON_COMBO, xPosition, yPosition, width, headerHeight, sortColumn, false, false))
            {
                // Workaround for the FLTK bug..
                draw_box(FL_FLAT_BOX, xPosition, yPosition, width, headerHeight, FL_BACKGROUND_COLOR);
                draw_box(FL_UP_FRAME, xPosition, yPosition, width, headerHeight, FL_BACKGROUND_COLOR);
            }
            int tx = xPosition + 3;
            const int twidth = width - 6;
            int clipWidth = twidth;
            if (sortColumn)
            {
                clipWidth -= markSize;
            }
            fl_color(FL_FOREGROUND_COLOR);
            fl_push_clip(xPosition, yPosition, clipWidth, headerHeight);
            fl_draw(column.name().c_str(), tx, yPosition, twidth, headerHeight, FL_ALIGN_LEFT);
            fl_pop_clip();
            if (sortColumn)
            {
                int yc = yPosition + headerHeight / 2;
                const int xc = tx + twidth - markSize;
                if (sortAscending())
                {
                    fl_color(FL_DARK3);
                    fl_line(xc + markSize / 2, yc + markSize / 2, xc, yc - markSize / 2, xc + markSize,
                            yc - markSize / 2);
                    fl_color(FL_LIGHT3);
                    fl_line(xc + markSize / 2, yc + markSize / 2, xc + markSize, yc - markSize / 2);
                }
                else
                {
                    fl_color(FL_LIGHT3);
                    fl_line(xc + markSize / 2, yc - markSize / 2, xc + markSize, yc + markSize / 2, xc,
                            yc + markSize / 2);
                    fl_color(FL_DARK3);
                    fl_line(xc, yc + markSize / 2, xc + markSize / 2, yc - markSize / 2);
                }
            }
            xPosition += width;
        }
    }
    draw_box(FL_FLAT_BOX, xPosition, yPosition, x() + w() - xPosition + 10, headerHeight, FL_BACKGROUND_COLOR);
    fl_pop_clip();
}

Fl_Color CListView::item_color(int, const CPackedStrings&) const
{
    return m_textColor;
}

unsigned CListView::cellBorderWidth() const
{
    unsigned bwidth = 2;
    if (m_showGrid)
    {
        bwidth = 4;
    }
    return bwidth;
}

void CListView::item_draw(
    unsigned index, const CPackedStrings* item, int xx, int yy, int ww, int hh, int focusMode, int verticalAlign,
    bool paintBackground) const
{
    bool selected = false;
    if (!item)
    {
        if ((int) index < 0)
        {
            item = nullptr;
        }
        else
        {
            item = m_rows[index];
        }
    }
    if (item)
    {
        selected = (item->flags & CLV_SELECTED) != 0;
    }
    Fl_Color lcol = FL_FOREGROUND_COLOR;
    Fl_Color boxcol = FL_BACKGROUND2_COLOR;
    if (m_showStripes && (index & 0x1))
    {
        boxcol = fl_darker(boxcol);
    }

    if (selected && m_showSelection)
    {
        if (focusMode == -1)
        {
            lcol = FL_FOREGROUND_COLOR;
        }
        else
        {
            lcol = fl_contrast(lcol, selection_color());
            boxcol = selection_color();
        }
    }

    fl_font(m_textFont, m_textSize);
    size_t cnt = m_columnList.size();
    if (item && (size_t) item->size() < cnt)
    {
        cnt = item->size();
    }
    auto align = Fl_Align(FL_ALIGN_LEFT | verticalAlign);
    auto borderWidth = (int) cellBorderWidth();
    int x = xx;
    int xf = xx + borderWidth - 1;
    const int maxX = xx + ww;

    if (paintBackground || boxcol != FL_LIGHT3)
    {
        draw_box(FL_FLAT_BOX, xx, yy, ww, hh, boxcol);
    }

    for (unsigned i = 0; i < cnt; i++)
    {
        const CColumn& column = m_columnList[i];
        if (column.visible())
        {
            int xt = x + borderWidth - 1;
            const int yt = yy + borderWidth - 1;
            const int w1 = column.width();

            if (x + w1 > 0 && x <= maxX)
            {
                if (m_showGrid)
                {
                    draw_box(FL_THIN_DOWN_FRAME, x, yy, w1, hh, FL_GRAY);
                }

                const char* str = "";
                if (item)
                {
                    str = (*item)[short(i)];
                }

                int wt = w1 - borderWidth * 2;

                if (((int) kind() & ((int) CControlKind::CHECKBUTTONS | (int) CControlKind::RADIOBUTTONS)) &&
                    i == 0)
                {
                    const int bw = m_textSize;
                    const int xb = xt + bw;
                    const int yb = yt + bw;

                    if (kind() == CControlKind::CHECKBUTTONS)
                    {
                        draw_box(FL_DOWN_BOX, xt, yt, bw + 1, bw + 1, FL_WHITE);

                        if (selected)
                        {
                            fl_color(FL_FOREGROUND_COLOR);
                            // left top to right bottom
                            fl_line(xt + 3, yt + 3, xb - 3, yb - 3);
                            fl_line(xt + 3, yt + 4, xb - 4, yb - 3);
                            fl_line(xt + 4, yt + 3, xb - 3, yb - 4);
                            // right top to left bottom
                            fl_line(xb - 3, yt + 3, xt + 3, yb - 3);
                            fl_line(xb - 4, yt + 3, xt + 3, yb - 4);
                            fl_line(xb - 3, yt + 4, xt + 4, yb - 3);
                        }
                    }
                    else if (kind() == CControlKind::RADIOBUTTONS)
                    {
                        draw_box(FL_ROUND_DOWN_BOX, xt, yt, bw + 1, bw + 1, FL_WHITE);
                        if (selected)
                        {
                            fl_color(FL_FOREGROUND_COLOR);
                            constexpr int fullRound = 360;
                            fl_pie(xt + 4, yt + 4, bw - 7, bw - 7, 0, fullRound);
                            fl_arc(xt + 4, yt + 4, bw - 7, bw - 7, 0, fullRound);
                        }
                    }
                    xt += bw + 3;
                    wt -= bw + 3;
                    xf = xt;
                }

                if (item)
                {
                    fl_push_clip(xt, yy, wt, hh);
                    if ((int) column.type() &
                        ((int) VariantDataType::VAR_IMAGE_PTR | (int) VariantDataType::VAR_IMAGE_NDX))
                    {
                        char* endptr {nullptr};
                        Fl_Image* image = nullptr;
#ifdef _WIN32
                        uint64_t value = strtol(str, &endptr, 16);
#else
                        auto value = (uint64_t) strtoll(str, &endptr, 16);
#endif
                        if (column.type() == VariantDataType::VAR_IMAGE_PTR)
                        {
                            image = (Fl_Image*) value;
                        }
                        else
                        {
                            if (value < m_iconNames.size())
                            {
                                CIcon* icon = CThemes::getIcon(m_iconNames[value], CIconSize::IS_SMALL_ICON);
                                if (icon)
                                {
                                    image = icon->image();
                                }
                            }
                        }
                        if (image)
                        {
                            image->draw(xt, yy + 2, image->w(), image->h(), 0, 0);
                        }
                    }
                    else
                    {
                        if (selected)
                        {
                            fl_color(lcol);
                        }
                        else
                        {
                            fl_color(item_color((int) i, *item));
                        }
                        fl_draw(str, xt, yy, wt, hh, align, nullptr, 0);
                    }
                    fl_pop_clip();
                }
            }
            x += w1;
        }
    }

    // Draw the focus rect if necessary
    if (index == m_activeRow && Fl::focus() == (Fl_Widget*) this)
    {
        draw_focus(FL_FLAT_BOX, xf - 1, yy + 1, x - xf + 2, hh - 1);
    }

    const int finalWidth = ww - x + this->x() + 10;
    if (finalWidth > 0)
    {
        draw_box(FL_FLAT_BOX, x, yy, finalWidth, hh, boxcol);
    }
}

void CListView::clear()
{
    m_top = 0;
    m_horizPosition = m_realHorizPosition = 0;
    m_rows.clear();
    m_selection.clear();
    m_maxWidth = 0;
    redraw_lines();
}

void CListView::addRow(CPackedStrings* ptr)
{
    item_compute_height(ptr);
    m_rows.add(ptr);
}

void CListView::addRow(int ident, const Strings& ss)
{
    auto* packedStrings = new CPackedStrings(ss);
    if (ident)
    {
        packedStrings->argument(ident);
    }
    addRow(packedStrings);
}

void CListView::insertRow(unsigned position, const Strings& ss, int ident)
{
    auto* packedStrings = new CPackedStrings(ss);
    if (ident)
    {
        packedStrings->argument(ident);
    }
    item_compute_height(packedStrings);
    m_rows.insert(position, packedStrings);
}

void CListView::updateRow(unsigned position, CPackedStrings* ptr)
{
    int oldh = item_height(position);
    CPackedStrings* row = m_rows[position];
    *row = *ptr;
    int dh = item_compute_height(row) - oldh;
    m_rows.m_fullHeight += dh;
}

void CListView::updateRow(unsigned position, const Strings& ss, int ident)
{
    const int oldh = item_height(position);
    CPackedStrings* row = m_rows[position];
    *row = ss;
    if (ident)
    {
        row->argument(ident);
    }

    const int dh = item_compute_height(row) - oldh;
    m_rows.m_fullHeight += dh;
}

CPackedStrings* CListView::row(unsigned line) const
{
    return m_rows[line];
}

int CListView::selected(int line) const
{
    const CPackedStrings* r = m_rows[line];
    if (r)
    {
        return r->flags & CLV_SELECTED;
    }
    else
    {
        return 0;
    }
}

void CListView::show(int line)
{
    CPackedStrings* t = m_rows[line];
    if (!t)
    {
        return;
    }
    if (t->flags & CLV_NOTDISPLAYED)
    {
        t->flags &= ~CLV_NOTDISPLAYED;
        m_rows.m_fullHeight += item_compute_height(t);
        if (displayed(uint32_t(line)))
        {
            redraw_lines();
        }
    }
}

void CListView::hide(int line)
{
    CPackedStrings* t = m_rows[line];
    if (!t)
    {
        return;
    }
    if (!(t->flags & CLV_NOTDISPLAYED))
    {
        m_rows.m_fullHeight -= item_height(uint32_t(line));
        t->flags |= CLV_NOTDISPLAYED;
        if (displayed((unsigned) line))
        {
            redraw_lines();
        }
    }
}

int CListView::visible(int line) const
{
    const auto* t = m_rows[line];
    if (!t)
    {
        return false;
    }
    return !(t->flags & CLV_NOTDISPLAYED);
}

std::string CListView::textValue() const
{
    const auto* row = selectedRow();
    if (!row)
    {
        return "";
    }
    return (*row)[0];
}

void CListView::textValue(const string& tv)
{
    const unsigned cnt = m_rows.size();
    bool dataWasChanged = false;
    void* dataValue = nullptr;

    if (const int column = m_rows.sortColumn();
        column >= -1)
    {
        for (unsigned line = 0; line < cnt; line++)
        {
            CPackedStrings* t = m_rows[line];
            bool wasSelected = (t->flags & CLV_SELECTED) != 0;
            if ((*t)[column] == tv)
            {
                activate_row(line);
                dataWasChanged = true;
                dataValue = t->user_data();
            }
            else if (wasSelected)
            {
                t->flags &= ~CLV_SELECTED;
                dataWasChanged = true;
            }
        }
    }
    if (dataWasChanged)
    {
        fireEvent(CEvent::DATA_CHANGED, (int32_t) (uint64_t) dataValue);
    }
    redraw();
}

Variant CListView::data() const
{
    CPackedStrings* t = selectedRow();
    if (!t)
    {
        switch (m_dataMode)
        {
            case CListViewDataMode::LV_DATA_KEY:
            case CListViewDataMode::LV_DATA_INDEX:
                return 0;
            default: // LV_DATA_CAPTION, LV_DATA_UNDEFINED
                return "";
        }
    }
    switch (m_dataMode)
    {
        case CListViewDataMode::LV_DATA_KEY:
            return t->argument();
        case CListViewDataMode::LV_DATA_INDEX:
            return selectedIndex();
        default: // LV_DATA_CAPTION, LV_DATA_UNDEFINED
            return (*t)[0];
    }
}

void CListView::data(const Variant& vv)
{
    unsigned cnt = m_rows.size();
    void* dataValue = nullptr;
    int intValue = vv.asInteger();
    const unsigned oldSelectedCount = m_selection.size();
    CPackedStrings* newSelectedRow = nullptr;

    const CPackedStrings* oldSelectedRow = oldSelectedCount > 0 ? &m_selection[0] : nullptr;

    switch (m_dataMode)
    {
        case CListViewDataMode::LV_DATA_KEY:
            newSelectedRow = findKey(vv.asInteger());
            break;
        case CListViewDataMode::LV_DATA_INDEX: {
            auto line = (unsigned) intValue;
            if (line < cnt)
            {
                newSelectedRow = m_rows[line];
            }
        }
        break;
        default: // LV_DATA_CAPTION, LV_DATA_UNDEFINED
            newSelectedRow = findCaption(vv.asString());
            break;
    }

    m_selection.deselectAll();
    m_selection.select(newSelectedRow);
    bool dataWasChanged = false;
    if ((newSelectedRow != nullptr && newSelectedRow != oldSelectedRow) || oldSelectedCount > 1)
    {
        dataWasChanged = true;
        displayRow((unsigned) selectedIndex());
        if (newSelectedRow)
        {
            dataValue = newSelectedRow->user_data();
        }
    }
    if (dataWasChanged)
    {
        fireEvent(CEvent::DATA_CHANGED, (int32_t) (uint64_t) dataValue);
    }
    redraw();
}

void CListView::showGrid(bool flag)
{
    if (m_showGrid != flag)
    {
        m_showGrid = flag;
        if (m_showGrid)
        {
            m_showStripes = false;
        }
        redraw();
    }
}

void CListView::showStripes(bool flag)
{
    if (m_showStripes != flag)
    {
        m_showStripes = flag;
        if (m_showStripes)
        {
            m_showGrid = false;
        }
        redraw();
    }
}

void CListView::columns(const CColumnList& _columns)
{
    m_columnList = _columns;
}

int CListView::columnAt(int xm)
{
    int xx = 0;
    m_draggingColumn = -1;
    for (int col = 0; col < (int) m_columnList.size(); col++)
    {
        const int leftDistance = xm - xx;
        xx += m_columnList[col].width();
        const int rightDistance = xx - xm;
        if (rightDistance < 0)
        {
            continue;
        }
        if (leftDistance < rightDistance)
        {
            if (leftDistance < 4)
            {
                m_draggingColumn = col - 1;
                fl_cursor(FL_CURSOR_WE);
                return -1;
            }
            return col;
        }

        if (leftDistance > rightDistance)
        {
            if (rightDistance < 4)
            {
                m_draggingColumn = col;
                fl_cursor(FL_CURSOR_WE);
                return -1;
            }
            return col;
        }
    }

    m_draggingStarted = xm;

    if (m_draggingColumn >= 0)
    {
        fl_cursor(FL_CURSOR_WE);
    }

    return -1;
}

void CListView::header_clicked(int x)
{
    const int column = columnAt(x);
    if (column >= 0)
    {
        if (sortColumn() == column)
        {
            // Same sort column clicked, reversing the sort order
            sortAscending(!sortAscending(), true);
        }
        else
        {
            // Sort the column ascending
            sortColumn(column, false);
            sortAscending(true, true);
        }
    }
}

bool CListView::header_drag_position(int x)
{
    int xx = 0;
    for (unsigned c = 0; c < m_columnList.size(); c++)
    {
        xx += m_columnList[c].width();
        if (abs(x - xx) < 3)
        {
            return true;
        }
        if (x < xx)
        {
            break;
        }
    }
    return false;
}

void CListView::header_dragged(int xm)
{
    if (m_draggingColumn < 0)
    {
        return;
    }
    int dx = xm - m_mousePressedX;
    CColumn& column = m_columnList[m_draggingColumn];
    int newWidth = column.width() + dx;
    if (newWidth < 1)
    {
        newWidth = 1;
    }
    column.width(short(newWidth));
    m_mousePressedX = xm;
    damage(FL_DAMAGE_SCROLL);
}

void CListView::header_released(int)
{
    m_draggingColumn = -1;
}

void CListView::key_changed(unsigned index)
{
    const auto* t = m_rows[index];
    int keyValue = 0;
    if (t)
    {
        keyValue = t->argument();
    }
    fireEvent(CEvent::DATA_CHANGED, keyValue);
}

// loading the key value in data entry dialog

void CListView::load(Query* loadQuery)
{
    Query& query = *loadQuery;
    if (!m_fieldName.length())
    {
        return;
    }
    Field& fld = query[m_fieldName.c_str()];
    data(*(Variant*) &fld);
}

// storing the key value in data entry dialog

void CListView::save(Query* updateQuery)
{
    if (!m_fieldName.length())
    {
        return;
    }
    QueryParameter& param = updateQuery->param(m_fieldName);
    param = data();
}

void CListView::item_clicked(int)
{
    // Should be defined in derived classes
}

int CListView::find_id(int id) const
{
    auto cnt = (int) m_rows.size();
    for (int i = 0; i < cnt; i++)
    {
        const auto* r = m_rows[i];
        if (id == r->argument())
        {
            return i;
        }
    }
    return -1;
}

void CListView::getSelections(vector<uint64_t>& sel) const
{
    sel.clear();
    if (!m_multipleSelection)
    {
        sel.push_back(unsigned(data().asInteger()));
    }
    else
    {
        unsigned cnt = m_rows.size();
        for (unsigned i = 0; i < cnt; i++)
        {
            const auto* r = m_rows[i];
            if (r->flags & CLV_SELECTED)
            {
                sel.push_back(unsigned(r->argument()));
            }
        }
    }
}

void CListView::setSelections(const vector<uint64_t>& sel)
{
    size_t scnt = sel.size();
    if (scnt)
    {
        data((int) sel[0]);
    }
    else
    {
        data(-1);
    }

    if (!m_multipleSelection)
    {
        return;
    }

    for (size_t si = 1; si < scnt; si++)
    {
        auto selectedKey = (int) sel[si];
        size_t cnt = m_rows.size();
        for (size_t i = 0; i < cnt; i++)
        {
            CPackedStrings* r = m_rows[(uint32_t) i];
            if (r->argument() == selectedKey)
            {
                r->flags |= CLV_SELECTED;
                continue;
            }
        }
    }
}

void CListView::autoRowHeight(bool arh)
{
    if (arh != m_autoRowHeight)
    {
        m_autoRowHeight = arh;
        item_compute_height_all();
        redraw_lines();
    }
}

int CListView::findString(const string& str, bool findAndSelect, unsigned startRow, unsigned endRow)
{
    unsigned cnt = m_rows.size();
    if (startRow >= cnt)
    {
        startRow = 0;
    }
    if (endRow <= startRow || endRow >= cnt)
    {
        endRow = cnt - 1;
    }

    int col = m_rows.sortColumn();
    if (col < 0)
    {
        col = 0;
    }
    String searchStr = lowerCase(str);
    for (uint32_t i = startRow; i <= endRow; i++)
    {
        const auto* row = m_rows[i];
        std::string currentStr((*row)[col]);
        size_t pos = lowerCase(currentStr).find(searchStr.c_str());
        if (pos != STRING_NPOS)
        {
            if (findAndSelect)
            {
                select_only(i);
                m_activeRow = i;
            }
            return i;
        }
    }
    return -1;
}

void CListView::top(int t)
{
    m_top = uint32_t(t);
    redraw();
}

bool CListView::preferredSize(int& w, int& h)
{
    constexpr int minSize = 100;
    if (w < minSize)
    {
        w = minSize;
    }
    if (h < minSize)
    {
        h = minSize;
    }
    return false;
}

void CListView::fill(DataSource& ds, const String& keyFieldName, unsigned recordsLimit, unsigned recordsEstimated,
                     CRefreshKind refreshKind)
{

    m_fillTerminated = false;
    if (m_fillInProgress)
    {
        m_fillTerminated = true;
        return;
    }

    do
    {
        m_fillTerminated = false;
        m_fillInProgress = true;

        fl_cursor(FL_CURSOR_WAIT);
        Fl::check();

        try
        {
            ds.open();

            auto fieldCount = (unsigned) ds.fieldCount();
            auto keyField = unsigned(-1);

            for (unsigned i = 0; i < fieldCount; i++)
            {
                if (ds[i].fieldName() == keyFieldName)
                {
                    keyField = i;
                    break;
                }
            }

            if (keyField == unsigned(-1))
            {
                // if keyFieldName is not defined (find the first integer field and use it)
                for (unsigned i = 0; i < fieldCount; i++)
                {
                    if (ds[i].dataType() == VariantDataType::VAR_INT)
                    {
                        keyField = i;
                        break;
                    }
                }
            }

            CColumnList newColumns;
            CColumnList& oldColumns = columns();
            for (unsigned f = 0; f < ds.fieldCount(); f++)
            {
                if (f == keyField)
                {
                    continue;
                }
                Field& field = ds[f];
                String columnName = field.fieldName();
                bool cvisible = columnName[0] != '_';
                columnName = columnName.replace("_", " ");
                if (m_capitalizeColumnNames)
                {
                    columnName = capitalizeWords(columnName);
                }
                auto cwidth = short(field.view().width + 1);
                VariantDataType ctype = field.dataType();
                switch (ctype)
                {
                    case VariantDataType::VAR_BOOL:
                        cwidth = 6;
                        break;

                    case VariantDataType::VAR_INT:
                        cwidth = 12;
                        break;

                    case VariantDataType::VAR_INT64:
                        cwidth = 20;
                        break;

                    case VariantDataType::VAR_FLOAT:
                        cwidth = 16;
                        break;

                    case VariantDataType::VAR_DATE_TIME:
                        cwidth = 24;
                        break;

                    case VariantDataType::VAR_DATE:
                        cwidth = 14;
                        break;

                    default:
                        break;
                }

                if (auto maxcw = short(columnName.length() + 1);
                    cwidth < maxcw)
                {
                    cwidth = maxcw;
                }
                if (cwidth > 50)
                {
                    cwidth = 50;
                }
                cwidth = short(cwidth * 7);


                if (int colIndex = oldColumns.indexOf(columnName.c_str());
                    colIndex < 0)
                {
                    newColumns.push_back(CColumn(columnName, ctype, cwidth, cvisible));
                }
                else
                {
                    const auto& existingColumn = oldColumns[colIndex];
                    newColumns.push_back(existingColumn);
                }
            }

            columns(newColumns);

            // Memorizing selected records IDs, if the keyField is defined
            int selectedCount = 0;
            int* selectedIDs = nullptr;
            if (keyField < unsigned(-1))
            {
                selectedCount = (int) m_selection.size();

                selectedIDs = new int[selectedCount];
                for (int i = 0; i < selectedCount; i++)
                {
                    selectedIDs[i] = m_selection[i].argument();
                }
            }
            m_selection.clear();

            int recordCount = 0;
            if (fieldCount > 0)
            {
                fireEvent(CEvent::PROGRESS, 0);

                Strings rowStrings;
                rowStrings.resize(fieldCount);

                unsigned lastProgression = 0;
                while (!ds.eof())
                {

                    auto listSize = (int) m_rows.size();

                    int keyValue = 0;
                    int j = 0;
                    for (unsigned i = 0; i < fieldCount; i++)
                    {
                        const auto& field = ds[i];
                        if (i == keyField)
                        {
                            keyValue = field.asInteger();
                            rowStrings.argument(keyValue);
                        }
                        else
                        {
                            rowStrings[j] = field.asString();
                            j++;
                        }
                    }

                    int rowNumber {0};
                    if (refreshKind == CRefreshKind::LV_REFRESH_FULL)
                    {
                        rowNumber = recordCount;
                        if (recordCount >= listSize)
                        {
                            addRow(new CPackedStrings(rowStrings));
                        }
                        else
                        {
                            CPackedStrings* row = m_rows[rowNumber];
                            int oldh = row->height;
                            *row = rowStrings;
                            int newh = item_compute_height(row);
                            m_rows.m_fullHeight += newh - oldh;
                        }
                    }
                    else
                    {
                        rowNumber = find_id(keyValue);
                        if (rowNumber < 0)
                        {
                            addRow(new CPackedStrings(rowStrings));
                        }
                        else
                        {
                            updateRow(uint32_t(rowNumber), rowStrings, keyValue);
                        }
                    }

                    recordCount++;
                    if (recordsLimit && recordCount >= (int) recordsLimit)
                    {
                        break;
                    }

                    if (recordsEstimated)
                    {
                        unsigned newProgression = (recordCount * 20 / recordsEstimated) * 5;
                        if (newProgression != lastProgression)
                        {
                            fireEvent(CEvent::PROGRESS, newProgression);
                            lastProgression = newProgression;
                        }
                    }

                    if (m_fillTerminated)
                    {
                        break;
                    }

                    try
                    {
                        ds.next();
                    }
                    catch (const Exception&)
                    {
                        ds.close();
                        throw;
                    }
                }
            }
            ds.close();

            if (refreshKind != CRefreshKind::LV_REFRESH_FULL)
            {
                recordCount = m_rows.size();
            }

            fireEvent(CEvent::PROGRESS, 100);

            // truncate and sort rows
            m_rows.truncate(recordCount);

            // Select previously selected rows
            for (int selectionIndex = 0; selectionIndex < selectedCount; selectionIndex++)
            {
                int keyValue = selectedIDs[selectionIndex];
                int rowNumber = find_id(keyValue);
                if (rowNumber >= 0)
                {
                    m_selection.select(m_rows[rowNumber]);
                }
            }

            delete[] selectedIDs;

            // check if we have any selection in SELECT_BROWSER
            if (!m_multipleSelection && !m_selection.size())
            {
                select(0, true);
            }

            if (unsigned(m_rows.sortColumn()) < columns().size())
            {
                m_rows.sort();
            }
        }
        catch (const Exception&)
        {
            m_fillInProgress = false;
            ds.close();
            fl_cursor(FL_CURSOR_DEFAULT);
            Fl::check();
            redraw();
            throw;
        }
    } while (m_fillTerminated);

    fl_cursor(FL_CURSOR_DEFAULT);
    Fl::check();

    m_fillInProgress = false;

    redraw();
}

int CListView::selectedIndex() const
{
    CPackedStrings* row = selectedRow();
    return m_rows.indexOf(row);
}

bool CListView::select_only(unsigned index, bool docallbacks)
{
    const auto* row = m_rows[index];
    bool change = false;
    if (row)
    {
        change = (row->flags & CLV_SELECTED) == 0;
        m_selection.deselectAll();
        if (select(index, true, docallbacks))
        {
            change = true;
        }
        m_activeRow = index;
        displayRow(index);
        if (change)
        {
            key_changed(index);
        }
    }
    return change;
}

// translate the current keystroke into up/down/left/right for navigation:
constexpr char ctrl(char x)
{
    return x ^ 0x40;
}

static int navkey()
{
    switch (Fl::event_key())
    {
        case 0: // not an FL_KEYBOARD/FL_SHORTCUT event
            break;
        case FL_Tab:
            if (!Fl::event_state(FL_SHIFT))
            {
                return FL_Right;
            }
            break;
        case 0xfe20: // XK_ISO_Left_Tab
            return FL_Left;
        case FL_Right:
        case FL_Left:
        case FL_Up:
        case FL_Down:
        case FL_Page_Up:
        case FL_Page_Down:
        case FL_Home:
        case FL_End:
            return Fl::event_key();
        default:
            switch (Fl::event_text()[0])
            {
                case ctrl('N'):
                    return FL_Down;
                case ctrl('P'):
                    return FL_Up;
                case ctrl('F'):
                    return FL_Right;
                case ctrl('B'):
                    return FL_Left;
                default:
                    break;
            }
            break;
    }
    return 0;
}

static int charkey(unsigned ch)
{
    if (ch >= 32 && ch <= 254)
    {
        return (int) ch;
    }
    else
    {
        return 0;
    }
}

int CListView::get_focus()
{
    if (contains(Fl::focus()))
    {
        return 1;
    }
    m_searchPhrase = "";
    Fl::focus(this);
    return 1;
}

int CListView::handle(int event)
{
    switch (event)
    {
        case FL_FOCUS:
            if (navkey())
            {
                int rc = get_focus();
                if (!m_hasFocus)
                {
                    m_hasFocus = true;
                    fireEvent(CEvent::FOCUS, 0);
                }
                return rc;
            }
            redraw();
            break;
        case FL_UNFOCUS:
            fireEvent(CEvent::UNFOCUS, 0);
            m_hasFocus = false;
            redraw();
            break;
        case FL_SHORTCUT:
        case FL_KEYBOARD:
            if (navkey())
            {
                switch (Fl::event_key())
                {
                    case FL_Down:
                        m_searchPhrase = "";
                        return select_next();
                    case FL_Page_Down:
                        m_searchPhrase = "";
                        return select_next_page();
                    case FL_Up:
                        m_searchPhrase = "";
                        return select_prior();
                    case FL_Page_Up:
                        m_searchPhrase = "";
                        return select_prior_page();
                    case FL_Home:
                        m_searchPhrase = "";
                        return select_first();
                    case FL_End:
                        m_searchPhrase = "";
                        return select_last();
                    default:
                        break;
                }
            }
            else
            {
                auto ch = (unsigned) Fl::event_key();
                if (m_multipleSelection)
                {
                    if (ch == ' ')
                    {
                        select(m_activeRow, !item_selected(m_activeRow), 1);
                    }
                    return 1;
                }
                // Type-in search
                size_t ln = m_searchPhrase.length();
                switch (ch)
                {
                    case FL_BackSpace:
                        if (ln)
                        {
                            m_searchPhrase[ln - 1] = 0;
                        }
                        break;
                    case FL_Escape:
                        m_searchPhrase = "";
                        return 1;
                    default:
                        break;
                }

                if (auto chr = (char) charkey(ch);
                    chr)
                {
                    std::string newSearchString = m_searchPhrase + chr;
                    int found = findString(newSearchString);
                    if (found != -1)
                    {
                        m_searchPhrase = newSearchString;
                        m_activeRow = uint32_t(found);
                    }
                }
                return 1;
            }
            break;

        default:
            break;
    }
    int X {0};
    int Y {0};
    int W {0};
    int H {0};
    bbox(X, Y, W, H);
    int my {0};
    unsigned l {0};
    static bool change;
    switch (event)
    {
        case FL_MOVE:
            if (Fl::event_inside(X, Y - (int) m_headerHeight, W, m_headerHeight) &&
                header_drag_position(Fl::event_x() + m_horizPosition - x()))
            {
                fl_cursor(FL_CURSOR_WE, FL_BLACK, FL_WHITE);
                break;
            }
            fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
            return 1;
        case FL_PUSH:
            get_focus();
            if (!m_hasFocus)
            {
                m_hasFocus = true;
                fireEvent(CEvent::FOCUS, 0);
            }
            m_mousePressedX = Fl::event_x() + m_horizPosition - x();
            if (!Fl::event_inside(X, Y, W, H))
            {
                if (Fl::event_inside(X, Y - (int) m_headerHeight, W, m_headerHeight))
                {
                    header_clicked(Fl::event_x() + m_horizPosition - x());
                    return 1;
                }
                break;
            }
            if (!m_multipleSelection)
            {
                m_selection.deselectAll();
            }
            my = Fl::event_y();
            change = false;
            l = (unsigned) find_item(my);
            if (m_multipleSelection && (Fl::event_state() & (FL_CTRL | FL_SHIFT)) != 0)
            {
                if (Fl::event_state() & FL_CTRL)
                { // Ctrl pressed
                    if (l < unsigned(-1))
                    {
                        bool selected = item_selected(l);
                        change = select(l, selected, when() & FL_WHEN_CHANGED);
                        m_activeRow = l;
                    }
                }
                else
                { // Shift pressed
                    if (m_selection.size())
                    {
                        CPackedStrings* ps = &m_selection[(int) m_selection.size() - 1];
                        int index = m_rows.indexOf(ps);
                        m_selection.deselectAll();
                        auto itemClicked = (int) l;
                        if (index < itemClicked)
                        {
                            int t = index;
                            index = itemClicked;
                            itemClicked = t;
                        }
                        if (itemClicked < 0)
                        {
                            itemClicked = 0;
                        }
                        if (index >= (int) m_rows.size())
                        {
                            index = (int) size() - 1;
                        }
                        for (int i = itemClicked; i <= index; i++)
                        {
                            m_selection.select(m_rows[i]);
                        }
                        change = true;
                        redraw();
                    }
                    else
                    {
                        change = select_only(l, when() & FL_WHEN_CHANGED);
                    }
                }
            }
            else
            {
                change = select_only(l, when() & FL_WHEN_CHANGED);
            }
            if (l < size())
            {
                item_clicked(Fl::event_key() - FL_Button);
            }
            if (Fl::event_clicks())
            {
                fireEvent(CEvent::MOUSE_DOUBLE_CLICK, 0);
            }
            else
            {
                fireEvent(CEvent::MOUSE_CLICK, 0);
            }
            break;
        case FL_DRAG:
            if (Fl::event_inside(X, Y - (int) m_headerHeight, W, m_headerHeight))
            {
                header_dragged(Fl::event_x() + m_horizPosition - x());
                return 1;
            }
            break;
        case FL_RELEASE:
            if (Fl::event_inside(X, Y - (int) m_headerHeight, W, m_headerHeight))
            {
                header_released(Fl::event_x() + m_horizPosition - x());
                fl_cursor(FL_CURSOR_DEFAULT);
                return 1;
            }
            if (change)
            {
                if (when() & FL_WHEN_RELEASE)
                {
                    fireEvent(CEvent::MOUSE_RELEASE, 0);
                }
                else if (!(when() & FL_WHEN_CHANGED))
                {
                    set_changed();
                }
            }
            break;
        default:
            break;
    }
    return Fl_Group::handle(event);
}

bool CListView::select_first()
{
    if (size() == 0)
    {
        return false;
    }
    m_top = 0;
    return activate_row(0);
}

bool CListView::select_last()
{
    if (size() == 0)
    {
        return false;
    }
    return activate_row(size() - 1);
}

bool CListView::select_next()
{
    int l = m_activeRow;
    // find next row to activate
    int newActiveRow = -1;
    while ((++l) < (int) size())
    {
        if (item_height(uint32_t(l)) > 0)
        {
            newActiveRow = l;
            break;
        }
    }
    if (newActiveRow < 0)
    {
        return false;
    }
    unsigned cnt = size();
    for (; m_top < cnt; m_top++)
    {
        if (!item_height(m_top))
        {
            continue;
        }
        if (displayed(uint32_t(newActiveRow)))
        {
            break;
        }
    }
    return activate_row(uint32_t(newActiveRow));
}

bool CListView::select_prior()
{
    int l = m_activeRow;
    // find next row to activate
    int newActiveRow = -1;
    while ((--l) >= 0)
    {
        if (item_height(uint32_t(l)) > 0)
        {
            newActiveRow = l;
            break;
        }
    }
    if (newActiveRow < 0)
    {
        return false;
    }
    for (int newTop = m_top; newTop >= 0; newTop--)
    {
        m_top = (uint32_t) newTop;
        if (!item_height(m_top))
        {
            continue;
        }
        if (displayed(uint32_t(newActiveRow)))
        {
            break;
        }
    }
    return activate_row(uint32_t(newActiveRow));
}

bool CListView::select_next_page()
{
    unsigned cnt = size();
    int X {0};
    int Y {0};
    int W {0};
    int H {0};
    int hh {0};
    bbox(X, Y, W, H);
    // find the new active row
    unsigned newActiveRow = m_activeRow;
    int row = m_activeRow;
    for (; row < (int) cnt; row++)
    {
        hh += item_height(uint32_t(row));
        if (hh >= H)
        {
            break;
        }
        newActiveRow = (unsigned) row;
    }
    if (newActiveRow == m_activeRow)
    {
        return false;
    }
    // find the new top
    hh = 0;
    int newTop = 0;
    for (row = (int) newActiveRow; row >= 0; row--)
    {
        hh += item_height(uint32_t(row));
        if (hh >= H)
        {
            break;
        }
        newTop = row;
    }
    m_top = (uint32_t) newTop;
    return activate_row(newActiveRow);
}

bool CListView::select_prior_page()
{
    int X {0};
    int Y {0};
    int W {0};
    int H {0};
    int hh {0};
    bbox(X, Y, W, H);
    // find the new active row
    unsigned newActiveRow = m_activeRow;
    int row = m_activeRow;
    for (; row >= 0; row--)
    {
        hh += item_height(uint32_t(row));
        if (hh >= H)
        {
            break;
        }
        newActiveRow = (unsigned) row;
    }

    if (newActiveRow == m_activeRow)
    {
        return false;
    }

    m_top = newActiveRow;

    return activate_row(newActiveRow);
}

int CListView::find_item(int my)
{
    int X {0};
    int Y {0};
    int W {0};
    int H {0};
    bbox(X, Y, W, H);
    int yy = Y;
    unsigned maxl = size();
    for (unsigned l = m_top; l < maxl; l++)
    {
        int hh = item_height(l);
        if (hh <= 0)
        {
            continue;
        }
        yy += hh;
        if (my <= yy || yy >= (Y + H))
        {
            return (int) l;
        }
    }
    return 0;
}

// Tell whether item is currently displayed:

bool CListView::displayed(unsigned index) const
{
    int X {0};
    int Y {0};
    int W {0};
    int H {0};
    bbox(X, Y, W, H);
    int yy = H;
    unsigned maxl = size();
    for (unsigned l = m_top; l < maxl && yy > 0; l++)
    {
        yy -= item_height(l);
        if (l == index)
        {
            return yy >= 0;
        }
    }
    return false;
}

unsigned CListView::fullHeight() const
{
    return m_rows.fullHeight() + m_headerHeight + cellBorderWidth() * 2;
}

unsigned CListView::fullWidth() const
{
    size_t cnt = m_columnList.size();
    size_t ww = 0;
    for (size_t i = 0; i < cnt; i++)
    {
        if (m_columnList[i].visible())
        {
            ww += m_columnList[i].width();
        }
    }
    return (unsigned) ww;
}

bool CListView::select(unsigned index, bool i, int docallbacks)
{
    if (index >= size())
    {
        return false;
    }
    bool sel = item_selected(index);
    if (m_multipleSelection)
    {
        item_select(index, i);
        redraw_line(index);
    }
    else
    {
        if (i == sel)
        {
            return false;
        }

        if (i)
        {
            m_selection.deselectAll();
        }
        item_select(index, i);
        if (i)
        {
            displayRow(index);
        }
        redraw();
    }

    if (docallbacks)
    {
        fireEvent(CEvent::DATA_CHANGED, 0);
    }

    return true;
}

// return where to draw the actual box:

void CListView::bbox(int& X, int& Y, int& W, int& H) const
{
    Fl_Boxtype b = box() ? box() : FL_DOWN_BOX;
    X = x() + Fl::box_dx(b);
    Y = y() + Fl::box_dy(b) + (int) m_headerHeight;
    W = w() - Fl::box_dw(b);
    H = h() - Fl::box_dh(b) - (int) m_headerHeight;
    if (scrollbar.visible())
    {
        W -= m_scrollbarWidth;
    }
    if (hscrollbar.visible())
    {
        H -= m_scrollbarWidth;
    }
    if (H < 0)
    {
        H = 0;
    }
    if (W < 0)
    {
        W = 0;
    }
}

bool CListView::activate_row(unsigned newActiveRow)
{
    if (newActiveRow < size() &&
        m_activeRow != newActiveRow)
    {
        select_only(newActiveRow, true);
        return true;
    }
    return false;
}

CPackedStrings* CListView::selectedRow() const
{
    if (m_selection.size())
    {
        CPackedStrings& ps = m_selection[0];
        return &ps;
    }
    return nullptr;
}

CColumn& CListView::column(const char* colname)
{

    if (int index = m_columnList.indexOf(colname);
        index >= 0)
    {
        return m_columnList[index];
    }
    throw Exception("Column '" + std::string(colname) + "' is not found.");
}

CPackedStrings* CListView::findCaption(const String& caption)
{
    CPackedStrings* row = m_selection.findCaption(caption);
    if (row)
    {
        return row;
    }

    unsigned cnt = m_rows.size();
    for (unsigned i = 0; i < cnt; i++)
    {
        row = m_rows[i];
        if (strcmp((*row)[0], caption.c_str()) == 0)
        {
            return row;
        }
    }

    return nullptr;
}

CPackedStrings* CListView::findKey(int keyValue)
{
    CPackedStrings* row = m_selection.findKey(keyValue);
    if (row)
    {
        return row;
    }

    unsigned cnt = m_rows.size();
    for (unsigned i = 0; i < cnt; i++)
    {
        row = m_rows[i];
        if (row->argument() == keyValue)
        {
            return row;
        }
    }
    return nullptr;
}

void CListView::loadList(const xdoc::SNode& node)
{
    clear();
    auto ntor = node->nodes().begin();

    for (; ntor != node->nodes().end(); ++ntor)
    {
        const auto& anode = *ntor;

        if (anode->name() == "columns")
        {
            m_columnList.load(anode);
            continue;
        }

        if (anode->name() == "rows")
        {
            auto itor = anode->nodes().begin();
            size_t colCount = m_columnList.size();
            if (colCount > 0)
            {
                Strings strings;
                strings.resize(colCount);
                for (; itor != anode->nodes().end(); ++itor)
                {
                    const auto& rowNode = *itor;
                    int rowID = rowNode->attributes().get("id").toInt();
                    auto rtor = rowNode->nodes().begin();
                    unsigned c = 0;
                    for (; rtor != rowNode->nodes().end(); ++rtor, ++c)
                    {
                        const auto& cellNode = *rtor;

                        if (unsigned index = cellNode->attributes().get("index").toInt();
                            index)
                        {
                            c = index;
                        }
                        if (c >= colCount)
                        {
                            break;
                        }
                        strings[c] = cellNode->getText();
                    }
                    addRow(rowID, strings);
                }
            }
        }
    }
}

void CListView::saveList(const xdoc::SNode& node) const
{
    const auto& columnListNode = node->pushNode("columns");
    m_columnList.save(columnListNode);
    const auto& rowsNode = node->pushNode("rows");
    unsigned rowCount = m_rows.size();
    size_t colCount = m_columnList.size();
    for (unsigned i = 0; i < rowCount; i++)
    {
        const auto* row = m_rows[i];
        const auto& rowNode = rowsNode->pushNode("row");
        if (row->argument())
        {
            rowNode->attributes().set("id", to_string(row->argument()));
        }
        size_t index = 0;
        for (size_t c = 0; c < colCount; c++)
        {
            const char* cell = (*row)[(uint16_t) c];
            if (*cell)
            {
                const auto& cellNode = rowNode->pushNode("cell", xdoc::Node::Type::Text);
                cellNode->set(cell);
                if (index != c)
                {
                    cellNode->attributes().set("index", to_string(c));
                    index = c;
                }
                index++;
            }
        }
    }
}

void CListView::load(const xdoc::SNode& node, CLayoutXMLmode xmlMode)
{
    CControl::load(node, xmlMode);
    loadList(node);
}

void CListView::save(const xdoc::SNode& node, CLayoutXMLmode xmlMode) const
{
    CControl::save(node, xmlMode);
    saveList(node);
}

void CListView::imageCollection(Strings& iconNames)
{
    m_iconNames.resize(iconNames.size());
    auto sz = iconNames.size();
    for (size_t i = 0; i < sz; i++)
    {
        m_iconNames[i] = iconNames[i];
    }
}
