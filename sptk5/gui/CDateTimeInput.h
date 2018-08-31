/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDateTimeInput.h - description                         ║
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

#ifndef __CDATETIMEINPUT_H__
#define __CDATETIMEINPUT_H__

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
 * @brief Base date and time input widget
 *
 * Base class for most of the date and time inputs (except fot CDateIntervalInput)
 */
class SP_EXPORT CDateTimeBaseInput : public CInput
{
    typedef class CInput inherited;

    /**
     * @brief Mininmum date and time value to check in valid()
     */
    DateTime            m_minValue;

    /**
     * @brief Maximum date and time value to check in valid()
     */
    DateTime            m_maxValue;

    /**
     * @brief Constructor initializer
     */
    void ctor_init();

protected:

    /**
     * @brief Popup calendar window
     */
    CPopupCalendar      *m_calendarWindow;

    /**
     * @brief Date input control
     */
    CDateControl        *m_dateInput;

    /**
     * @brief Time input control
     */
    CInput_             *m_timeInput;

    /**
     * @brief Loads data from query
     */
    virtual void load(Query *);

    /**
     * @brief Saves data to query
     */
    virtual void save(Query *);

    /**
     * @brief Loads control data from XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void     load(const xml::Node* node,CLayoutXMLmode xmlMode);

    /**
     * @brief Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void save(xml::Node* node,CLayoutXMLmode xmlMode) const;

    /**
     * @brief Returns true if the input data is valid
     */
    virtual bool valid() const;

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     * @param autoCreate bool, auto create CInput_ widget. Internal.
     */
    CDateTimeBaseInput(const char * label,int layoutSize,CLayoutAlign layoutAlign,bool autoCreate);

    /**
     * @brief Internal callback function
     */
    static void calendarButtonPressed(Fl_Widget *btn,void *data);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CDateTimeBaseInput(int x,int y,int w,int h,const char * label,bool autoCreate);
#endif

    /**
     * @brief Shows drop-down calendar
     */
    bool showCalendar(Fl_Widget *btn);

public:

    /**
     * @brief Sets limits for the value inside
     * @param limited bool, true if use the limits
     * @param min DateTime, minimum value
     * @param max DateTime, maximum value
     */
    void setLimits(bool limited,DateTime min=DateTime(),DateTime max=DateTime());

    /**
     * @brief Returns widget date or time  value
     */
    virtual DateTime dateTimeValue() const;

    /**
     * @brief Sets widget date or time value
     */
    virtual void     dateTimeValue(DateTime dt);
};

class CTimeInput;

/**
 * @brief Date input widget
 *
 * Masked date input with the drop-down calendar
 */
class SP_EXPORT CDateInput : public CDateTimeBaseInput 
{
    /**
     * @brief Constructor initializer
     */
    void ctor_init();
public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CDateInput(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CDateInput(int x,int y,int w,int h,const char * label=0);
#endif

    ~CDateInput();

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    virtual CControlKind kind() const {
        return DCV_DATE;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    virtual String className() const {
        return "date_input";
    }

    /**
     * @brief Universal data connection, returns data from date control.
     * Data is returned as VAR_DATE.
     */
    virtual Variant data() const;

    /**
     * @brief Universal data connection, sets data to date control
     */
    virtual void     data(const Variant v);

    /**
     * @brief Resizes the control and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    virtual void resize(int x,int y,int w,int h);

    /**
     * Computes the optimal widget height
     * @param h int&, input - height offered by the program, output - height required by widget
     */
    virtual void preferredHeight(int& h) const;

    /**
     * @brief Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(xml::Node *node);
};

/**
 * @brief Time input widget
 *
 * Masked time input
 */
class SP_EXPORT CTimeInput : public CDateTimeBaseInput 
{
public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CTimeInput(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CTimeInput(int x,int y,int w,int h,const char * label=0);
#endif

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    virtual CControlKind kind() const {
        return DCV_TIME;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    virtual String className() const {
        return "time_input";
    }

    /**
     * @brief Computes the optimal widget width
     * @param w int&, input - width offered by the program, output - width required by widget
     */
    virtual void preferredWidth(int& w) const;

    /**
     * @brief Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(xml::Node *node);
};

/**
 * @brief Date and time input widget.
 *
 * Combines date and time input widgets. Includes two input boxes and the drop-down calendar
 * for the date input.
 */
class SP_EXPORT CDateTimeInput : public CDateTimeBaseInput {

    /**
     * @brief Constructor initializer
     */
    void ctor_init();

public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CDateTimeInput(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CDateTimeInput(int x,int y,int w,int h,const char * label=0);
#endif

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    virtual CControlKind kind() const {
        return DCV_DATETIME;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    virtual String className() const {
        return "date_time_input";
    }

    /**
     * @brief Resizes the control and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    virtual void resize(int x,int y,int w,int h);

    /**
     * @brief Returns widget date or time  value
     */
    virtual DateTime dateTimeValue() const;

    /**
     * @brief Sets widget date or time value
     */
    virtual void     dateTimeValue(DateTime dt);

    /**
     * @brief Universal data connection, returns data from control
     */
    virtual Variant data() const;

    /**
     * @brief Universal data connection, sets data from control
     */
    virtual void     data(const Variant v);

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
