/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CControl.h - description                               ║
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

#ifndef __CCONTROL_H__
#define __CCONTROL_H__

#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Button.H>

#include <sptk5/db/Query.h>
#include <sptk5/gui/CEvent.h>
#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/cxml>

#include <string>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Control kind is the constant to report the internal SPTK RTTI.
 */
enum CControlKind
{
    /**
     * Control kind is unknown
     */
    DCV_UNKNOWN,

    /**
     * Control is a box of plain text
     */
    DCV_BOX=1,

    /**
     * Control is a box of HTML text
     */
    DCV_HTMLBOX=2,

    /**
     * Control is a text input (single line)
     */
    DCV_STRING=4,

    /**
     * Control is a text input (multiple lines)
     */
    DCV_MEMO=8,

    /**
     * Control is an integer value input
     */
    DCV_INTEGER=0x10,

    /**
     * Control is an integer value input
     */
    DCV_FLOAT=0x20,

    /**
     * Control is a date input
     */
    DCV_DATE=0x40,

    /**
     * Control is a time input
     */
    DCV_TIME=0x80,

    /**
     * Control is a date and time input
     */
    DCV_DATETIME=0x100,

    /**
     * Control is a date interval input
     */
    DCV_DATEINTERVAL=0x200,

    /**
     * Control is a combobox
     */
    DCV_COMBO=0x400,

    /**
     * Control is a combobox with int key value
     */
    DCV_INTVALUECOMBO=0x800,

    /**
     * Control is a listbox
     */
    DCV_LISTBOX=0x1000,

    /**
     * Control is a set of check buttons
     */
    DCV_CHECKBUTTONS=0x2000,

    /**
     * Control is a set of radio buttons
     */
    DCV_RADIOBUTTONS=0x4000,

    /**
     * Control is a phone number input
     */
    DCV_PHONE=0x8000,

    /**
     * Control is a tree widget
     */
    DCV_TREEVIEW=0x10000,

    /**
     * Control is a group
     */
    DCV_GROUP=0x20000,

    /**
     * Control is a value slider
     */
    DCV_SLIDER=0x40000,

    /**
     * Control is a progress bar
     */
    DCV_PROGRESS=0x80000

};

/**
 * @brief Special control flags (used as bit combination)
 */
enum FGENTRYFLAGS
{
    /**
     * No flags
     */
    FGE_NONE,

    /**
     * Requires spell checking
     */
    FGE_SPELLCHECK=1,

    /**
     * Must have some value
     */
    FGE_MANDATORY=2,

    /**
     * Is a custom field
     */
    FGE_CFIELD=4,

    /**
     * Is a single-line entry
     */
    FGE_SINGLELINEENTRY=256,

    /**
     * Is a multi-line entry
     */
    FGE_MULTILINEENTRY=512

};

/**
 * @brief Default height for the single-row edit control
 */
#define SINGLE_EDIT_ENTRY_HEIGHT 23

class CDBDropDownList;
class CPackedStrings;
class CColumnList;
class Strings;

/**
 * @brief Internal field name validation
 */
SP_EXPORT bool checkFieldName(const String& fldName);

/**
 * @brief Base class for most of SPTK widgets.
 *
 * It is a group used as a container for the widget(s) inside.
 * Every CControl has a field name, and it can be a layout client.
 */
class SP_EXPORT CControl : public ::Fl_Group, public CLayoutClient
{
    friend class CInternalComboBoxPanel;
    friend class CEditor;
    friend class CBaseButton;
    friend class CRadioButton;
    friend class CCheckButton;
    friend class CInput_;

    /**
     * Constructor initializer
     */
    void ctor_init(const char *);

protected:

    /**
     * Control has the defined min and max values
     */
    bool              m_limited;

    /**
     * Control label width (on the left)
     */
    uint32_t          m_labelWidth;

    /**
     * Control label color
     */
    Fl_Color          m_labelColor;

    /**
     * Control text color
     */
    Fl_Color          m_textColor;

    /**
     * Control text font
     */
    Fl_Font           m_textFont;

    /**
     * Control text font size
     */
    uchar             m_textSize;

    /**
     * Field name for the universal data connection
     */
    String            m_fieldName;

    /**
     * The text of the hint
     */
    String            m_hint;

    /**
     * The main widget inside the control.
     */
    Fl_Widget*        m_control;

    /**
     * Control's special data tag
     */
    int               m_tag;

    /**
     * @brief Control's special flags (a bit combination of FGENTRYFLAGS).
     * @see FGENTRYFLAGS for details.
     */
    uint32_t          m_controlFlags;
    /**
     * Control's menu button - to show right-mouse click menu
     */
    Fl_Menu_Button   *m_menuButton;

    /**
     * The last SPTK event inside this control.
     */
    CEventInfo        m_event;

    /**
     * Does the control have focus?
     */
    bool              m_hasFocus;


protected:

    /**
     * @brief Does the control contain focus?
     */
    bool containsFocus() const;

    /**
     * @brief Internal focus notification on focus change
     */
    void notifyFocus(bool gotFocus = true);

    /**
     * @brief Internal focus notification on focus change
     */
    virtual void onEnter();

    /**
     * @brief Internal focus notification on focus change
     */
    virtual void onExit();

    /**
     * @brief Computes the label height based on the labelFont() and labelWidth()
     */
    uint32_t labelHeight() const;

    /**
     * @brief Special handle() method
     */
    int handle(int) override;

    /**
     * @brief Internal callback function
     */
    static void internalCallback(Fl_Widget *internalWidget, void *data);

public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CControl(const char *label, int layoutSize = 20, CLayoutAlign layoutAlign = SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CControl(int,int,int,int,const char * = 0);
#endif

public:

    /**
     * @brief Resizes the control and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    void resize(int x, int y, int w, int h) override;

    /**
     * @brief Draws the control
     */
    void draw() override;

    /**
     * @brief Returns the control's user data tag
     */
    virtual int tag() const
    {
        return m_tag;
    }

    /**
     * @brief Sets the control's user data tag
     */
    virtual void tag(int t)
    {
        m_tag = t;
    }

    /**
     * @brief Returns the control's text font
     */
#if FL_MAJOR_VERSION < 2
    virtual Fl_Font textFont() const;

    /**
     * @brief Sets the control's text font
     */
    virtual void textFont(Fl_Font);
#else

    virtual Font* textFont() const;

    /**
     * @brief Sets the control's text font
     */
    virtual void textFont(Font *);
#endif

    /**
     * @brief Returns the control's text font size
     */
    virtual uchar textSize() const;

    /**
     * @brief Sets the control's text font size
     */
    virtual void textSize(uchar);

    /**
     * @brief Returns the control's text color
     */
    virtual Fl_Color textColor() const;

    /**
     * @brief Sets the control's text color
     */
    virtual void textColor(Fl_Color);

    /**
     * @brief Returns the control's label color
     */
    virtual Fl_Color labelColor() const;

    /**
     * @brief Sets the control's label color
     */
    virtual void labelColor(Fl_Color);

    /**
     * @brief Returns the control's background color
     */
    virtual Fl_Color color() const;

    /**
     * @brief Sets the control's background color
     */
    virtual void color(Fl_Color);

    /**
     * @brief Returns the control's flags
     */
    uint32_t flags() const
    {
        return m_controlFlags;
    }

    /**
     * @brief Sets the control's flags
     */
    void flags(uint32_t flags);

    /**
     * @brief Returns main widget inside the container
     */
    Fl_Widget* control() const
    {
        return m_control;
    }

    /**
     * @brief Returns control's label
     */
    const String& label() const
    {
        return m_label;
    }

    /**
     * @brief Sets control's label
     */
    void label(const String&);

    /**
     * @brief Returns control's menu
     */
    Fl_Menu_* menu() const;

    /**
     * @brief Sets control's menu
     */
    void menu(const Fl_Menu_Item*);

    /**
     * @brief Returns control's label font
     */
    Fl_Font labelFont() const
    {
        return labelfont();
    }

    /**
     * @brief Sets control's label font
     */
    void labelSize(Fl_Font f)
    {
        labelfont(f);
    }

    /**
     * @brief Returns control's label font size
     */
    uchar labelSize() const;

    /**
     * @brief Sets control's label font size
     */
    void labelSize(uchar);

    /**
     * @brief Returns control's label width
     */
    uint32_t labelWidth() const
    {
        return m_labelWidth;
    }

    /**
     * @brief Sets control's label width
     */
    void labelWidth(uint32_t);

    /**
     * @brief Sets control's hint (tooltip)
     * @param str               Tooltip text
     */
    void hint(const String& str)
    {
        if (m_control) {
            m_hint = str;
            m_control->tooltip(m_hint.c_str());
        }
    }

    /**
     * @brief Returns control's hint (tooltip)
     */
    String hint() const
    {
        return m_hint;
    }

    /**
     * @brief Returns control's max input length, if applicable
     */
    virtual int maxLength() const
    {
        return 0;
    }

    /**
     * @brief Sets control's max input length, if applicable
     */
    virtual void maxLength(int)
    {
    }

    /**
     * @brief Returns control's field name for universal data connection
     */
    virtual const String& fieldName() const
    {
        return m_fieldName;
    }

    /**
     * @brief Sets control's field name for universal data connection
     */
    virtual void fieldName(const String&);

    /**
     * @brief Returns control's kind (internal SPTK RTTI).
     */
    virtual CControlKind kind() const = 0;

    /**
     * @brief Returns control's class name (internal SPTK RTTI).
     */
    String className() const override
    {
        return "control";
    }

    /**
     * @brief Sets the control to empty text (for the input entries)
     *
     * Also sets no selection state / default selection (for the lists)
     */
    virtual void reset()
    {
        data("");
    }

    /**
     * @brief Loads control data from query fields
     */
    virtual void load(Query *)
    {
    }

    /**
     * @brief Saves control data to query params
     */
    virtual void save(Query *)
    {
    }

    /**
     * @brief Loads control data from XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void load(const xml::Node* node, CLayoutXMLmode xmlMode);

    /**
     * @brief Loads control data from XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void load(const xml::Node* node)
    {
        load(node, LXM_DATA);
    }

    /**
     * @brief Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void save(xml::Node* node, CLayoutXMLmode xmlMode) const;

    /**
     * @brief Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     */
    virtual void save(xml::Node* node) const
    {
        save(node, LXM_DATA);
    }

    /**
     * @brief Returns true if the control state is valid, and the data is inside the limits (if applicable)
     */
    virtual bool valid() const = 0;

    /**
     * @brief The universal data connector
     * @returns control data
     */
    virtual Variant data() const
    {
        return Variant("");
    }

    /**
     * @brief The universal data connector, sets control data
     */
    virtual void data(const Variant v)
    {
    }

    /**
     * @brief Fires the event generated by this control
     */
    void fireEvent(CEvent ev, int32_t arg);

    /**
     * @brief Returns the last event fired by this control
     */
    const CEventInfo& event() const
    {
        return m_event;
    }

    /**
     * @brief Returns the event type for the last fired by this control
     */
    CEvent eventType() const
    {
        return m_event.type();
    }

    /**
     * @brief Returns the event argument for the last fired by this control
     */
    int eventArgument() const
    {
        return m_event.argument();
    }

    /**
     * @brief Converts control name to control kind
     */
    static CControlKind controlNameToType(const String& typeName, int& maxLength, const String& values = "");

    /**
     * @brief Control data assignment
     */
    CControl& operator =(const String& str)
    {
        data(str);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(const char *str)
    {
        data(str);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(int32_t v)
    {
        data(v);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(uint32_t v)
    {
        data(v);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(int64_t v)
    {
        data(v);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(uint64_t v)
    {
        data(v);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(float v)
    {
        data(v);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(double v)
    {
        data(v);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(DateTime dt)
    {
        data(dt);
        return *this;
    }

    /**
     * @brief Control data assignment
     */
    CControl& operator =(Field& fld)
    {
        data(fld);
        return *this;
    }

    /**
     * @brief Control data conversion
     */
    operator String() const
    {
        return data();
    }

    /**
     * @brief Control data conversion
     */
    operator int32_t() const
    {
        return data();
    }

    /**
     * @brief Control data conversion
     */
    operator uint32_t() const
    {
        return data();
    }

    /**
     * @brief Control data conversion
     */
    operator int64_t() const
    {
        return data();
    }

    /**
     * @brief Control data conversion
     */
    operator uint64_t() const
    {
        return data();
    }

    /**
     * @brief Control data conversion
     */
    operator float() const
    {
        return data();
    }

    /**
     * @brief Control data conversion
     */
    operator double() const
    {
        return data();
    }

    /**
     * @brief Control data conversion
     */
    operator DateTime() const
    {
        return data();
    }

public:
    /**
     * @brief Default callback function to support CControl default menu item 'Copy'
     */
    static void defaultControlMenuCopy(Fl_Widget *, void *);

    /**
     * @brief Default callback function to support CControl default menu item 'Delete'
     */
    static void defaultControlMenuCut(Fl_Widget *, void *);

    /**
     * @brief Default callback function to support CControl default menu item 'Paste'
     */
    static void defaultControlMenuPaste(Fl_Widget *, void *);

    /**
     * @brief Default callback function to support CControl default menu item 'Clear'
     */
    static void defaultControlMenuClear(Fl_Widget *, void *);

    /**
     * @brief CControl default menu
     */
    static const Fl_Menu_Item defaultControlMenu[];
};

/**
 * @brief Creates a single control
 *
 * The utility function that allows to create a control with one step
 * @param controlKind int, a control kind
 * @param label std::string, a control label
 * @param fieldName std::string, a control field name
 * @param layoutSize int, a control layout size
 */
CControl *createControl(int controlKind, const String& label, const String& fieldName, int layoutSize);

/**
 * @brief Create a control or a group of controls
 *
 * Every XML node in xmlControls should be CONTROL node
 * and may contain the following elements:
 *   type       a control type, one of the
 *      {
 *        AREA CODE, CURRENCY, CREDIT CARD, COMBO, CHOICE, CHECK BUTTONS,
 *        DATE, DATE AND TIME, FLOAT, GROUP, HTML, INTEGER, LABEL, LONG ZIP CODE,
 *        MASK, MULTIPLE CHOICE, MONEY, PHONE, PHONE EXT, PLAIN TEXT,
 *        PHONE NUMBER, PHONE EXTENSION, SIMPLE, STRING, SSN, TIME, ZIP CODE
 *      }
 *   label      a label, optional
 *   fieldName  a field name, optional
 *   values     a list of values separated with '|' for list-based controls, string, optional
 *   sqlValues  an SQL query to build the list values, string, optional, presented as a sub-node with CDATA
 *   visible    visibility flag, optional, true or false (1/0)
 *   enable     enable flage, optional, true or false (1/0)
 *   align      layout align, optional, one of { top,bottom,left,right,client}, the default is top
 *   size       layout size in pixels, optional
 *
 * The group node may contain other controls. If the control type or parameter is not recognized,
 * the exception is thrown.
 * @param xmlControls           The controls description in XML
 */
void createControls(const xml::NodeList& xmlControls);

/**
 * @}
 */
}
#endif
