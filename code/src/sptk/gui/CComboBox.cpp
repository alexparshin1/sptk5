/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CComboBox.cpp - description                            ║
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
#include <sptk5/gui/CButton.h>
#include <sptk5/gui/CComboBox.h>
#include <sptk5/gui/CDropDownBox.h>

using namespace std;
using namespace sptk;

enum CChangeType
{
    CT_REFRESH_DATA, CT_SET_KEY, CT_SET_TEXT, CT_CHOOSE_ITEM
};

#define IS_LIST_BOX   1
#define IS_COMBO_BOX  2

namespace sptk {

    class SP_EXPORT CInternalComboBoxPanel : public Fl_Box
    {
        void draw() override;

    public:
        CInternalComboBoxPanel(int x, int y, int w, int h, const char* label = nullptr);

        int handle(int) override;
    };

}

CInternalComboBoxPanel::CInternalComboBoxPanel(int x, int y, int w, int h, const char* label)
        :
        Fl_Box(x, y, w, h, label)
{
    align(FL_ALIGN_LEFT);
}

void CInternalComboBoxPanel::draw()
{
    int focused = -1;
    if (Fl::focus() == this || Fl::focus() == parent())
        focused = 1;

    auto combo = (CBaseListBox*) parent();
    if (!combo)
        return;
    CDBDropDownList* ddl = combo->m_dropDownWindow;
    if (!ddl)
        return;
    CDBDropDownListView* listView = ddl->listView;
    if (!listView)
        return;

    draw_box(FL_DOWN_FRAME, x(), y(), w(), h(), FL_LIGHT3);
    listView->color(FL_LIGHT3);
    CPackedStrings* row = listView->selectedRow();

    fl_clip(x() + 2, y() + 2, w() - 4 - combo->m_buttonSpace, h() - 4);
    draw_box(FL_FLAT_BOX, x(), y(), w(), h(), FL_LIGHT3);
    listView->textFont(combo->textFont());
    listView->textSize(combo->textSize());
    fl_color(FL_FOREGROUND_COLOR);
    listView->item_draw((unsigned) -1, row, x() + 2, y(), w() - 2, h() - 2, focused, FL_ALIGN_CENTER, true);
    fl_pop_clip();
}

int CInternalComboBoxPanel::handle(int event)
{
    auto control = (CControl*) parent();
    color(control->color());
    switch (event) {
        case FL_FOCUS:
            redraw();
            control->notifyFocus();
            return 1;
        case FL_UNFOCUS:
            redraw();
            control->notifyFocus(false);
            return 1;
        case FL_PUSH:
            if (contains(Fl::focus()))
                return 1;
            Fl::focus(this);
            control->notifyFocus();
            return 1;
        case FL_KEYBOARD: {
            int ch = Fl::event_key();
            if (ch == FL_Tab || ch == FL_Enter)
                break;
            auto combo = (CBaseListBox*) parent();
            if (!combo)
                return 0;
            CDBDropDownList* ddl = combo->m_dropDownWindow;
            if (!ddl)
                return 0;
            CDBDropDownListView* listView = ddl->listView;
            if (!listView)
                return 0;
            int oldIntValue = listView->data();
            int rc = listView->handle(event);
            redraw();
            int newIntValue = listView->data();
            if (oldIntValue != newIntValue) {
                combo->fireEvent(CE_DATA_CHANGED, newIntValue);
            }
            return rc;
        }
        default:
            break;
    }
    if (Fl_Box::handle(event))
        return 1;

    return 0;
}

//===========================================================================
const static CButtonKind buttonKind[] = {SP_BROWSE_BUTTON, SP_ADD_BUTTON, SP_EDIT_BUTTON, SP_DELETE_BUTTON,
                                         SP_REFRESH_BUTTON};

void CBaseListBox::comboButtonPressed(Fl_Widget* btn, void* data)
{
    auto combo = (CBaseListBox*) btn->parent();
    if (!combo)
        return;
    combo->button_handle((uint32_t)(long) data);
}

void CBaseListBox::ctor_init(const char* label, int _mode)
{
    m_mode = _mode;
    m_buttonClicked = nullptr;
    if (m_mode == IS_COMBO_BOX) {
        m_buttonSet = SP_BROWSE_BUTTON;
        m_control = new CInternalComboBoxPanel(0, 0, 10, 10);
    } else {
        m_controlFlags = FGE_MULTILINEENTRY;
        m_buttonSet = SP_REFRESH_BUTTON;
        m_control = m_list = new CDBListView;
    }
    m_control->align(FL_ALIGN_LEFT);
    m_buttonSpace = 0;
    for (int i = 0; i < 5; i++) {
        long kind = buttonKind[i];
        auto btn = new CSmallButton(SP_UNDEFINED_BUTTON, SP_ALIGN_NONE);
        btn->buttonImage(buttonKind[i], IS_COMBO_ICON);
        btn->callback(comboButtonPressed);
        btn->user_data((void*) kind);
        btn->visible_focus(false);
        m_buttons[i] = btn;
    }
    if (m_mode == IS_COMBO_BOX)
        m_dropDownWindow = new CDBDropDownList(w(), 200, label);
    else
        m_dropDownWindow = nullptr;

    m_droppedDown = false;
    end();
    resize(x(), y(), w(), h());
}

CBaseListBox::CBaseListBox(const char* label, int layoutSize, CLayoutAlign layoutAlignment, int _mode)
        :
        CControl(label, layoutSize, layoutAlignment)
{
    ctor_init(label, _mode);
}

#ifdef __COMPATIBILITY_MODE__
CBaseListBox::CBaseListBox(int x,int y,int w,int h,const char * label,int _mode)
: CControl(x,y,w,h,label) {
    ctor_init(label,_mode);
}
#endif

CBaseListBox::~CBaseListBox()
{
    delete m_dropDownWindow;
}

void CBaseListBox::clear()
{
    m_list->clear();
}

void CBaseListBox::resize(int x, int y, int w, int h)
{
    //h = this->h();
    if (m_mode == IS_COMBO_BOX)
        h = textSize() + 8;

    int bh = h - 3;
    int bw = bh;

    int extraSpace = 0;

    if (m_mode != IS_COMBO_BOX) {
        bh = 21;
        bw = bh;
        extraSpace = bw + 4;
    } else {
        if (CThemes::sizeButton(THM_BUTTON_COMBO, bw, bh))
            h = bh + 4;
    }

    CControl::resize(x, y, w, h);

    int xright = x + w - 2 - bw;
    int ytop = y + 2;
    m_buttonSpace = 0;
    for (int i = 0; i < 5; i++) {
        Fl_Button* btn = m_buttons[i];
        if (!btn)
            break;
        if (m_buttonSet & buttonKind[i]) {
            btn->resize(xright, ytop, bw, bh);
            btn->show();
            if (m_mode == IS_COMBO_BOX) {
                xright -= bw;
            } else {
                ytop += bh;
            }
            m_buttonSpace += bw;
        } else {
            btn->hide();
        }
    }

    m_control->resize(x + m_labelWidth, y, w - m_labelWidth - extraSpace, h);
    if (m_menuButton)
        m_menuButton->resize(x + m_labelWidth, y, w - m_labelWidth - extraSpace, h);
}

bool CBaseListBox::preferredSize(int& w, int& h)
{
    int maxWidth = 4;
    if (m_mode == IS_COMBO_BOX)
        maxWidth += m_buttonSpace;

    CColumnList& columns = m_list->columns();
    size_t cnt = columns.size();
    for (size_t i = 0; i < cnt; i++)
        maxWidth += columns[i].width();

    if (maxWidth < 30)
        maxWidth = 30;
    maxWidth += m_labelWidth;

    int hh = textSize() + 8;
    if (hh < (int) labelHeight())
        hh = labelHeight();
    if (h < hh)
        h = hh;

    if (w < int(m_labelWidth + m_buttonSpace) + 10)
        w = m_labelWidth + m_buttonSpace + 10;
    if (m_mode == IS_COMBO_BOX) {
        int bw = 0, bh = 0;
        if (CThemes::sizeButton(THM_BUTTON_COMBO, bw, bh)) {
            if (h < bh + 4)
                h = bh + 4;
        }
        if (w > maxWidth)
            w = maxWidth;
    } else {
        if (h < 30)
            h = 30;
        if (h < m_buttonSpace)
            h = m_buttonSpace;
    }
    return false;
}

void CBaseListBox::load(Query* loadQuery)
{
    Query& query = *loadQuery;
    if (!fieldName().length())
        return;
    Field& fld = query[fieldName().c_str()];
    data(*(Variant*)&fld);
}

void CBaseListBox::save(Query* updateQuery)
{
    if (!fieldName().length())
        return;
    QueryParameter& param = updateQuery->param(fieldName().c_str());
    param = data();
}

void CBaseListBox::load(const xml::Node* node, CLayoutXMLmode xmlMode)
{
    CControl::load(node, xmlMode);
}

void CBaseListBox::save(xml::Node* node, CLayoutXMLmode xmlMode) const
{
    CControl::save(node, xmlMode);
}

void CBaseListBox::changeControlData(int changeType, int intData, string stringData)
{
    CPackedStrings* oldSelection = m_list->selectedRow();
    switch (changeType) {
        case CT_REFRESH_DATA:
            m_list->refreshData();
            break;
        case CT_SET_KEY:
            m_list->data(intData);
            break;
        case CT_SET_TEXT:
            m_list->textValue(stringData);
            break;
        case CT_CHOOSE_ITEM:
            m_dropDownWindow->showModal();
            damage(FL_DAMAGE_ALL);
            redraw();
            m_droppedDown = false;
            break;
        default:
            break;
    }
    CPackedStrings* newSelection = m_list->selectedRow();

    if (oldSelection != newSelection)
        fireEvent(CE_DATA_CHANGED, m_list->data());
}

void CBaseListBox::buttons(uint32_t buttonSet)
{
    if (m_buttonSet != buttonSet) {
        m_buttonSet = buttonSet;
        resize(x(), y(), w(), h());
    }
}

void CBaseListBox::button_handle(uint32_t buttonKind)
{
    Fl::focus(m_control);
    switch (buttonKind) {
        case SP_BROWSE_BUTTON:
            if (m_mode == IS_COMBO_BOX)
                dropDownList();
            break;
        case SP_REFRESH_BUTTON:
            refreshData();
            break;
        case SP_ADD_BUTTON:
            m_event = UC_ADD_ITEM;
            do_callback();
            break;
        case SP_EDIT_BUTTON:
            m_event = UC_EDIT_ITEM;
            do_callback();
            break;
        case SP_DELETE_BUTTON:
            m_event = UC_DELETE_ITEM;
            do_callback();
            break;
        default:
            break;
    }
}

void CBaseListBox::dropDownList()
{
    if (m_droppedDown)
        return;
    m_droppedDown = true;
    Fl_Window* parentWindow = window();
    int xx = parentWindow->x() + x() + m_labelWidth;
    int yy = parentWindow->y() + y() + m_control->h();

    int hh = 0, ww = 0;
    m_dropDownWindow->preferredSize(ww, hh);
    ww = w() - m_labelWidth;

    if (hh > Fl::h())
        hh = Fl::h();
    if (xx + ww > Fl::w())
        ww = Fl::w() - xx;

    if (yy + hh > Fl::h()) {
        // Window doesn't fit the screen under the widget
        if (parentWindow->y() + y() + m_control->h() / 2 < Fl::h() / 2) {
            // Trying to make window smaller
            hh = Fl::h() - yy;
        } else {
            // Placing window on top of the widget
            yy = parentWindow->y() + y() - hh;
            if (yy < 0) {
                yy = 0;
                hh += yy;
            }
        }
    }
    m_dropDownWindow->resize(xx, yy, ww + 1, hh);
    changeControlData(CT_CHOOSE_ITEM);
}

CPackedStrings* CBaseListBox::selectedRow() const
{
    return m_list->selectedRow();
}

PoolDatabaseConnection* CBaseListBox::database() const
{
    return m_list->database();
}

void CBaseListBox::database(PoolDatabaseConnection* db)
{
    m_list->database(db);
}

String CBaseListBox::sql() const
{
    return m_list->sql();
}

void CBaseListBox::sql(string s)
{
    m_list->sql(s);
}

QueryParameter& CBaseListBox::param(const char* p)
{
    return m_list->param(p);
}

void CBaseListBox::refreshData()
{
    changeControlData(CT_REFRESH_DATA);
}

Variant CBaseListBox::data() const
{
    return m_list->data();
}

void CBaseListBox::data(const Variant newData)
{
    CPackedStrings* oldSelection = m_list->selectedRow();

    m_list->data(newData);

    CPackedStrings* newSelection = m_list->selectedRow();

    if (oldSelection != newSelection)
        fireEvent(CE_DATA_CHANGED, m_list->data());
}

string CBaseListBox::keyField() const
{
    return m_list->keyField();
}

void CBaseListBox::keyField(string kf)
{
    m_list->keyField(kf);
}

void CBaseListBox::setup(PoolDatabaseConnection* db, string sql, string keyField)
{
    m_list->setup(db, sql, keyField);
}

void CBaseListBox::columns(const CColumnList& cl)
{
    m_list->columns(cl);
}

CColumnList& CBaseListBox::columns()
{
    return m_list->columns();
}

void CBaseListBox::addColumn(string cname, VariantType type, short cwidth, bool cvisible)
{
    m_list->columns().push_back(CColumn(cname, type, cwidth, cvisible));
}

void CBaseListBox::addRow(CPackedStrings* psl)
{
    m_list->addRow(psl);
}

void CBaseListBox::addRow(int rowId, const Strings& ss)
{
    m_list->addRow(rowId, ss);
}

void CBaseListBox::addRows(string columnName, Strings strings)
{
    CColumn newColumn(columnName, VAR_STRING, w() - labelWidth(), true);
    CColumnList newColumns;

    newColumns.push_back(newColumn);
    columns(newColumns);
    size_t cnt = strings.size();

    for (size_t i = 0; i < cnt; i++) {
        String& str = strings[i];
        cpchar strs[2] = {str.c_str(), nullptr};
        auto psl = new CPackedStrings(1, strs);
        int id = str.ident();
        psl->argument(id);
        m_list->addRow(psl);
    }
}

int CBaseListBox::sortColumn() const
{
    return m_list->sortColumn();
}

void CBaseListBox::sortColumn(int column)
{
    m_list->sortColumn(column, true);
}

unsigned CBaseListBox::size() const
{
    return m_list->size();
}

int CBaseListBox::findString(const string& str, bool select, unsigned startRow, unsigned endRow)
{
    return m_list->findString(str, select, startRow, endRow);
}

void CBaseListBox::showHeaders()
{
    m_list->headerHeight(20);
}

void CBaseListBox::hideHeaders()
{
    m_list->headerHeight(0);
}

void CBaseListBox::selectRow(unsigned rowNumber)
{
    m_list->selectRow(rowNumber);
}

//===========================================================================
CComboBox::CComboBox(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        :
        CBaseListBox(label, layoutSize, layoutAlignment, IS_COMBO_BOX)
{
    m_list = m_dropDownWindow->listView;
    m_list->multiSelect(false);
}

#ifdef __COMPATIBILITY_MODE__
CComboBox::CComboBox(int x,int y,int w,int h,const char *l)
: CBaseListBox (x,y,w,h,l,IS_COMBO_BOX) {
    m_list = m_dropDownWindow->listView;
    m_list->multiSelect(false);
}
#endif

CComboBox::~CComboBox() = default;

CControlKind CComboBox::kind() const
{
    return DCV_INTVALUECOMBO;
}

CLayoutClient* CComboBox::creator(xml::Node* node)
{
    auto widget = new CComboBox("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

//===========================================================================
CListBox::CListBox(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        :
        CBaseListBox(label, layoutSize, layoutAlignment, IS_LIST_BOX)
{
}

#ifdef __COMPATIBILITY_MODE__
CListBox::CListBox(int x,int y,int w,int h,const char *l)
: CBaseListBox (x,y,w,h,l,IS_LIST_BOX) {}
#endif
