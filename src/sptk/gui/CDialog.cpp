/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDialog.cpp - description                              ║
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <sptk5/Exception.h>
#include <sptk5/Registry.h>
#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/db/Query.h>
#include <sptk5/gui/CMessageDialog.h>

using namespace std;
using namespace sptk;

//==============================================================================
void CDialogTabs::prepareNewPage(Fl_Group* page, bool autoColor)
{
    CTabs::prepareNewPage(page, autoColor);
    if (pageCount() == 1) {
        box(FL_THIN_DOWN_FRAME);
        showTabs(false);
        page->box(FL_NO_BOX);
    } else {
        box(FL_THIN_UP_BOX);
        showTabs(true);
        page->box(FL_FLAT_BOX);
    }
}

//==============================================================================
CDialog::CDialog(int w, int h, const char* label)
        :
        CWindow(w, h, label)
{
    m_queriesBuilt = false;
    m_controlsScanned = false;
    m_keyValue = -1;
    m_defaultButton = nullptr;

    fl_cursor(FL_CURSOR_WAIT);
    Fl::check();

    m_selectQuery = new Query;
    m_updateQuery = new Query;
    m_insertQuery = new Query;

    m_buttonGroup = new CGroup("", 20, SP_ALIGN_BOTTOM);
    m_okButton = new CButton(SP_OK_BUTTON);
    m_cancelButton = new CButton(SP_CANCEL_BUTTON);
    m_buttonGroup->end();

    m_pages = new CDialogTabs;

    defaultButton(m_okButton);

    newPage("", false);

    m_modalResult = DMR_NONE;
}

CDialog::~CDialog()
{
    delete m_selectQuery;
    delete m_updateQuery;
    delete m_insertQuery;
}

int CDialog::handle(int event)
{
    int rc = CWindow::handle(event);
    if (rc)
        return rc;
    switch (event) {
        case FL_KEYBOARD:
            if (Fl::event_key() == FL_Enter) {
                auto btn = dynamic_cast<Fl_Button*>(Fl::focus());
                if (btn)
                    btn->do_callback();
                else if (m_defaultButton)
                    m_defaultButton->do_callback();
                return 1;
            }
            break;
        default:
            break;
    }
    return 0;
}

void CDialog::defaultButton(CButton* newDefaultButton)
{
    auto cnt = (unsigned) m_buttonGroup->children();
    for (unsigned bi = 0; bi < cnt; bi++) {
        auto button = (CButton*) m_buttonGroup->child(bi);
        button->defaultButton(button == newDefaultButton);
    }
    m_defaultButton = newDefaultButton;
}

bool CDialog::okPressed()
{
    try {
        save();
    } catch (exception& e) {
        spError(e.what());
        return false;
    }
    return true;
}

bool CDialog::cancelPressed()
{
    return true;
}

bool CDialog::showModal()
{
    resize(x(), y(), w(), h());
    int x = (Fl::w() - w()) / 2;
    int y = (Fl::h() - h()) / 2;
    position(x, y);
    set_modal();
    fl_cursor(FL_CURSOR_DEFAULT);
    show();
    bool rc = false;

    m_modalResult = DMR_NONE;

    while (m_modalResult == DMR_NONE) {
        Fl_Widget* pressed = Fl::readqueue();

        if (!pressed)
            Fl::wait(0.1);
        else {
            fl_cursor(FL_CURSOR_WAIT);
            Fl::check();
            if (pressed == this || pressed == m_cancelButton) {
                if (cancelPressed()) {
                    m_modalResult = DMR_CANCEL;
                    break;
                }
            }
            if (pressed == m_okButton) {
                if (okPressed()) {
                    rc = true;
                    m_modalResult = DMR_OK;
                    break;
                }
            }
            fl_cursor(FL_CURSOR_DEFAULT);
        }
    }

    if (m_modalResult == DMR_USER)
        rc = true;

    fl_cursor(FL_CURSOR_DEFAULT);
    hide();
    Fl::check();
    return rc;
}

void CDialog::database(PoolDatabaseConnection* db)
{
    m_selectQuery->database(db);
    m_updateQuery->database(db);
    m_insertQuery->database(db);
}

PoolDatabaseConnection* CDialog::database() const
{
    return m_selectQuery->database();
}

void CDialog::table(const String& tableName)
{
    if (m_tableName != tableName) {
        m_tableName = tableName;
        m_queriesBuilt = false;
    }
}

void CDialog::table(PoolDatabaseConnection* db, const String& tb, const String& key)
{
    database(db);
    table(tb);
    keyField(key);
}

void CDialog::keyField(const String& fldName)
{
    if (m_keyField != fldName) {
        m_keyValue = -1;
        m_keyField = fldName;
        m_queriesBuilt = false;
    }
}

void CDialog::keyValue(int val)
{
    m_keyValue = val;
    try {
        load();
    } catch (exception& e) {
        m_keyValue = -1;
        spError(e.what());
    }
}

bool CDialog::buildQueries()
{
    stringstream columnNames;
    stringstream paramNames;
    stringstream updateNames;

    if (!m_controlsScanned)
        scanControls();

    if (m_queriesBuilt)
        return true;

    bool first = true;
    for (auto itor: m_defaultFields) {
        const CControl* control = itor.second;
        const string& fieldName = control->fieldName();

        if (first)
            first = false;
        else {
            columnNames << ",";
            paramNames  << ",";
            updateNames << ",";
        }

        columnNames << fieldName;
        paramNames  << ":" << fieldName;
        updateNames << fieldName << "=:" << fieldName;
    }

    m_selectQuery->sql("SELECT " + columnNames.str() + " FROM " + m_tableName + " WHERE " + m_keyField + "=:key");
    m_insertQuery->sql("INSERT INTO " + m_tableName + "(" + columnNames.str() + ") VALUES (" + paramNames.str() + ")");
    m_updateQuery->sql("UPDATE " + m_tableName + " SET " + updateNames.str() + " WHERE " + m_keyField + "=:key");

    m_queriesBuilt = true;

    return true;
}

bool CDialog::load()
{
    buildQueries();
    reset();
    if (m_keyValue > 0) {
        m_selectQuery->param("key").setInteger(m_keyValue);
        m_selectQuery->open();
        auto itor = m_defaultFields.begin();
        for (; itor != m_defaultFields.end(); ++itor) {
            try {
                CControl* control = itor->second;
                control->load(m_selectQuery);
            } catch (exception& e) {
                spError(e.what());
            }
        }
        m_selectQuery->close();
    }
    return true;
}

bool CDialog::save()
{

    if (!m_controlsScanned)
        scanControls();

    if (m_defaultFields.empty())
        return true;

    // Validating data in controls
    auto itor = m_defaultFields.begin();
    for (; itor != m_defaultFields.end(); ++itor) {
        CControl* control = itor->second;
        if (!control->valid()) {
            Fl::focus(control->control());
            throw Exception(control->label() + " entry has the incorrect value!");
        }
        if (control->flags() & FGE_MANDATORY) {
            string test = control->data();
            if (!trim(test).length()) {
                Fl::focus(control->control());
                throw Exception(control->label() + " entry can't be empty!");
            }
        }
    }

    if (!database())
        return true;
    buildQueries();
    Query* query = m_insertQuery;

    if (m_keyValue > 0) {
        query = m_updateQuery;
        query->param("key").setInteger(m_keyValue);
    }

    itor = m_defaultFields.begin();
    for (; itor != m_defaultFields.end(); ++itor) {
        CControl* control = itor->second;
        control->save(query);
    }
    query->exec();

    return true;
}

void CDialog::load(const xml::Node* node)
{
    CLayoutManager::loadLayout(node, LXM_DATA);
    loadPosition(node);
}

void CDialog::save(xml::Node* node) const
{
    CLayoutManager::saveLayout(node, LXM_DATA);
    savePosition(node);
}

CControl& CDialog::operator[](const String& fieldName)
{
    if (!m_controlsScanned)
        scanControls();
    auto itor = m_allFields.find(fieldName);
    if (itor != m_allFields.end())
        return *itor->second;
    throw Exception("The dialog window doesn't have a field '" + fieldName + "'");
}

CButton* CDialog::addExtraButton(CButtonKind buttonKind, const char* label, Fl_Callback_p callbackFunction)
{
    m_buttonGroup->begin();
    auto extraButton = new CButton(buttonKind, SP_ALIGN_RIGHT, label);
    m_buttonGroup->end();
    if (callbackFunction)
        extraButton->callback(callbackFunction);
    return extraButton;
}

int CDialog::keyValue() const
{
    return m_keyValue;
}

void CDialog::alert(const String& s) const
{
    spWarning(s);
}

void CDialog::scanControls()
{
    m_allFields.clear();
    m_defaultFields.clear();
    m_allFields << *m_pages;
    m_defaultFields = m_allFields;
    m_defaultFields.remove(m_specialProcessingControls);
    m_controlsScanned = true;
    m_queriesBuilt = false;
}

Fl_Group* CDialog::newPage(const char* label, bool autoColor)
{
    return m_pages->newPage(label, autoColor);
}

Fl_Group* CDialog::newScroll(const char* label, bool autoColor)
{
    return m_pages->newScroll(label, autoColor);
}

bool CDialog::reset()
{
    if (!m_controlsScanned)
        scanControls();
    m_allFields.reset();
    return true;
}
