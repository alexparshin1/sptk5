/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTabs.cpp - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/sptk.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>

#include <sptk5/gui/CScroll.h>
#include <sptk5/gui/CTabs.h>
#include <sptk5/gui/CThemes.h>

using namespace std;

namespace sptk {

class CTabButtons;

class CTabButton : public Fl_Button
{
    Fl_Group* m_page;
    CTabButtons* m_ownerRow;
public:
    explicit CTabButton(Fl_Group* page)
            : Fl_Button(0, 0, 10, 10, "")
    {
        m_page = page;
        m_ownerRow = nullptr;
    }

    ~CTabButton() override = default;

    virtual bool preferredSize(int& w, int& h);

    void ownerRow(CTabButtons* ownerRow)
    {
        m_ownerRow = ownerRow;
    }

    CTabButtons* ownerRow()
    {
        return m_ownerRow;
    }

    bool selected() const;

    Fl_Group* page()
    {
        return m_page;
    }

    void draw() override;

    int handle(int event) override;
};

class CTabButtons : public vector<CTabButton*>
{
    int m_maxHeight;
public:
    CTabButtons()
    {
        m_maxHeight = 0;
    }

    virtual ~CTabButtons() = default;

    int height() const
    {
        return m_maxHeight;
    }

    void height(int h)
    {
        if (m_maxHeight < h)
            m_maxHeight = h;
    }

    virtual void clear()
    {
        m_maxHeight = 0;
        vector<CTabButton*>::clear();
    }
};

class CTabRows : public std::vector<CTabButtons*>
{
public:
    CTabRows() = default;

    virtual ~CTabRows()
    {
        clear();
    }

    virtual void clear()
    {
        for (auto button: *this)
            delete button;
        vector<CTabButtons*>::clear();
    }
};

class CTabGroup : public CGroup
{
    friend class CTabButton;
    friend class CTabs;

    CTabButtons m_buttons;
protected:
    CTabButton* m_activeTabButton;
    CTabRows m_rows;

    static void cb_tabButton(Fl_Widget* btn, void* data);

    void activate(CTabButton* button);

public:
    CTabGroup()
    : CGroup("", 16, SP_ALIGN_TOP)
    {
        m_activeTabButton = nullptr;
        layoutSpacing(0);
        box(FL_FLAT_BOX);
    }

    void add(Fl_Group* group);

    void remove(Fl_Group* group);

    virtual bool preferredSize(int x, int y, int& width, int& height, bool buildRows);

    bool preferredSize(int& width, int& height) override;

    void resize(int xx, int yy, int width, int height) override;

    void draw() override;

    /// @brief Returns currently selected tab
    Fl_Group* page()
    {
        if (!m_activeTabButton)
            return nullptr;
        return m_activeTabButton->page();
    }

    /// @brief Returns selected page number
    int pageNumber() const
    {
        auto itor = std::find(m_buttons.begin(), m_buttons.end(), m_activeTabButton);
        if (itor == m_buttons.end())
            return -1;
        return (int) distance(m_buttons.begin(), itor);
    }

    /// @brief Sets selected page by number
    ///
    /// @param pagNumber unsigned, a page number to select
    void pageNumber(unsigned pgNumber)
    {
        if (pgNumber >= m_buttons.size())
            return;
        activate(m_buttons[pgNumber]);
    }
};

}

using namespace sptk;

int CTabButton::handle(int event)
{
    auto tabGroup = (CTabGroup*) parent();
    switch (event) {
        case FL_FOCUS:
            tabGroup->activate(this);
            break;
        case FL_KEYBOARD:
            if (Fl::event_key() == FL_Down) {
                redraw();
                return m_page->handle(FL_FOCUS);
            }
        default:
            break;
    }
    return Fl_Button::handle(event);
}

bool CTabButton::selected() const
{
    auto tabGroup = dynamic_cast<CTabGroup*>(parent());
    if (!tabGroup)
        return false;
    return tabGroup->m_activeTabButton == this;
}

bool CTabButton::preferredSize(int& w, int& h)
{
    w = 0;
    h = 0;

    if (m_page != nullptr) {
        fl_font(m_page->labelfont(), m_page->labelsize());
        int frame_dw = Fl::box_dw(parent()->box());
        int frame_dh = Fl::box_dh(parent()->box());
        fl_measure(m_page->label(), w, h);
        w += frame_dw + 10;
        h += frame_dh + 6;
    }

    return true;
}

void CTabButton::draw()
{
    auto tabsWidget = dynamic_cast<CTabGroup*>(parent());
    if (!tabsWidget)
        return;
    int hh = parent()->h() - (y() - parent()->y());
    bool selected = tabsWidget->m_activeTabButton == this;

    Fl_Align labelAlign = FL_ALIGN_CENTER;
    int labelLeft = 0;
    int wt = 0, ht = 0;

    m_page->measure_label(wt, ht);

    // Draw the background of the tab
    CThemes::drawTab(x(), y(), w(), hh, selected);

    labelLeft = x() + 4 + (w() - 6 - wt) / 2;
    m_page->draw_label(labelLeft, y() + 2, wt, ht + 2, labelAlign);

    if (Fl::focus() == this && selected)
        draw_focus(box(), labelLeft - 3, y() + 2, wt + 6, ht + 4);
}

void CTabGroup::draw()
{
    draw_box();
    auto itor = m_rows.begin();
    for (; itor != m_rows.end(); ++itor) {
        CTabButtons* row = *itor;
        auto btor = row->begin();
        for (; btor != row->end(); btor++) {
            CTabButton* button = *btor;
            button->draw();
        }
    }
}

void CTabGroup::add(Fl_Group* group)
{
    Fl_Group* saveParent = Fl_Group::current();
    begin();
    auto button = new CTabButton(group);
    button->callback(cb_tabButton, this);
    m_buttons.push_back(button);
    end();
    if (!m_activeTabButton)
        m_activeTabButton = button;
    Fl_Group::current(saveParent);
}

void CTabGroup::remove(Fl_Group* group)
{
    auto itor = m_buttons.begin();
    for (; itor != m_buttons.end(); ++itor) {
        CTabButton* button = *itor;
        if (button->page() == group) {
            if (button == m_activeTabButton)
                m_activeTabButton = nullptr;
            m_buttons.erase(itor);
            Fl_Group::remove
                    (button);
            delete button;
            break;
        }
    }
}

void CTabGroup::activate(CTabButton* button)
{
    if (m_activeTabButton == button)
        return;
    m_activeTabButton->page()->hide();
    m_activeTabButton = button;
    m_activeTabButton->page()->show();
    auto tabControl = (CGroup*) parent();
    tabControl->relayout();
    redraw();
}

void CTabGroup::cb_tabButton(Fl_Widget* btn, void* data)
{
    auto button = dynamic_cast<CTabButton*>(btn);
    if (!button)
        return;
    auto group = (CTabGroup*) data;
    group->activate(button);
}

void CTabGroup::resize(int xx, int yy, int width, int height)
{
    CGroup::resize(xx, yy, width, height);
    preferredSize(xx, yy, width, height, true);
}

bool CTabGroup::preferredSize(int& width, int& height)
{
    return preferredSize(0, 0, width, height, false);
}

bool CTabGroup::preferredSize(int, int, int& width, int& height, bool buildRows)
{
    unsigned offset = 0;
    auto buttonCount = (unsigned) children();
    int maxWidth = 0;

    CTabButtons* row = nullptr;
    if (buildRows) {
        m_rows.clear();
        if (buttonCount) {
            row = new CTabButtons;
            m_rows.push_back(row);
        }
    }

    height = 0;
    int rowHeight = 0;
    auto itor = m_buttons.begin();
    auto iend = m_buttons.end();
    for (; itor != iend; ++itor) {
        CTabButton* button = *itor;
        if (!button)
            continue;
        int bw = 0, bh = 0;
        button->preferredSize(bw, bh);
        unsigned newOffset = offset + bw + 2;
        if (newOffset <= (unsigned) width) {
            if (newOffset > (unsigned) maxWidth)
                maxWidth = newOffset;
        } else {
            if (buildRows) {
                row = new CTabButtons;
                m_rows.push_back(row);
            }
            height += rowHeight;
            rowHeight = 0;
            //offset = 0;
            newOffset = (unsigned) bw;
        }
        if (rowHeight < bh)
            rowHeight = bh;
        if (buildRows && row) {
            row->push_back(button);
            button->ownerRow(row);
            button->size(bw, bh);
            button->box(FL_THIN_UP_BOX);
            row->height(bh);
        }
        offset = newOffset;
    }
    height += rowHeight;
    if (width < maxWidth)
        width = maxWidth;
    if (buildRows) {
        if (m_activeTabButton) {
            CTabButtons* activeRow = m_activeTabButton->ownerRow();
            auto activeRowPos = std::find(m_rows.begin(), m_rows.end(), activeRow);
            if (activeRowPos != m_rows.end()) {
                m_rows.erase(activeRowPos);
                m_rows.push_back(activeRow);
            }
        }
        auto rtor = m_rows.begin();
        auto rend = m_rows.end();
        auto yPosition = (unsigned) y();
        for (; rtor != rend; rtor++) {
            CTabButtons* brow = *rtor;
            auto btor = brow->begin();
            auto bend = brow->end();
            auto xPosition = (unsigned) x();
            for (; btor != bend; btor++) {
                CTabButton* button = *btor;
                button->position(xPosition, yPosition);
                xPosition += button->w() + 2;
            }
            yPosition += brow->height();
        }
    }
    return false;
}

const Fl_Color CTabs::AutoColorTable[16] = {
        fl_rgb_color(0xB0, 0xD0, 0xD0),
        fl_rgb_color(0xC0, 0xC0, 0xE0),
        fl_rgb_color(192, 176, 160),
        fl_rgb_color(0xD0, 0xD0, 0xB0),
        fl_rgb_color(240, 190, 190),
        fl_rgb_color(0xC0, 0xB0, 0xC0),
        fl_rgb_color(0xC0, 0xA0, 0x90),
        fl_rgb_color(0xD0, 0xD0, 0xE8),
        fl_rgb_color(0xE8, 0xC0, 0xC0),
        fl_rgb_color(0xC0, 0xE8, 0xC0),
        fl_rgb_color(0xE8, 0xC0, 0xE8),
        fl_rgb_color(0xE0, 0xE0, 0xC0),
        fl_rgb_color(0xC0, 0xE0, 0xE0),
        fl_rgb_color(0xE0, 0xC0, 0xE0),
        fl_rgb_color(0xA0, 0xB8, 0xA0),
        fl_rgb_color(0xB8, 0xC0, 0xE8)
};

CTabs::CTabs(const char* label, int layoutSize, CLayoutAlign layoutAlign)
        : CGroup(label, layoutSize, layoutAlign), m_autoColorIndex(0)
{
    box(FL_THIN_UP_BOX);
    layoutSpacing(0);
    m_tabs = new CTabGroup;
}

#ifdef __COMPATIBILITY_MODE__
CTabs::CTabs(int x, int y, int w, int h, const char *label)
        : CGroup(x, y, w, h, label), m_autoColorIndex(0)
{
    box(FL_UP_BOX);
    layoutSpacing(0);
    m_tabs = new CTabGroup;
}
#endif

CTabs::~CTabs()
{
    delete m_tabs;
}

CLayoutClient* CTabs::creator(xml::Node* node)
{
    auto widget = new CTabs("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

void CTabs::showTabs(bool show)
{
    if (show)
        m_tabs->show();
    else
        m_tabs->hide();
}

void CTabs::draw()
{
    //int dy = Fl::box_dy(box());
    if (m_tabs->visible()) {
        m_tabs->draw();
        fl_draw_box(box(), x(), y() + m_tabs->h(), w(), h() - m_tabs->h(), color());
    } else
        fl_draw_box(box(), x(), y(), w(), h(), color());
    for (int i = 1; i < children(); i++) {
        Fl_Widget* page = child(i);
        if (page->visible())
            page->draw();
    }
    if (m_tabs->visible() && m_tabs->m_activeTabButton)
        m_tabs->m_activeTabButton->draw();
}

void CTabs::removeEmptyLastPage()
{
    int i = children() - 1;
    if (i < 1)
        return;
    auto agroup = (Fl_Group*) child(i);
    if (agroup) {
        if (!agroup->children())
            remove(agroup);
        else
            agroup->box(FL_FLAT_BOX);
    }
}

void CTabs::prepareNewPage(Fl_Group* page, bool autoColor)
{
    page->box(FL_FLAT_BOX);
    if (pageCount())
        page->hide();
    if (autoColor) {
        Fl_Color clr = AutoColorTable[m_autoColorIndex & 0xF];
        page->color(clr);
        m_autoColorIndex++;
    }
    m_tabs->add(page);
}

Fl_Group* CTabs::newScroll(const char* label, bool autoColor)
{
    removeEmptyLastPage();

    begin();
    auto scroll = new CScroll(label, 10, SP_ALIGN_CLIENT);
    scroll->scrollbar.visible_focus(false);
    scroll->hscrollbar.visible_focus(false);

    prepareNewPage(scroll, autoColor);
    return scroll;
}

Fl_Group* CTabs::newPage(const char* label, bool autoColor)
{
    removeEmptyLastPage();

    begin();
    auto group = new CGroup(label, 10, SP_ALIGN_CLIENT);

    prepareNewPage(group, autoColor);
    return group;
}

uint32_t CTabs::pageCount() const
{
    return (uint32_t) m_tabs->m_buttons.size();
}

uint32_t CTabs::pageNumber() const
{
    return (unsigned) m_tabs->pageNumber();
}

void CTabs::pageNumber(uint32_t pgNumber)
{
    m_tabs->pageNumber(pgNumber);
}

void CTabs::remove
        (Fl_Group* page)
{
    m_tabs->remove
                  (page);
    Fl_Group::remove
            (page);
    delete page;
}
