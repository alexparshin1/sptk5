/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CComboBox.h - description                              ║
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

#ifndef __CCOMBOBOX_H__
#define __CCOMBOBOX_H__

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/Query.h>
#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CDBListView.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

class CInternalComboBoxPanel;
class CDBListView;

/**
 * @brief Base list box widget
 *
 * A base class for CListBox and CComboBox.
 * Implements most of the common methods for these two classes.
 */
class SP_EXPORT CBaseListBox: public CControl
{
    friend class CInternalComboBoxPanel;
private:

    /**
     * Internal button callback
     */
    Fl_Callback_p   m_buttonClicked;

    /**
     * The list of buttons
     */
    Fl_Button*      m_buttons[5];

    /**
     * Is the list dropped down (shown)?
     */
    bool            m_droppedDown;

    /**
     * The bit-combination of button IDs
     */
    uint32_t        m_buttonSet;


    /**
     * The internal function to process data changes
     */
    void changeControlData(int changeType, int intData = 0, std::string stringData = "");
    /**
     * The constructor initializer
     */
    void ctor_init(const char *label, int _mode);

    /**
     * Internal buttons callback function.
     */
    static void comboButtonPressed(Fl_Widget *btn, void *data);

protected:
    /**
     * Control mode - CListBox or CComboBox
     */
    int m_mode;

    /**
     * Drop down window - CComboBox only
     */
    CDBDropDownList *m_dropDownWindow;

    /**
     * Internal CDBListView widget
     */
    CDBListView *m_list;

    /**
     * The width of space taken by buttons
     */
    int m_buttonSpace;


    /**
     * Drops down the window with the List View. CComboBox only.
     */
    virtual void dropDownList();

    /**
     * @brief Loads data from the Query
     */
    void load(Query *);

    /**
     * @brief Saves data to the Query
     */
    void save(Query *);

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
     * @brief Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void save(xml::Node* node, CLayoutXMLmode xmlMode) const;

    /**
     * Returns true, if the data is valid.
     */
    bool valid() const
    {
        return true;
    }

protected:

    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     * @param mode int, IS_COMBO_BOX or IS_LIST_BOX - internal
     */
    CBaseListBox(const char *label, int layoutSize, CLayoutAlign layoutAlign, int mode);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     * @param mode int, IS_COMBO_BOX or IS_LIST_BOX - internal
     */
    CBaseListBox(int x,int y,int w,int h,const char *label,int mode);
#endif

    /**
     * Destructor
     */
    ~CBaseListBox();

    /**
     * Internal buttons callback function
     */
    virtual void button_handle(uint32_t buttonKind);

public:

    /**
     * Clears the list view inside
     */
    virtual void clear();

    /**
     * Returns the bitmask of visible buttons' IDs
     */
    uint32_t buttons()
    {
        return m_buttonSet;
    }

    /**
     * Sets the bitmask for visible buttons' IDs
     */
    void buttons(uint32_t);

    /**
     * Returns buttons' callback function
     */
    Fl_Callback_p buttonClicked() const
    {
        return m_buttonClicked;
    }

    /**
     * Sets buttons' callback function
     */
    void buttonClicked(Fl_Callback* c)
    {
        m_buttonClicked = c;
    }

    /**
     * Sets the internal list view columns from the column list. The old column list is destroyed
     * and replaced with a copy of columnList.
     * @param columnList CColumnList&, new column list
     */
    void columns(const CColumnList& columnList);

    /**
     * Returns current column list of the internal list view.
     * @returns the column list reference
     */
    CColumnList& columns();

    /**
     * Adds a new column to the internal list view.
     * @param cname std::string, column name
     * @param type VariantType, column data type
     * @param cwidth int16_t, column width
     * @param cvisible bool, is the column visible?
     */
    void addColumn(std::string cname, VariantType type = VAR_STRING, int16_t cwidth = 70, bool cvisible = true);

    /**
     * Adds a new row to the internal list view. Doesn't make a copy - just
     * inserts the pointer. The row will be destroyed in CCombo destructor.
     * The row ID may be defined inside CPackedStrings object.
     * @param row CPackedStrings *, new row
     */
    void addRow(CPackedStrings *row);

    /**
     * Adds a new row to the internal list view. Makes a copy of data in rowStrings.
     * The row ID may be defined inside Strings object, or with rowID parameter.
     * @param rowId             Row argument (key value)
     * @param row               Row data
     */
    void addRow(int rowId, const Strings& strings);

    /**
     * Creates the column as columnName and adds rows to the internal list view. If the column with such name
     * already exists, it will be used. The method is good the a very simple cases of one-column
     * combo boxes. The row IDs may be defined inside Strings object strings.
     * @param columnName std::string, new columnName
     * @param rows Strings, new rows
     */
    void addRows(std::string columnName, Strings rows);

    /**
     * Resizes the control and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    virtual void resize(int x, int y, int w, int h);

    /**
     * Universal data connection. Operates with selected list view item's ID.
     */
    virtual Variant data() const;

    /**
     * Universal data connection. Operates with selected list view item's ID.
     */
    virtual void data(const Variant v);

    /**
     * Sets data mode for the data() methods.
     * Defines which information list view are working with in data() method - key value, item index, or item caption.
     * @see CListViewDataMode for more information
     */
    void dataMode(CListViewDataMode dm)
    {
        m_list->dataMode(dm);
    }

    /**
     * Returns data mode as LVDM_KEY, LVDM_INDEX, or LVDM_TEXT.
     * @see CListViewDataMode for more information
     */
    CListViewDataMode dataMode()
    {
        return m_list->dataMode();
    }

    /**
     * Finds an item with the caption (a string in the first column).
     * @param caption std::string, the caption to find and select.
     * @returns an item, or NULL if item caption is not found
     */
    CPackedStrings *findCaption(std::string caption)
    {
        return m_list->findCaption(caption);
    }

    /**
     * Finds an item with the key (an integer associated with the item - argument()).
     * @param keyValue int, the caption to find and select.
     * @returns an item, or NULL if item caption is not found
     */
    CPackedStrings *findKey(int keyValue)
    {
        return m_list->findKey(keyValue);
    }

    /**
     * Returns database connection pointer
     */
    PoolDatabaseConnection* database() const;

    /**
     * Defines database connection
     */
    void database(PoolDatabaseConnection* db);

    /**
     * Returns SQL query text
     */
    String sql() const;

    /**
     * Defines SQL query text
     * @param s std::string, SQL text
     */
    void sql(std::string s);

    /**
     * Returns SQL query field name that contains unique row IDS, preferrably - integers.
     */
    std::string keyField() const;

    /**
     * Sets SQL query field name that contains unique row IDS, preferrably - integers.
     * @param keyFieldName std::string, a name of the key field
     */
    void keyField(std::string keyFieldName);

    /**
     * Returns sortColumn
     */
    int sortColumn() const;

    /**
     * Sorts the list
     */
    void sort()
    {
        m_list->sort();
    }

    /**
     * Defines sortColumn.
     * @param sortColumn int, sort column number. -1 means don't sort.
     */
    void sortColumn(int sortColumn);

    /**
     * The fast way to define a database connection.
     * @param db DatabaseConnection *, a pointer to the database
     * @param sql std::string, a SQL query text
     * @param keyField std::string, a name of the query field with unique row IDs
     */
    void setup(PoolDatabaseConnection* db, std::string sql, std::string keyField);

    /**
     * Returns the SQL query parameter reference for the parameter name. It is
     * typical to call:
     *   myCombo.param("customer_id") = 1234;
     * @param paramName const char *, parameter name.
     * @returns the parameter reference, if found, or throws an excception.
     */
    QueryParameter& param(const char *paramName);

    /**
     * Reloads data from the database if database connection is defined.
     * Throws an exception if any error.
     */
    void refreshData();

    /**
     * Finds a string in the sort column of list view defined with sortColumn()
     * @param str std::string, string to find
     * @param select bool, true if we want to select the found row
     * @param startRow uint32_t, the row number to start
     * @param endRow uint32_t, the row number to finish
     * @returns the row number, or -1 if not found
     */
    int findString(const std::string& str, bool select = true, uint32_t startRow = 0, unsigned endRow = 0);

    /**
     * Returns the currently selected row, or reference to NULL if not selected
     */
    CPackedStrings* selectedRow() const;

    /**
     * Selects a row
     * @param rowNumber unsigned, row number
     */
    void selectRow(unsigned rowNumber);

    /**
     * Shows column headers in the list
     */
    void showHeaders();

    /**
     * Hides column headers in the list
     */
    void hideHeaders();

    /**
     * Returns the number of rows in list view
     */
    unsigned size() const;

    /**
     * Computes the optimal widgets size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize(int& w, int& h);
};

/**
 * @brief Combo box widget
 *
 * The extended version of standard combo box that exists in any OS.
 * It may include up to four special buttons to insert, edit, delete, refresh data inside
 * the drop down list besides the standard button to show the drop down list
 */
class SP_EXPORT CComboBox: public CBaseListBox
{
public:
    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CComboBox(const char * label = 0, int layoutSize = 10, CLayoutAlign layoutAlign = SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CComboBox(int x,int y,int w,int h,const char *label = 0);
#endif
    /**
     * Destructor
     */
    ~CComboBox();

    /**
     * SPTK RTTI information
     * @returns control type as CControlKind
     * @see CControlKind
     */
    virtual CControlKind kind() const;

    /**
     * SPTK RTTI information
     * @returns control type name
     */
    virtual String className() const
    {
        return "combo_box";
    }

    /**
     * @brief Creates a combo box based on the XML node information
     */
    static CLayoutClient* creator(xml::Node *node);
};

/**
 * @brief List Box widget
 *
 * The extended version of standard list box that exists in any OS.
 * It may include up to four special buttons to insert, edit, delete, refresh data inside
 * the list.
 */
class SP_EXPORT CListBox: public CBaseListBox
{
public:
    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CListBox(const char * label = 0, int layoutSize = 10, CLayoutAlign layoutAlign = SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CListBox(int,int,int,int,const char * = 0);
#endif

    /**
     * SPTK RTTI information
     * @returns control type as CControlKind
     * @see CControlKind
     */
    virtual CControlKind kind() const
    {
        return DCV_LISTBOX;
    }

    /**
     * SPTK RTTI information
     * @returns control type name
     */
    virtual String className() const
    {
        return "list_box";
    }
};
/**
 * @}
 */
}
#endif
