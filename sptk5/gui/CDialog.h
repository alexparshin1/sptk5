/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDialog.h - description                                ║
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

#ifndef __CDIALOG_H__
#define __CDIALOG_H__

#include <sptk5/cxml>

#include <sptk5/Strings.h>
#include <sptk5/gui/CDataControl.h>
#include <sptk5/gui/CButton.h>
#include <sptk5/gui/CTabs.h>
#include <sptk5/gui/CGroup.h>
#include <sptk5/gui/CScroll.h>
#include <sptk5/gui/CWindow.h>
#include <sptk5/gui/CControlList.h>

class Fl_Group;

namespace sptk {

class Query;
class PoolDatabaseConnection;
class CButton;
class CControl;
class CDlgControls;

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * The closing dialog result value.
 *
 * You can check it after the dialog exits the modal state, using modalResult() method.
 * If the Ok button was pressed, it would return DMR_OK.
 * For Cancel button it would return DMR_CANCEL. And, for user buttons, it would be DMR_USER.
 */
enum CDialogModalResult
{
    /**
     * Modal result undefined
     */
    DMR_NONE,

    /**
     * Ok button pressed
     */
    DMR_OK,

    /**
     * Cancel button pressed
     */
    DMR_CANCEL,

    /**
     * User-added button pressed
     */
    DMR_USER

};

/**
 * Extended CTabs for dialog.
 *
 * The main difference is - it hides the tab label
 * and the border when there is only one tab.
 */
class CDialogTabs: public CTabs
{
    /**
     * @brief Sets the proper box type for the widget and for the group depending on the number of tabs
     * @param page              Newly added tab page
     * @returns same page
     */
    Fl_Group *adjustPage(Fl_Group *page);

protected:
    /**
     * Sets default parameters for a new page
     */
    virtual void prepareNewPage(Fl_Group *page, bool autoColor);


public:

    /**
     * @brief Default constructor
     */
    CDialogTabs() :
            CTabs("", 10, SP_ALIGN_CLIENT)
    {
    }
};

/**
 * Dialog window.
 *
 * Shows a modal window on the screen.
 * CDialog has support for the database data exchange to edit a database record.
 * The required SQL queries in this case are created on the fly, from the list
 * of controls inside the dialog. Controls should have the field names defined, though.
 * And don't forget to define a table name and key field name.
 */
class SP_EXPORT CDialog: public CWindow
{
    /**
     * @brief Loads window coordinates and widgets from XML node
     *
     * @param node              Node to load data from
     * @param xmlMode           Mode defining how the layout and/or data should be loaded
     */
    void load(const xml::Node* node, CLayoutXMLmode xmlMode) override
    {
        CWindow::load(node, xmlMode);
    }

    /**
     * Saves window coordinates and widgets into XML node
     *
     * @param node              Node to save data into
     * @param xmlMode           Mode defining how the layout and/or data should be loaded
     */
    void save(xml::Node* node, CLayoutXMLmode xmlMode) const override
    {
        CWindow::save(node, xmlMode);
    }

protected:
    /**
     * Standard Ok button
     */
    CButton*        m_okButton;

    /**
     * Standard Cancel button
     */
    CButton*        m_cancelButton;

    /**
     * Pointer to the default button
     */
    CButton*        m_defaultButton;

    /**
     * Database interface: table name
     */
    String          m_tableName;

    /**
     * Database interface: table key field name
     */
    String          m_keyField;

    /**
     * Database interface: table key field value for the record to edit
     */
    int             m_keyValue;

    /**
     * Database interface: The list of controls for standard processing
     */
    CControlList    m_defaultFields;

    /**
     * The list of all controls (CControl-descendants)
     */
    CControlList    m_allFields;

    /**
     * The group buttons are placed on.
     */
    CGroup*         m_buttonGroup;

    /**
     * The tabs
     */
    CDialogTabs*    m_pages;

    /**
     * Database interface: The query to select a database record
     */
    Query*         m_selectQuery;

    /**
     * Database interface: The query to update a database record
     */
    Query*         m_updateQuery;

    /**
     * Database interface: The query to insert a new database record
     */
    Query*         m_insertQuery;

    /**
     * Database interface: The flag indicating if queries are created from available controls.
     */
    bool            m_queriesBuilt;

    /**
     * Database interface: The flag indicating if available controls were scanned for field names.
     */
    bool            m_controlsScanned;

    /**
     * Database interface: The list of the controls to exclude from standard database queries.
     */
    CControlList    m_specialProcessingControls;

    /**
     * The result of exiting the modal mode.
     */
    CDialogModalResult m_modalResult;


    /**
     * Ok button callback
     *
     * If it returns true the dialog closes.
     * False indicates an error and dialog stays open.
     */
    virtual bool okPressed();

    /**
     * Ok button callback
     *
     * If it returns true the dialog closes.
     * False indicates an error and dialog stays open.
     */
    virtual bool cancelPressed();

    /**
     * Scans dialog controls
     *
     * If m_controlsScanned is already true, does nothing.
     */
    virtual void scanControls();

    /**
     * Builds database interface queries using table(), keyField(), and control field names
     *
     * If m_queriesBuilt is already true, does nothing.
     */
    virtual bool buildQueries();

    /**
     * Loads data into dialog
     *
     * Returns true upon success.
     * If you're overwriting this method make sure you're calling
     * the original CDialog::load() and use the result of it.
     */
    virtual bool load();

    /**
     * Saves data from dialog
     *
     * Returns true upon success.
     * If you're overwriting this method make sure you're calling
     * the original CDialog::save() and use the result of it.
     */
    virtual bool save();

public:

    /**
     * Constructor
     * @param w                 Dialog width
     * @param h                 Dialog height
     * @param label             Dialog caption
     */
    CDialog(int w, int h, const char *label = 0);

    /**
     * Destructor
     */
    ~CDialog();

    /**
     * Resets all the controls inside the dialog tabs by calling reset() for every control
     */
    virtual bool reset();

    /**
     * Own handle() method to process dialog events the special way
     * @param event             FLTK event
     * @returns true if event was processed
     */
    int handle(int event) override;

    /**
     * Shows modal dialog
     * @returns true if Ok button was used to close the dialog
     */
    bool showModal();

    /**
     * Shows an alert box
     */
    void alert(const String& s) const;

    /**
     * Defines the default button
     * @param btn               Button in dialog to become the default button
     */
    void defaultButton(CButton *btn);

    /**
     * Sets the database connection
     * @param db                Database connection
     */
    virtual void database(PoolDatabaseConnection *db);

    /**
     * Returns current database connection
     */
    PoolDatabaseConnection *database() const;

    /**
     * Defines database table to use
     * @param tableName         Name of the database table
     */
    void table(const String& tableName);

    /**
     * Returnes used database table
     */
    String table() const
    {
        return m_tableName;
    }

    /**
     * @brief Fast setup of the database connection
     * @param db                Database connection
     * @param tableName         Name of the database table
     * @param keyFieldName      Name of the key field in the database table
     */
    void table(PoolDatabaseConnection* db, const String& tableName, const String& keyFieldName);

    /**
     * Sets the key field name for the database table.
     *
     * The key field contains the unique value to find the database record.
     * @param keyFieldName      Name of the key field in the database table
     */
    void keyField(const String& keyFieldName);

    /**
     * Returns the key field name for the database table.
     */
    String keyField() const
    {
        return m_keyField;
    }

    /**
     * Finds the database record by the value in previously defined key field.
     */
    void keyValue(int val);

    /**
     * Returns the current database record key value.
     */
    int keyValue() const;

    /**
     * Creates a new page as CGroup
     * @param                   Page label
     * @param autoColor         If true the page color is assigned automatically
     * @returns created group
     */
    virtual Fl_Group* newPage(const char *label, bool autoColor);

    /**
     * Creates a new page as CScroll
     * @param label             Page label
     * @param autoColor         If true the page color is assigned automatically
     * @returns created group
     */
    virtual Fl_Group* newScroll(const char *label, bool autoColor);

    /**
     * Adds a user-defined button to the dialog.
     *
     * User-defined buttons usually do something in the dialog
     * without closing the dialog.
     * @param buttonKind CButtonKind, button type
     * @param label const char *, button label
     * @param callback Fl_Callback_p, button callback
     */
    CButton* addExtraButton(CButtonKind buttonKind, const char *label, Fl_Callback_p callback);

    /**
     * Returns the number of controls inside the dialog with defined field names
     */
    uint32_t fieldCount() const
    {
        return (uint32_t) m_allFields.size();
    }

    /**
     * @brief Index operator to access controls with defined field name
     *
     * @param fieldName         The control field name
     */
    CControl& operator [](const String& fieldName);

    /**
     * @brief Makes dialog to scan the widgets inside
     */
    void rescan()
    {
        m_queriesBuilt = false;
        m_controlsScanned = false;
    }

    /**
     * Loads the dialog controls data from XML
     * @param node              XML node to load data from
     * @see xml::Node
     */
    void load(const xml::Node* node) override;

    /**
     * Saves the dialog controls into XML
     * @param node              XML node to save data into
     * @see xml::Node* node
     */
    void save(xml::Node* node) const override;

    /**
     * Returns the modal result of the dialog.
     * @see CDialogModalResult for more information.
     * @returns modal result
     */
    CDialogModalResult modalResult() const
    {
        return m_modalResult;
    }

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    String className() const override
    {
        return "dialog";
    }
};
/**
 * @}
 */
}
#endif
