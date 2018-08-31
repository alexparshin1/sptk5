/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDateIntervalInput.h - description                     ║
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

#ifndef __CDATEINTERVALINPUT_H__
#define __CDATEINTERVALINPUT_H__

#include <sptk5/sptk.h>

#include <sptk5/DateTime.h>
#include <sptk5/gui/CInput.h>
#include <sptk5/gui/CControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

class CPopupCalendar;
class CDateControl;

/**
 * @brief Date interval input widget
 *
 * Masked date interval input with the drop-down calendar
 */
class SP_EXPORT CDateIntervalInput : public CInput {
    typedef class CInput inherited;

    /**
     * First date input widget
     */
    CDateControl   *m_firstDateInput;

    /**
     * Second date input widget
     */
    CDateControl   *m_secondDateInput;

    /**
     * Separator to display
     */
    std::string     m_separator;

    /**
     * Pop-up calendar window
     */
    CPopupCalendar *m_calendarWindow;


    /**
     * @brief Constructor initializer
     */
    void ctor_init();

    /**
     * @brief Internal callback function
     */
    static void intervalCalendarButtonPressed(Fl_Widget *btn,void *data);

protected:

    /**
     * @brief Shows drop-down calendar for the particular date input that owns the button btn
     * @param btn Fl_Widget *, button of the date input widget
     */
    void showCalendar(Fl_Widget *btn);

public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CDateIntervalInput(const char *label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CDateIntervalInput(int x,int y,int w,int h,const char *label=0);
#endif

    /**
     * @brief Sets the separator to display between date inputs
     * @param s std::string, separator value
     */
    void separator(const std::string& s);

    /**
     * @brief Returns the separator between date inputs
     */
    std::string separator() {
        return m_separator;
    }

    /**
     * @brief Returns the date from the first date input - the beginning of interval
     */
    DateTime beginOfInterval() const;

    /**
     * @brief Sets the date in the first date input - the beginning of interval
     * @param dt const DateTime, date and time value for the first control
     */
    void      beginOfInterval(const DateTime dt);

    /**
     * @brief Returns the date from the second date input - the end of interval
     */
    DateTime endOfInterval() const;

    /**
     * Sets the date from the second date input - the end of interval
     * @param dt const DateTime, date and time value for the second control
     */
    void      endOfInterval(const DateTime dt);

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    virtual CControlKind kind() const {
        return DCV_DATEINTERVAL;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    virtual String className() const {
        return "date_interval_input";
    }

    /**
     * @brief Universal data connection, returns data from control
     */
    virtual Variant data() const;

    /**
     * @brief Universal data connection, sets data from control
     */
    virtual void     data(const Variant v);

    /**
     * @brief Resizes the control and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    virtual void     resize(int x,int y,int w,int h);

    /**
     * @brief Returns true if the control state is valid, and the data is inside the limits
     * if applicable
     */
    virtual bool     valid() const;

    /**
     * @brief Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(xml::Node *node);
};
/**
 * @}
 */
}
#endif
