/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CCalendar.cpp - description                            ║
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

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

#include <sptk5/gui/CCalendar.h>

using namespace std;
using namespace sptk;

static const char* monthDayLabels[31] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
        "21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
        "31"
};

static const char* switchLabels[4] = {
        "@<<", "@<", "@>", "@>>"
};

static const long monthChanges[4] = {
        -12, -1, 1, 12
};

// Callback function for day buttons
void CCalendar::cbDayButtonClicked(Fl_Widget* button, void* param)
{
    Fl_Group* buttonBox = button->parent();
    auto calendar = dynamic_cast<CCalendar*>(buttonBox->parent());
    if (!calendar) return;
    calendar->dayButtonClicked((uint32_t)(unsigned long) param);
}

// Callback function for switch buttons
void CCalendar::cbSwitchButtonClicked(Fl_Widget* button, void* param)
{
    auto calendar = dynamic_cast<CCalendar*>(button->parent());
    if (!calendar) return;
    calendar->switchButtonClicked((uint32_t)(long) param);
}

void CCalendar::dayButtonClicked(uint32_t day)
{
    if (day < 1 || day > 31) return;
    if (m_activeButtonIndex > -1) {
        Fl_Button* btn = m_dayButtons[m_activeButtonIndex];
        btn->box(FL_THIN_UP_BOX);
        btn->color(fl_lighter(color()));
    }
    m_activeButtonIndex = day - 1;
    Fl_Button* btn = m_dayButtons[m_activeButtonIndex];
    btn->box(FL_FLAT_BOX);
    btn->color(color());
    redraw();

    do_callback();

    // Check if this calendar is on a popup-window
    auto w = dynamic_cast<CPopupCalendar*>(btn->window());
    if (w) {
        w->clicked();
        w->hide();
    }
}

void CCalendar::switchButtonClicked(int32_t monthChange)
{
    short year, month, day, wday, yday;
    m_date.decodeDate(&year, &month, &day, &wday, &yday);
    month += (short) monthChange;
    if (month < 1) {
        month += 12;
        year--;
    }
    if (month > 12) {
        month -= 12;
        year++;
    }
    while (day > 0) {
        try {
            DateTime newDate(year, month, day);
            date(newDate);
            break;
        }
        catch (...) {
            day--;
        }
    }

    do_callback();
}

void CCalendar::ctor_init()
{
    int xx = x();
    int yy = y();
    int ww = w();
    m_activeButtonIndex = -1;

    box(FL_FLAT_BOX);

    // Header box
    m_headerBox = new Fl_Group(xx, yy, ww, 32);
    m_monthNameBox = new Fl_Box(xx, yy, ww - 64, 16);
    m_monthNameBox->box(FL_FLAT_BOX);
    m_monthNameBox->labelfont(1);

    // Weekday headers
    unsigned i;
    for (i = 0; i < 7; i++) {
        char* label = m_weekDayLabels + (i * 2);
        label[0] = DateTime::weekDayNames[i].c_str()[0];
        label[1] = 0;
        m_dayNameBoxes[i] = new Fl_Box(xx + i * 16, yy + 16, 16, 16, label);
        m_dayNameBoxes[i]->box(FL_THIN_UP_BOX);
        if (i == 0 || i == 6)
            m_dayNameBoxes[i]->labelcolor(FL_RED);
    }
    m_headerBox->end();

    // Day buttons, correct positions are set by resize()
    m_buttonBox = new Fl_Group(xx, yy + 32, ww, 64);
    m_buttonBox->box(FL_FLAT_BOX);
    m_buttonBox->color(fl_darker(FL_GRAY));
    for (i = 0; i < 31; i++) {
        auto btn = new Fl_Button(xx, yy, 16, 16, monthDayLabels[i]);
        m_dayButtons[i] = btn;
        btn->callback(cbDayButtonClicked, (void*)long(i + 1));
        btn->color(fl_lighter(fl_lighter(color())));
        btn->box(FL_THIN_UP_BOX);
    }
    m_buttonBox->end();

    // Switch buttons, correct positions are set by resize()
    for (i = 0; i < 4; i++) {
        m_switchButtons[i] = new Fl_Button(xx, yy, 16, 16, switchLabels[i]);
        m_switchButtons[i]->box(FL_THIN_UP_BOX);
        m_switchButtons[i]->labelcolor(fl_darker(FL_GREEN));
        m_switchButtons[i]->callback(cbSwitchButtonClicked, (void*) monthChanges[i]);
    }

    end();
    m_date = DateTime::Now();
}

CCalendar::CCalendar(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : Fl_Group(0, 0, 10, 10, label), CLayoutClient(this, layoutSize, layoutAlignment)
{
    ctor_init();
}

CCalendar::CCalendar(int x, int y, int w, int h, const char* lbl)
        : Fl_Group(x, y, w, h, lbl), CLayoutClient(this, w, SP_ALIGN_NONE)
{
    ctor_init();
}

void CCalendar::resize(int xx, int yy, int ww, int hh)
{
    int bh = 18;
    int bw = ww / 7;
    ww = bw * 7;
    hh = hh / bh * bh;

    Fl_Group::resize(xx, yy, ww, hh);

    // resize header
    m_headerBox->resize(xx, yy, ww, bh * 2);
    m_monthNameBox->resize(xx, yy, ww, bh);

    unsigned i;
    for (i = 0; i < 7; i++)
        m_dayNameBoxes[i]->resize(xx + i * bw, yy + bh, bw, bh);

    // compute the month start date
    short year, month, day, wday, yday;
    if (m_date.zero())
        m_date = DateTime::Now();
    m_date.decodeDate(&year, &month, &day, &wday, &yday);
    DateTime monthDate(year, month, 1);
    m_headerLabel = monthDate.monthName() + ", " + int2string(year);
    m_monthNameBox->label(m_headerLabel.c_str());

    // resize day buttons
    m_buttonBox->resize(xx, yy + bh * 2, ww, hh - bh * 2);
    int dayOffset = monthDate.dayOfWeek() - 1;
    int daysInMonth = monthDate.daysInMonth();
    int weekOffset = bh * 2;
    for (i = 0; i < 31; i++) {
        Fl_Button* btn = m_dayButtons[i];
        btn->resize(xx + dayOffset * bw, yy + weekOffset, bw, bh);
        dayOffset++;
        if ((int) i < daysInMonth) {
            if (dayOffset > 6) {
                dayOffset = 0;
                weekOffset += bh;
            }
            btn->show();
        } else btn->hide();
    }

    // last visible button
    Fl_Button* btn = m_dayButtons[daysInMonth - 1];
    int sby = btn->y() + bh;

    bw = bw * 3 / 2;
    for (i = 0; i < 2; i++)
        m_switchButtons[i]->resize(xx + i * bw, sby, bw, bh);

    int x1 = xx + ww - bw * 2;
    for (i = 2; i < 4; i++)
        m_switchButtons[i]->resize(x1 + (i - 2) * bw, sby, bw, bh);

    // Check if this calendar is on a popup-window
    btn = m_switchButtons[3];
    auto pcw = dynamic_cast<CPopupCalendar*>(btn->window());
    if (pcw) {
        int requiredWidth = ww + 4;
        int requiredHeight = sby + bh + 3 - yy;
        if (pcw->h() != requiredHeight || pcw->w() != requiredWidth)
            pcw->size(requiredWidth, requiredHeight);
    }
}

void CCalendar::date(DateTime dt)
{
    m_date = dt;
    resize(x(), y(), w(), h());
    redraw();
}

DateTime CCalendar::date() const
{
    short year, month, day, wday, yday;
    m_date.decodeDate(&year, &month, &day, &wday, &yday);
    if (m_activeButtonIndex > -1)
        day = short(m_activeButtonIndex + 1);
    return DateTime(year, month, day);
}

int CCalendar::maxHeight() const
{
    return 154;
}

void CCalendar::autoHeight(int minHeight)
{
    int height = maxHeight();
    if (height < minHeight) height = minHeight;
    resize(x(), y(), w(), height);
}

//------------------------------------------------------------------------------------------------------
CPopupCalendar::CPopupCalendar(Fl_Widget* dateControl)
        : CPopupWindow(100, 140, "Calendar")
{
    box(FL_UP_BOX);
    m_dateControl = dateControl;
    m_calendar = new CCalendar(0, 0, 100, 100);
    add(m_calendar);
}

void CPopupCalendar::resize(int xx, int yy, int ww, int hh)
{
    CPopupWindow::resize(xx, yy, ww, hh);
    m_calendar->resize(2, 2, ww - 4, hh - 4);
}

bool CPopupCalendar::showModal()
{
    bool rc = false;
    if (m_dateControl) {
        int width = 175;
        resize(m_dateControl->x() + m_dateControl->w() - width + m_dateControl->window()->x(),
               m_dateControl->y() + m_dateControl->window()->y() + m_dateControl->h() - 1,
               width,
               140);
        rc = CPopupWindow::showModal();
    }
    return rc;
}

int CPopupCalendar::handle(int event)
{
    int rc = CPopupWindow::handle(event);
    if (rc) return rc;

    if (event != FL_DRAG)
        return m_calendar->handle(event);
    else return Fl_Widget::handle(event);
}
