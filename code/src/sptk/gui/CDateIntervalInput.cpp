/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDateIntervalInput.cpp - description                   ║
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

#include <FL/fl_draw.H>

#include <sptk5/gui/CDateIntervalInput.h>
#include <sptk5/gui/CButton.h>
#include <sptk5/gui/CCalendar.h>
#include <sptk5/gui/CDateControl.h>

using namespace std;
using namespace sptk;

void CDateIntervalInput::intervalCalendarButtonPressed(Fl_Widget* btn, void*)
{
    auto intervalInput = (CDateIntervalInput*) btn->parent()->parent();
    intervalInput->showCalendar(btn);
}

void CDateIntervalInput::ctor_init()
{
    m_calendarWindow = new CPopupCalendar(nullptr);
    m_separator = "..";

    begin();

    m_firstDateInput = new CDateControl(0, 0, 10, 10);
    m_firstDateInput->callback(CControl::internalCallback);
    m_firstDateInput->when(FL_WHEN_CHANGED);
    m_firstDateInput->button()->callback(intervalCalendarButtonPressed);

    m_control = m_firstDateInput->input();

    m_secondDateInput = new CDateControl(0, 0, 10, 10, "to");
    m_secondDateInput->align(FL_ALIGN_LEFT);
    m_secondDateInput->callback(CControl::internalCallback);
    m_secondDateInput->when(FL_WHEN_CHANGED);
    m_secondDateInput->button()->callback(intervalCalendarButtonPressed);

    end();
}

CDateIntervalInput::CDateIntervalInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CInput(label, layoutSize, layoutAlignment, false)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CDateIntervalInput::CDateIntervalInput(int x,int y,int w,int h,const char * label)
        : CInput(x,y,w,h,label,false) {
    ctor_init();
}
#endif

CLayoutClient* CDateIntervalInput::creator(xml::Node* node)
{
    auto widget = new CDateIntervalInput("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

void CDateIntervalInput::showCalendar(Fl_Widget* btn)
{
    DateTime originalDate;
    CDateControl* dateControl;
    if (btn == m_firstDateInput->button()) {
        originalDate = beginOfInterval();
        dateControl = m_firstDateInput;
    } else {
        originalDate = endOfInterval();
        dateControl = m_secondDateInput;
    }
    m_calendarWindow->attachTo(dateControl);
    m_calendarWindow->date(originalDate);
    bool rc = m_calendarWindow->showModal();
    if (rc && m_calendarWindow->date() != originalDate) {
        if (btn == m_firstDateInput->button()) {
            beginOfInterval(m_calendarWindow->date());
        } else {
            endOfInterval(m_calendarWindow->date());
        }
        dateControl->do_callback();
    }
}

void CDateIntervalInput::separator(const string& s)
{
    m_separator = s;
}

void CDateIntervalInput::resize(int x, int y, int w, int h)
{
    int twidth = 16;
    fl_font(textFont(), textSize());
    int editorWidth = (int) fl_width("00/00/00000") + h;
    int maxWidth = 2 * editorWidth + twidth + m_labelWidth;

    if (w > maxWidth)
        w = maxWidth;

    CControl::resize(x, y, w, h);
    h = this->h();

    w -= m_labelWidth;
    x += m_labelWidth;
    if (m_menuButton)
        m_menuButton->resize(x, y, w, h);

    m_firstDateInput->resize(x, y, editorWidth, h);
    x += editorWidth + twidth;
    m_secondDateInput->resize(x, y, editorWidth, h);
}

Variant CDateIntervalInput::data() const
{
    DateTime dt1 = beginOfInterval();
    DateTime dt2 = endOfInterval();

    if (dt1.zero() && dt2.zero())
        return "";

    std::string result = m_firstDateInput->input()->value();
    result += m_separator;
    result += m_secondDateInput->input()->value();
    return result;
}

void CDateIntervalInput::data(const Variant s)
{
    string firstStringData(s.asString().c_str());
    string secondStringData;

    size_t pos = firstStringData.find(m_separator);
    size_t separatorLength = m_separator.length();

    if (!separatorLength)
        throw Exception("Can't work with an empty dates separator");

    if (pos != STRING_NPOS && pos > 0) {
        firstStringData[pos] = 0;
        secondStringData = firstStringData.substr(pos + separatorLength, 20);
    }
    m_firstDateInput->input()->value(firstStringData.c_str());
    m_secondDateInput->input()->value(secondStringData.c_str());
}

bool CDateIntervalInput::valid() const
{
    DateTime dt1 = beginOfInterval();
    DateTime dt2 = endOfInterval();

    return !(!dt1.zero() && !dt2.zero() && dt1 > dt2);
}

DateTime CDateIntervalInput::beginOfInterval() const
{
    return DateTime(m_firstDateInput->input()->value());
}

void CDateIntervalInput::beginOfInterval(const DateTime bi)
{
    m_firstDateInput->input()->value(bi.dateString().c_str());
}

DateTime CDateIntervalInput::endOfInterval() const
{
    return DateTime(m_secondDateInput->input()->value());
}

void CDateIntervalInput::endOfInterval(const DateTime ei)
{
    m_secondDateInput->input()->value(ei.dateString().c_str());
}
