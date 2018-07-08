/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CCalendar.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CCALENDAR_H__
#define __CCALENDAR_H__

class Fl_Box;
class Fl_Button;

#include <sptk5/gui/CPopupWindow.h>
#include <sptk5/gui/CControl.h>
#include <sptk5/DateTime.h>
#include <sptk5/gui/CLayoutClient.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Calendar widget
 *
 * A calendar class implements one month calendar.
 * The calendar window allows to select the date.
 * You can change a day, a month and a year selected in the widget.
 */
class SP_EXPORT CCalendar : public ::Fl_Group, public CLayoutClient {
    Fl_Group  *m_headerBox;
    Fl_Group  *m_buttonBox;
    Fl_Box    *m_monthNameBox;
    Fl_Box    *m_dayNameBoxes[7];
    Fl_Button *m_dayButtons[31];
    Fl_Button *m_switchButtons[4];
    DateTime  m_date;
    std::string m_headerLabel;
    int        m_activeButtonIndex;
    char       m_weekDayLabels[14];

    /**
     * Day button callback class function
     */
    static void cbDayButtonClicked(Fl_Widget *,void *);

    /**
     * Month button callback class function
     */
    static void cbSwitchButtonClicked(Fl_Widget *,void *);

    /**
     * Constructor initializer
     */
    void ctor_init();

protected:
    /**
     * @brief The reaction function for the clicking on one of the day buttons
     */
    void dayButtonClicked(uint32_t day);

    /**
     * @brief The reaction function for the clicking on one of the month change buttonss
     */
    void switchButtonClicked(int32_t monthChange);

public:

    /**
     * @brief SPTK-style constructor
     * @param label const char *, the widget label
     * @param layoutSize int, the size of widget in layout
     * @param layoutAlignment CLayoutAlign, widget align in the layout
     */
    CCalendar(const char * label,int layoutSize=20,CLayoutAlign layoutAlignment=SP_ALIGN_TOP);

    /**
     * @brief FLTK-style constructor
     * @param x int, widget x-coordinate
     * @param y int, widget y-coordinate
     * @param w int, widget width
     * @param h int, widget height
     * @param label int, optional widget label
     */
    CCalendar(int x,int y,int w,int h,const char *label=0L);

    /**
     * @brief Widget resize
     * @param x int, widget x-coordinate
     * @param y int, widget y-coordinate
     * @param w int, widget width
     * @param h int, widget height
     */
    void resize(int x,int y,int w,int h);

    /**
     * @brief Reports the maximum height of the calendar widget
     */
    int  maxHeight() const;

    /**
     * @brief Resizes widget to the optimal height, considering minHeight
     */
    void autoHeight(int minHeight);

    /**
     * @brief Sets the calendar date
     */
    void      date(DateTime dt);

    /**
     * @brief Reports the calendar date
     */
    DateTime date() const;
};

/**
 * @brief Popup Calendar
 *
 * Creates a popup window with the calendar widget on it.
 * Allows to select day, month, and year with increment/decrement buttons.
 */
class SP_EXPORT CPopupCalendar : public CPopupWindow {
    friend class CCalendar;
    /**
     * The calendar widget
     */
    CCalendar     *m_calendar;

    /**
     * The widget that shows the pop-up calendar
     */
    Fl_Widget     *m_dateControl;

protected:
    /**
     * Internal handle() function for the FLTK widgets
     */
    int  handle(int);

public:
    /**
     * @brief Constructor
     * @param dateControl Fl_Widget, the widget the shows the popup calendar
     */
    explicit CPopupCalendar(Fl_Widget *dateControl);

    /**
     * @brief Attaches the calendar to another widget
     */
    void attachTo(Fl_Widget *dateControl) {
        m_dateControl = dateControl;
    }

    /**
     * @brief Widget resize
     * @param x int, widget x-coordinate
     * @param y int, widget y-coordinate
     * @param w int, widget width
     * @param h int, widget height
     */
    void resize(int x,int y,int w,int h);

    /**
     * @brief Function that is used to report that calendar is clicked on and should be hidden.
     */
    void clicked() {
        m_clicked = 1;
    }

    /**
     * @brief Sets the calendar date
     * @param dt DateTime, new calendar date
     */
    void date(DateTime dt) {
        m_calendar->date(dt);
    }

    /**
     * @brief Reports the calendar date
     * @returns DateTime value of the currently selected date
     */
    DateTime date() const       {
        return m_calendar->date();
    }

    /**
     * @brief Shows the calendar in modal mode
     *
     * The only difference from standard
     * modal mode is that if you click anywhere outside the calendar window and it closes.
     * @returns true if date was selected
     */
    bool showModal();
};
/**
 * @}
 */
}
#endif
