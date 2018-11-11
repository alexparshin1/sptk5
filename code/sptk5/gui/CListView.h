/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CListView.h - description                              ║
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

#ifndef __LISTVIEW_H__
#define __LISTVIEW_H__

#include <FL/Fl_Group.H>

#include <sptk5/IntList.h>
#include <sptk5/DataSource.h>
#include <sptk5/CSmallPixmapIDs.h>

#include <sptk5/sptk.h>
#include <sptk5/gui/CScrollBar.h>
#include <sptk5/gui/CDataControl.h>
#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CListViewSelection.h>
#include <sptk5/gui/CListViewRows.h>
#include <sptk5/PackedStrings.h>
#include <sptk5/gui/CColumn.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief The refersh kind
 */

enum CRefreshKind
{
    /**
     * Retrieve the full dataset and replace the existing records completely
     */
    LV_REFRESH_FULL,

    /**
     * Retrieve only the changed records and replace the changed records only
     */
    LV_REFRESH_FAST

};

class CInternalComboBoxPanel;
class CDBDropDownList;

/**
 * @brief The data mode for the data() methods.
 */

enum CListViewDataMode
{
    /**
     * data() sets/returns item caption. If the data connection is defined (in CDBListView) - it flips to LV_DATA_KEY mode.
     */
    LV_DATA_UNDEFINED,

    /**
     * data() sets/returns item key value
     */
    LV_DATA_KEY,

    /**
     * data() sets/returns item index
     */
    LV_DATA_INDEX,

    /**
     * data() sets/returns item caption
     */
    LV_DATA_CAPTION

};

/**
 * @brief List view widget
 *
 * Class CListView is the list of multicolumn rows. It allows to sort rows
 * by column, supports type-in incremental search etc.
 */

class SP_EXPORT CListView : public CControl, public SharedMutex
{
    friend class CInternalComboBoxPanel;
    friend class CDBDropDownList;
private:

    /**
     * @brief Constructor initializer
     */
    void ctor_init();

    /**
     * Flag to indicate the current fill() operation
     */
    bool m_fillInProgress;

    /**
     * Flag to terminate the current fill() operation
     */
    bool m_fillTerminated;


protected:
    /**
     * Should the column headers to be capitilized?
     */
    bool m_capitalizeColumnNames;

    /**
     * Is multiple selection allowed?
     */
    bool m_multipleSelection;

    /**
     * The horizontal position - to be
     */
    int m_horizPosition;

    /**
     * The horizontal position - existing
     */
    int m_realHorizPosition;

    /**
     * The maximum required width for the columns
     */
    int m_maxWidth;

    /**
     * The minimal update pointer
     */
    int m_redraw1;

    /**
     * The minimal update pointer
     */
    int m_redraw2;

    /**
     * The scrolling position
     */
    uint32_t m_top;

    /**
     * Scrollbar width for this class of object
     */
    static int m_scrollbarWidth;

    /**
     * Text font, currently used to draw the list view
     */
    int m_currentTextFont;

    /**
     * Text size, currently used to draw the list view
     */
    int m_currentTextSize;

    /**
     * Icon names that may be referred from items (icon size is 16)
     */
    Strings m_iconNames;


    /**
     * @brief Data mode
     * Defines the kind of data to work through data() method
     * @see CListViewDataMode for more information
     */
    CListViewDataMode m_dataMode;

protected:

    /**
     * The mouse pressed position
     */
    int m_mousePressedX;

    /**
     * The list of currently selected items
     */
    CSelection m_selection;

    /**
     * Type-in search buffer
     */
    std::string m_searchPhrase;

    /**
     * The row where the focus is or should be
     */
    uint32_t m_activeRow;

    /**
     * The height of the header area
     */
    uint32_t m_headerHeight;


    /**
     * The list of rows
     */
    CListViewRows m_rows;

    /**
     * The list of columns
     */
    CColumnList m_columnList;


    /**
     * Dragged column
     */
    int m_draggingColumn;

    /**
     * Dragging column started
     */
    int m_draggingStarted;


    /**
     * The text font type
     */
    Fl_Font m_textFont;

    /**
     * The text font size
     */
    uchar m_textSize;

    /**
     * The text font color
     */
    Fl_Color m_textColor;


    /**
     * Show/hide horizontal stripes. Stripes is an alternative to cell grid.
     */
    bool m_showStripes;

    /**
     * Show/hide cell grid. Grid is an alternative to stripes.
     */
    bool m_showGrid;

    /**
     * Show/hide selection
     */
    bool m_showSelection;

    /**
     * Auto row height on/off
     */
    bool m_autoRowHeight;


    /**
     * @brief Computes which column contains x-coordinate
     * @param x int, coordinate
     * @returns column number
     */
    int columnAt(int x);

    /**
     * @brief Finds an item in vertical position y
     * @param y int, vertical position
     * @returns row number, or -1 if not found
     */
    int find_item(int y);

    /**
     * @brief Returns the actual size of the drawing area (internal)
     * @param X int, output, x-xoordinate
     * @param Y int, output, y-xoordinate
     * @param W int, output, width
     * @param H int, output, height
     */
    void bbox(int& X, int& Y, int& W, int& H) const;

protected:

    /**
     * @brief Internal vertical scrollback callback
     */
    static void scrollbar_callback(Fl_Widget* s, void*);

    /**
     * @brief Internal horizontal scrollback callback
     */
    static void hscrollbar_callback(Fl_Widget* s, void*);

    /**
     * @brief Vertical scrollback
     */
    CScrollBar scrollbar;

    /**
     * @brief Horizontal scrollbar
     */
    CScrollBar hscrollbar;

    /**
     * @brief Returns horizontal position of the scroll
     */

    int hposition() const
    {
        return m_horizPosition;
    }

    /**
     * @brief Sets horizontal position of the scroll
     * @param hpos int, new horizontal position
     */
    void hposition(int hpos); // pan to here

    /**
     * @brief Changes the default scrollbar width for the class
     * @param b int, new scrollbar width
     */

    static void scrollbar_width(int b)
    {
        m_scrollbarWidth = b;
    }

    /**
     * @brief Returns the default scrollbar width for the class
     */

    static int scrollbar_width()
    {
        return m_scrollbarWidth;
    }

protected:

    /**
     * @brief Finds item with ID
     * @param id int, item id
     * @returns row number, or -1 if not found
     */
    int find_id(int id) const;

    /**
     * @brief Get focus callback (internal)
     */
    virtual int get_focus();

    /**
     * @brief Returns true if item is selected
     * @param index uint32_t, item index
     */
    virtual bool item_selected(uint32_t index) const;

    /**
     * @brief Selects or deselects the item
     * @param index uint32_t, item index
     * @param select bool, select or deselect
     */
    virtual void item_select(uint32_t index, bool select);

    /**
     * @brief Recomputes all items height
     */
    virtual int item_compute_height_all();

    /**
     * @brief Recomputes items height
     * @param rowData CPackedStrings *, item pointer
     * @returns item height
     */
    virtual int item_compute_height(CPackedStrings *rowData);

    /**
     * @brief Recomputes items height
     * @param index uint32_t, row number
     * @returns item height
     */
    virtual int item_height(uint32_t index) const;

    /**
     * @brief Returns item width
     * @param index uint32_t, row number
     * @returns item width
     */
    virtual int item_width(uint32_t index) const;

    /**
     * @brief Draws a row in coordinates (internal)
     * @param index uint32_t, row number
     * @param rowData const CPackedStrings *, row pointer
     * @param x int, x-coordinate
     * @param y int, y-coordinate
     * @param w int, width
     * @param h int, height
     * @param focusMode int, focus flag
     * @param verticalAlign int, vertical align
     * @param paintBackground bool, true if the item should paint the background
     * @returns item width
     */
    virtual void item_draw(uint32_t index, const CPackedStrings *rowData, int x, int y, int w, int h, int focusMode, int verticalAlign, bool paintBackground) const;

    /**
     * Internal mouse callback
     */
    virtual void item_clicked(int mouse_btn);

    /**
     * @brief The function that allows to overwrite the default text color for the cell.
     * @param column int, the column number
     * @param rowData const CPackedStrings&, row
     * @returns textColor()
     */
    virtual Fl_Color item_color(int column, const CPackedStrings& rowData) const;

    /**
     * @brief Draws header (internal)
     * @param x int, x-coordinate
     * @param y int, y-coordinate
     * @param hh int, header height
     */
    virtual void header_draw(int x, int y, int hh) const;

    /**
     * @brief Header clicked mouse callback
     * @param x int, x-coordinate
     */
    virtual void header_clicked(int x);

    /**
     * @brief Returns true if x is the drag-sensitive area
     */
    virtual bool header_drag_position(int x);

    /**
     * @brief Header dragged to position x notification
     * @param x int, x-coordinate
     */
    virtual void header_dragged(int x);

    /**
     * @brief Header released in position x notification
     * @param x int, x-coordinate
     */
    virtual void header_released(int x);

    /**
     * @brief Request to redraw line index
     * @param index uint32_t, row number
     */
    void redraw_line(uint32_t index);

    /**
     * @brief Redraw all the lines
     */

    void redraw_lines()
    {
        damage(FL_DAMAGE_SCROLL);
    }

    /**
     * @brief General draw function. Draws everything
     */
    void draw() override;

    /**
     * @brief Internal callback if selected another item with the different user _data (key).
     */
    void key_changed(uint32_t index);

public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CListView(const char *label = 0, int layoutSize = 20, CLayoutAlign layoutAlign = SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CListView(int, int, int, int, const char* = 0);
#endif

    /**
     * @brief Destructor
     */

    ~CListView()
    {
        clear();
    }

    /**
     * @brief Overwritten handle()
     */
    int handle(int) override;

    /**
     * @brief Resizes the control and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    void resize(int x, int y, int w, int h) override;

    /**
     * @brief Removes row
     * @param index             Row number
     */
    virtual void removeRow(uint32_t index);

    /**
     * @brief Adds new row
     *
     * Optional row ID may be defined inside row parameter.
     * @param row               Row data
     */
    virtual void addRow(CPackedStrings *row);

    /**
     * @brief Adds new row
     *
     * Optional row ID may be defined inside row parameter,
     * or through ident parameter.
     * @param ident int, row argument (key value)
     * @param row const Strings&, row data
     */
    virtual void addRow(int ident, const Strings& row);

    /**
     * @brief Inserts new row
     *
     * Optional row argument may be defined inside row parameter,
     * or through ident parameter.
     * @param position uint32_t, insert position
     * @param row const Strings&, row data
     * @param ident int, row argument (key value)
     */
    virtual void insertRow(uint32_t position, const Strings& row, int ident);

    /**
     * @brief Updates a row
     *
     * Optional row ID may be defined inside row parameter.
     * @param position uint32_t, insert position
     * @param row CPackedStrings *, row data
     */
    virtual void updateRow(uint32_t position, CPackedStrings *row);

    /**
     * @brief Updates a row. Optional row argument may be defined inside row parameter.
     * @param position uint32_t, insert position
     * @param row const Strings&, row data
     * @param ident int, row argument (key value)
     */
    virtual void updateRow(uint32_t position, const Strings& row, int ident = 0);

    /**
     * @brief Removes all the rows
     */
    virtual void clear();

    /**
     * @brief Returns the row count
     */

    virtual uint32_t size() const
    {
        return m_rows.size();
    }

    /**
     * @brief Returns the scroll position
     */

    virtual int top() const
    {
        return (int) m_top;
    }

    /**
     * @brief Sets the scroll position
     * @param index int, top row number
     */
    virtual void top(int index);

    /**
     * Is row number index selected?
     * @param index int, row number
     */
    int selected(int index) const;

    /**
     * Switch the visibility flag for the row index
     * @param index int, row number
     */
    void show(int index);

    /**
     * Show widget
     */
    void show() override
    {
        CControl::show();
    }

    /**
     * Switch the visibility flag for the row index
     * @param index int, row number
     */
    void hide(int index);

    /**
     * Hides the list view widget
     */
    void hide() override
    {
        Fl_Widget::hide();
    }

    /**
     * Is the row visible?
     * @param index int, row number
     */
    int visible(int index) const;

    /**
     * Is the widget visible?
     */

    int visible() const
    {
        return (int) Fl_Widget::visible();
    }

    /**
     * Returns the value the selected row and sort column cell
     */
    std::string textValue() const;

    /**
     * Selects the row where cell in the sort column is tv
     * @param text CString, text to find and select
     */
    void textValue(const std::string& text);

    /**
     * Sets data mode for the data() methods as LV_DATA_KEY, LV_DATA_INDEX, LV_DATA_CAPTION.
     * Defines which information list view is working with in data() method - key value, item index, or item caption.
     * @see CListViewDataMode for more information
     */

    void dataMode(CListViewDataMode dm)
    {
        m_dataMode = dm;
    }

    /**
     * Returns data mode as LV_DATA_KEY, LV_DATA_INDEX, LV_DATA_CAPTION.
     * Defines which information list view is working with in data() method - key value, item index, or item caption.
     * @see CListViewDataMode for more information
     */

    CListViewDataMode dataMode()
    {
        return m_dataMode;
    }

    /**
     * The universal data connector,
     * @returns control data
     */
    Variant data() const override;

    /**
     * The universal data connector, selects row with user data
     * @param v const CVariant, user data value to find and select
     */
    void data(const Variant v) override;

    /**
     * Returns row pointer
     * @param index int, row number
     * @returns row pointer
     */
    CPackedStrings *row(uint32_t index) const;

    /**
     * Returns selection list
     */

    const CSelection& selection() const
    {
        return m_selection;
    }

    /**
     * Returns currently selected row, or the last selected row in case of multiple selection.
     * Returns 0L if there is no selection.
     */
    CPackedStrings* selectedRow() const;

    /**
     * Returns the row number for the selected row
     */
    int selectedIndex() const;

    /**
     * Selects a row
     * @param index int, row number
     * @param sel bool, true if select
     * @returns true if success
     */

    bool selectRow(uint32_t index, bool sel = true)
    {
        return select(index, sel, true);
    }

    /**
     * Returns true if the row is currently visible in the scroll area
     * @param index int, row number
     */
    bool displayed(uint32_t index) const;

    /**
     * Makes the row visible in the scroll area
     * @param index int, row number
     */
    virtual void displayRow(uint32_t index);

    /**
     * Requests to redraw just one row
     * @param index int, row number
     */

    void redrawRow(uint32_t index)
    {
        redraw_line(index);
    }

protected:

    /**
     * @brief Loads the key value from the query
     */
    void load(Query *) override;

    /**
     * @brief Saves the key value from the query
     */
    void save(Query *) override;

    /**
     * @brief Loads control data from XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    void load(const xml::Node* node, CLayoutXMLmode xmlMode) override;

    /**
     * @brief Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    void save(xml::Node* node, CLayoutXMLmode xmlMode) const override;

    /**
     * @brief True if the data is valid
     */

    virtual bool valid() const override
    {
        return true;
    }

    /**
     * @brief Returns the cell border width
     */
    virtual uint32_t cellBorderWidth() const;

    /**
     * @brief Selects the row. Internal
     * @param index int, row number
     * @param sel bool, true if select
     * @param docallbacks bool, true if we want to call callbacks
     * @returns true if success
     */
    bool select(uint32_t index, bool sel = true, int docallbacks = false);

    /**
     * @brief Selects the only row. Internal
     * @param index int, row number
     * @param docallbacks bool, true if we want to call callbacks
     * @returns true if success
     */
    bool select_only(uint32_t index, bool docallbacks = false);

    /**
     * @brief Selects the row. Executes the callbacks. Internal
     * @param index int, row number
     * @returns true if success
     */
    bool activate_row(uint32_t index);

    /**
     * @brief Selects the first row. Executes the callbacks. Internal
     * @returns true if success
     */
    bool select_first();

    /**
     * @brief Selects the last row. Executes the callbacks. Internal
     * @returns true if success
     */
    bool select_last();

    /**
     * @brief Selects the next row. Executes the callbacks. Internal
     * @returns true if success
     */
    bool select_next();

    /**
     * @brief Selects the prior row. Executes the callbacks. Internal
     * @returns true if success
     */
    bool select_prior();

    /**
     * @brief Selects the next page row. Executes the callbacks. Internal
     * @returns true if success
     */
    bool select_next_page();

    /**
     * @brief Selects the prior page row. Executes the callbacks. Internal
     * @returns true if success
     */
    bool select_prior_page();

public:
    /**
     * @brief Sets a new column list
     * @param columns const CColumnList&, new column list
     */
    void columns(const CColumnList& columns);

    /**
     * @brief Returns column list
     */

    CColumnList& columns()
    {
        return m_columnList;
    }

    /**
     * @brief Returns column informartion
     * @param col uint32_t, column number 0..columnCount() -1
     */

    CColumn& column(uint32_t col)
    {
        return m_columnList[col];
    }

    /**
     * @brief Returns column informartion
     * @param colname const char *, column name
     */
    CColumn& column(const char *colname);

    /**
     * @brief Adds a new column as a copy of column
     * @param column const CColumn&, new column information
     */

    void addColumn(const CColumn& column)
    {
        m_columnList.push_back(column);
    }

    /**
     * @brief Adds a new column as a copy of column
     * @param colname const char *, column name
     * @param type VariantType, column data type
     * @param cwidth uint32_t, column width
     * @param cvisible bool, is the column visible?
     */

    void addColumn(const String& colname, VariantType type, uint32_t cwidth = 100, bool cvisible = true)
    {
        m_columnList.push_back(CColumn(colname, type, (short) cwidth, cvisible));
    }

    /**
     * @brief SPTK RTTI information
     * @returns control type as CControlKind
     * @see CControlKind
     */

    CControlKind kind() const override
    {
        return DCV_LISTBOX;
    }

    /**
     * @brief Returns auto row height mode as true/false
     */

    bool autoRowHeight()
    {
        return m_autoRowHeight;
    }

    /**
     * @brief Sets auto row height mode as true/false
     */
    void autoRowHeight(bool arh);

    /**
     * @brief Returns text font type
     */

    Fl_Font textFont() const override
    {
        return m_textFont;
    }

    /**
     * @brief Sets text font type
     */

    void textFont(Fl_Font f) override
    {
        m_textFont = f;
    }

    /**
     * Returns text font size
     */

    uchar textSize() const override
    {
        return m_textSize;
    }

    /**
     * @brief Sets text font size
     */

    void textSize(uchar s) override
    {
        m_textSize = s;
    }

    /**
     * @brief Returns text color
     */

    Fl_Color textColor() const override
    {
        return m_textColor;
    }

    /**
     * @brief Sets text color
     */

    void textColor(Fl_Color n) override
    {
        m_textColor = n;
    }

    /**
     * @brief Returns show grid mode
     */

    bool showGrid()
    {
        return m_showGrid;
    }

    /**
     * @brief Sets show grid mode
     */
    void showGrid(bool flag);

    /**
     * @brief Returns show stripes
     */

    bool showStripes()
    {
        return m_showStripes;
    }

    /**
     * @brief Sets show stripes mode
     */
    void showStripes(bool flag);

    /**
     * @brief Returns show selection mode
     */

    bool showSelection()
    {
        return m_showSelection;
    }

    /**
     * @brief Sets show selection mode
     */

    void showSelection(bool flag)
    {
        m_showSelection = flag;
    }

    /**
     * @brief Returns multiple selection enabled or disabled
     */

    bool multiSelect() const
    {
        return m_multipleSelection;
    }

    /**
     * @brief Sets multiple selection enabled or disabled
     */

    void multiSelect(bool ms)
    {
        m_multipleSelection = ms;
    }

    /**
     * @brief Returns a selected rows ID list
     */
    void getSelections(IntList& selection) const;

    /**
     * @brief Sets a selected rows ID list
     */
    void setSelections(const IntList& selection);

    /**
     * @brief Finds an item with the caption (a string in the first column).
     * @param caption std::string, the caption to find and select.
     * @returns an item, or NULL if item caption is not found
     */
    CPackedStrings *findCaption(const String& caption);

    /**
     * @brief Finds an item with the key (an integer associated with the item - argument()).
     * @param keyValue int, the caption to find and select.
     * @returns an item, or NULL if item caption is not found
     */
    CPackedStrings *findKey(int keyValue);

    /**
     * @brief Returns maximum width of all items including the header row
     */
    uint32_t fullWidth() const; // current width of all items

    /**
     * @brief Returns summary height of all items including the header row
     */
    uint32_t fullHeight(); // current height of all items

    /**
     * @brief Finds a string in the sort column of list view defined with sortColumn()
     * @param str std::string, string to find
     * @param select bool, true if we want to select the found row
     * @param startRow uint32_t, the row number to start
     * @param endRow uint32_t, the row number to finish
     * @returns the row number, or -1 if not found
     */
    virtual int findString(const std::string& str, bool select = true, uint32_t startRow = 0, uint32_t endRow = 0);

    /**
     * @brief Returns sort column name
     */
    std::string sortColumnName() const;

    /**
     * @brief Sets the sort column
     * @param column int, sort column number
     * @param sortNow bool, true if you want to sort immediatedly
     */
    void sortColumn(int column, bool sortNow);

    /**
     * @brief Returns the sort column number
     */
    int sortColumn() const;

    /**
     * @brief Returns the sort direction - ascending/descending
     */

    bool sortAscending() const
    {
        return m_rows.sortAscending();
    }

    /**
     * @brief Sets the sort direction - ascending/descending
     * @param ascending bool, sort direction - ascending/descending
     * @param sortNow bool, true if you want to sort immediatedly
     */
    void sortAscending(bool ascending, bool sortNow);


    /**
     * @brief Sorts ListView rows using existing information about sort column and direction
     */

    void sort()
    {
        m_rows.sort();
    }

    /**
     * @brief Fills the list view with the data from datasource
     *
     * Uses all available datasource information about column and data types.
     * @param ds CDataSource&, the datasource
     * @param keyFieldName std::string, field used to fill in key values
     * @param recordsLimit uint32_t, maximum records to retrieve
     * @param recordsEstimated uint32_t, records estimated (to use the fill progression event)
     * @param refreshKind CRefreshKind, type of refresh - full or just changed records
     * @see CRefreshKind
     * @see CDataSource
     */
    void fill(DataSource& ds, const String& keyFieldName = "", uint32_t recordsLimit = 0, uint32_t recordsEstimated = 0,
              CRefreshKind refreshKind = LV_REFRESH_FULL);

    /**
     * @brief Fills the list with the data from XML node
     *
     * The XML node should contain the columns information, and optional list items
     * @param node const xml::Node&, the node to load list from
     */
    void loadList(const xml::Node* node);

    /**
     * @brief Saves the list data into XML node
     *
     * The XML node should contain the columns information, and optional list items
     * @param node const xml::Node&, the node to save list into
     */
    void saveList(xml::Node* node) const;

    /**
     * @brief Computes the optimal widgets size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * @brief Returns header height
     */

    uint32_t headerHeight() const
    {
        return m_headerHeight;
    }

    /**
     * @brief Sets the header height, 0 to hide headers
     */

    void headerHeight(uint32_t hh)
    {
        m_headerHeight = hh;
    }

    /**
     * @brief Returns the currently active row
     */

    uint32_t activeRow() const
    {
        return m_activeRow;
    }

    /**
     * @brief Returns true if column names are auto-capitilized
     */

    bool capitalizeColumnNames() const
    {
        return m_capitalizeColumnNames;
    }

    /**
     * @brief Defines if column names should be auto-capitilized
     * @param ccn bool, true if column names should be auto-capitilized
     */

    void capitalizeColumnNames(bool ccn)
    {
        m_capitalizeColumnNames = ccn;
    }

    /**
     * @brief Terminate the current fill() operation (if in progress)
     */

    void terminateFill()
    {
        m_fillTerminated = true;
    }

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */

    String className() const override
    {
        return "list_view";
    }

    /**
     * @brief Sets the images that may be referred from items
     * @param iconNames Strings&, a list of icon names
     */
    void imageCollection(Strings& iconNames);
};
/**
 * @}
 */
}
#endif
