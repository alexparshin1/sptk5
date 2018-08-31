/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDateTimeInput.cpp - description                       ║
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
#include <sptk5/gui/CDateTimeInput.h>
#include <sptk5/gui/CButton.h>
#include <sptk5/gui/CDateControl.h>
#include <sptk5/gui/CCalendar.h>

using namespace sptk;

void CDateTimeBaseInput::calendarButtonPressed(Fl_Widget* btn, void*)
{
    auto dateInput = (CDateTimeBaseInput*) btn->parent()->parent();
    if (!dateInput)
        return;
    DateTime dt = dateInput->dateTimeValue();
    dateInput->showCalendar(btn);
    if (dt != dateInput->dateTimeValue())
        dateInput->m_dateInput->do_callback();
}

void CDateTimeBaseInput::ctor_init()
{
    m_dateInput = nullptr;
    m_timeInput = nullptr;
    m_minValue = DateTime();
    m_maxValue = DateTime();
    m_calendarWindow = nullptr;
    m_timeInput = (CInput_*) m_control;
}

CDateTimeBaseInput::CDateTimeBaseInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment, bool autoCreate)
        : CInput(label, layoutSize, layoutAlignment, autoCreate)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CDateTimeBaseInput::CDateTimeBaseInput(int x,int y,int w,int h,const char * label,bool autoCreate)
        : CInput(x,y,w,h,label,autoCreate) {
    ctor_init();
}
#endif

void CDateTimeBaseInput::setLimits(bool limited, DateTime min, DateTime max)
{
    m_limited = limited;
    m_minValue = min;
    m_maxValue = max;
}

void CDateTimeBaseInput::load(Query* loadQuery)
{
    if (!m_fieldName.length())
        return; // no field name - no data loaded
    if (!m_fieldName.length())
        return;
    Field& fld = (*loadQuery)[m_fieldName.c_str()];
    dateTimeValue(fld.asDateTime());
}

void CDateTimeBaseInput::save(Query* updateQuery)
{
    if (!m_fieldName.length())
        return; // no field name - no data saved
    QueryParameter& param = updateQuery->param(m_fieldName.c_str());
    DateTime dt = dateTimeValue();
    param.setDateTime(dt);
}

void CDateTimeBaseInput::load(const xml::Node* node, CLayoutXMLmode xmlMode)
{
    CControl::load(node, xmlMode);
}

void CDateTimeBaseInput::save(xml::Node* node, CLayoutXMLmode xmlMode) const
{
    CControl::save(node, xmlMode);
}

bool CDateTimeBaseInput::valid() const
{
    try {
        DateTime val = dateTimeValue();
        if (m_limited)
            return val >= m_minValue && val <= m_maxValue;
        return true;
    } catch (...) {
    }
    return false;
}

DateTime CDateTimeBaseInput::dateTimeValue() const
{
    return data();
}

void CDateTimeBaseInput::dateTimeValue(DateTime dt)
{
    if (kind() == DCV_DATE)
        data(dt.dateString());
    else
        data(dt.timeString());
}

bool CDateTimeBaseInput::showCalendar(Fl_Widget* btn)
{
    if (!m_calendarWindow)
        return false;
    m_calendarWindow->attachTo(btn->parent());
    m_calendarWindow->date(dateTimeValue());
    bool rc = m_calendarWindow->showModal();
    if (rc) {
        if (m_calendarWindow->date() != dateTimeValue()) {
            dateTimeValue(m_calendarWindow->date());
            do_callback();
        }
    }
    return rc;
}

//===========================================================================
void CDateInput::ctor_init()
{
    begin();
    m_dateInput = new CDateControl(0, 0, 10, 10);
    m_control = m_dateInput->input();
    m_dateInput->callback(CControl::internalCallback);
    m_dateInput->when(FL_WHEN_CHANGED);
    m_dateInput->button()->callback(calendarButtonPressed);
    m_calendarWindow = new CPopupCalendar(m_control);
    end();
    m_control->color(FL_LIGHT3);
}

CDateInput::CDateInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CDateTimeBaseInput(label, layoutSize, layoutAlignment, false)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CDateInput::CDateInput(int x,int y,int w,int h,const char * label)
        : CDateTimeBaseInput(x,y,w,h,label,false) {
    ctor_init();
}
#endif

CDateInput::~CDateInput()
{
    delete m_calendarWindow;
}

CLayoutClient* CDateInput::creator(xml::Node* node)
{
    auto widget = new CDateInput("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

void CDateInput::resize(int xx, int yy, int ww, int hh)
{
    CControl::resize(xx, yy, ww, hh);
    m_dateInput->resize(xx + m_labelWidth, yy, ww - m_labelWidth, hh);
}

Variant CDateInput::data() const
{
    DateTime dt(m_dateInput->input()->value());
    Variant rc;
    rc.setDate(dt);
    return rc;
}

void CDateInput::data(const Variant s)
{
    DateTime dt = s;
    m_dateInput->input()->value(dt.dateString().c_str());
}

void CDateInput::preferredHeight(int& h) const
{
    CDateTimeBaseInput::preferredHeight(h);
    m_dateInput->preferredHeight(h);
}

//===========================================================================
CTimeInput::CTimeInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CDateTimeBaseInput(label, layoutSize, layoutAlignment, true)
{
    m_timeInput = (CInput_*) m_control;
    m_timeInput->mask(DateTime::shortTimeFormat);
}

#ifdef __COMPATIBILITY_MODE__
CTimeInput::CTimeInput(int x,int y,int w,int h,const char * label)
        : CDateTimeBaseInput(x,y,w,h,label,true) {
    m_timeInput = (CInput_ *)m_control;
    m_timeInput->mask(DateTime::shortTimeFormat);
}
#endif

CLayoutClient* CTimeInput::creator(xml::Node* node)
{
    auto widget = new CTimeInput("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

void CTimeInput::preferredWidth(int& w) const
{
    w = int(7 * fl_width('W')) + m_labelWidth + 6;
}

//===========================================================================
void CDateTimeInput::ctor_init()
{
    begin();
    m_dateInput = new CDateControl(0, 0, 10, 10);
    m_dateInput->callback(CControl::internalCallback);
    m_dateInput->when(FL_WHEN_CHANGED);
    m_dateInput->button()->callback(calendarButtonPressed);
    m_dateInput->color(FL_LIGHT3);

    m_control = m_dateInput->input();

    m_timeInput = new CInput_("");
    m_timeInput->callback(CControl::internalCallback);
    m_timeInput->when(FL_WHEN_CHANGED);
    m_timeInput->color(FL_LIGHT3);

    m_calendarWindow = new CPopupCalendar(m_dateInput);
    end();
}

CDateTimeInput::CDateTimeInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CDateTimeBaseInput(label, layoutSize, layoutAlignment, false)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CDateTimeInput::CDateTimeInput(int x,int y,int w,int h,const char * label)
        : CDateTimeBaseInput(x,y,w,h,label,false) {
    ctor_init();
}
#endif

CLayoutClient* CDateTimeInput::creator(xml::Node* node)
{
    auto widget = new CDateTimeInput("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

void CDateTimeInput::resize(int x, int y, int w, int h)
{
    CControl::resize(x, y, w, h);
    w -= m_labelWidth;
    x += m_labelWidth;

    int hh = textSize() + 6;

    if (m_menuButton)
        m_menuButton->resize(x, y, w, hh);

    m_dateInput->resize(x, y, w / 2 - 2, hh);
    x += m_dateInput->w() + 4;

    fl_font(textFont(), textSize());
    int timeWidth = (int) fl_width("00:00AM") + 6;
    m_timeInput->resize(x, y, timeWidth, m_dateInput->h());
}

void CDateTimeInput::dateTimeValue(DateTime dt)
{
    m_dateInput->input()->value(dt.dateString().c_str());
    m_timeInput->value(dt.timeString().c_str());
}

DateTime CDateTimeInput::dateTimeValue() const
{
    DateTime dt = DateTime(m_dateInput->input()->value());
    DateTime::duration tm = DateTime(m_timeInput->value()).sinceEpoch();
    return dt + tm;
}

Variant CDateTimeInput::data() const
{
    return std::string(m_dateInput->input()->value()) + " " + std::string(m_timeInput->value());
}

void CDateTimeInput::data(const Variant s)
{
    dateTimeValue(s.asDateTime());
}
