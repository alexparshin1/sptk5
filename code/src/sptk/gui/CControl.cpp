/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CControl.cpp - description                             ║
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

#include <sptk5/gui/CEvent.h>
#include <sptk5/gui/CControl.h>

using namespace std;
using namespace sptk;

const Fl_Menu_Item CControl::defaultControlMenu[] = {
        {"Copy",  FL_CTRL + FL_Insert,  CControl::defaultControlMenuCopy, nullptr, 0, 0, 0, 12, 0},
        {"Cut",   FL_SHIFT + FL_Delete, CControl::defaultControlMenuCut, nullptr, 0, 0, 0, 12, 0},
        {"Paste", FL_SHIFT + FL_Insert, CControl::defaultControlMenuPaste, nullptr, 0, 0, 0, 12, 0},
        {"Clear", 0, CControl::defaultControlMenuClear, nullptr, 0, 0, 0, 12, 0},
        {nullptr, 0, nullptr, nullptr, 0, 0, 0, 0,  0}
};

class CControlKindIndex
{
    typedef map<CControlKind, const string*> CTypeNameMap;
    typedef map<string, CControlKind, CaseInsensitiveCompare> CNameTypeMap;
    static CTypeNameMap m_typeNameMap;
    static CNameTypeMap m_nameTypeMap;

    void registerType(CControlKind type, const char* name);

public:
    CControlKindIndex() noexcept;

    static string name(CControlKind type);

    static CControlKind type(const string& name)
    {
        return type(name.c_str());
    }

    static CControlKind type(const char* name);
};

struct CControlKindName
{
    CControlKind type;
    const char* name;
};

static CControlKindName typeNames[] = {
        {DCV_BOX,           "BOX"},
        {DCV_HTMLBOX,       "HTML"},
        {DCV_STRING,        "STRING"},
        {DCV_MEMO,          "MEMO"},
        {DCV_INTEGER,       "INTEGER"},
        {DCV_FLOAT,         "FLOAT"},
        {DCV_DATE,          "DATE"},
        {DCV_TIME,          "TIME"},
        {DCV_DATETIME,      "DATETIME"},
        {DCV_DATEINTERVAL,  "DATEINTERVAL"},
        {DCV_COMBO,         "COMBO"},
        {DCV_INTVALUECOMBO, "INTVALUECOMBO"},
        {DCV_LISTBOX,       "LISTBOX"},
        {DCV_CHECKBUTTONS,  "CHECKBUTTONS"},
        {DCV_RADIOBUTTONS,  "RADIOBUTTONS"},
        {DCV_PHONE,         "PHONE"},
        {DCV_TREEVIEW,      "TREEVIEW"},
        {DCV_GROUP,         "GROUP"},
        {DCV_SLIDER,        "SLIDER"},
        {DCV_PROGRESS,      "PROGRESS"},
        {DCV_UNKNOWN,       ""}
};

CControlKindIndex::CControlKindIndex() noexcept
{
    CControlKindName* typeName = typeNames;
    while (typeName->type != DCV_UNKNOWN) {
        registerType(typeName->type, typeName->name);
        typeName++;
    }
}

CControlKindIndex::CTypeNameMap CControlKindIndex::m_typeNameMap;
CControlKindIndex::CNameTypeMap CControlKindIndex::m_nameTypeMap;
static CControlKindIndex controlKindIndex;

void CControlKindIndex::registerType(CControlKind type, const char* name)
{
    pair<CNameTypeMap::iterator, bool> insertResult;

    insertResult = m_nameTypeMap.insert(CNameTypeMap::value_type(name, type));
    auto itor = insertResult.first;
    m_typeNameMap[type] = &itor->first;
}

string CControlKindIndex::name(CControlKind type)
{
    auto itor = m_typeNameMap.find(type);
    if (itor == m_typeNameMap.end())
        throw Exception("Control type " + int2string(type) + " is undefined");
    return *itor->second;
}

CControlKind CControlKindIndex::type(const char* name)
{
    auto itor = m_nameTypeMap.find(name);
    if (itor == m_nameTypeMap.end())
        throw Exception("Control name " + string(name) + " is undefined");
    return itor->second;
}

//=========================================================================

void CControl::defaultControlMenuCopy(Fl_Widget* w, void*)
{
    auto control = dynamic_cast<CControl*>(w->parent());
    if (control != nullptr) {
        String text = control->data().asString();
        Fl::copy(text.c_str(), (int) text.length(), 1);
    }
}

void CControl::defaultControlMenuCut(Fl_Widget* w, void*)
{
    auto control = dynamic_cast<CControl*>(w->parent());
    if (control != nullptr) {
        String text = control->data().asString();
        Fl::copy(text.c_str(), (int) text.length(), 1);
        control->data("");
    }
}

void CControl::defaultControlMenuPaste(Fl_Widget* w, void*)
{
    auto control = dynamic_cast<CControl*>(w->parent());
    if (control != nullptr) {
        control->data("");
        Fl::paste(*control->m_control, 1);
    }
}

void CControl::defaultControlMenuClear(Fl_Widget* w, void*)
{
    auto control = dynamic_cast<CControl*>(w->parent());
    if (control != nullptr)
        control->data("");
}

void CControl::ctor_init(const char* label)
{
    if (label != nullptr)
        m_label = label;
    m_limited = false;
    m_controlFlags = FGE_SINGLELINEENTRY;
    m_textFont = FL_HELVETICA;
    m_textSize = (uchar) FL_NORMAL_SIZE;
    m_textColor = FL_FOREGROUND_COLOR;
    m_control = nullptr;
    m_tag = 0;
    m_labelWidth = 100;
    m_labelColor = FL_FOREGROUND_COLOR;
    m_menuButton = nullptr;
    m_hasFocus = false;
    box(FL_NO_BOX);
}

CControl::CControl(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        :
        Fl_Group(0, 0, layoutSize, layoutSize), CLayoutClient(this, layoutSize, layoutAlignment)
{
    ctor_init(label);
}

#ifdef __COMPATIBILITY_MODE__
CControl::CControl(int x,int y,int w,int h,const char * label) :
    Fl_Group(x,y,w,h,""), CLayoutClient(this,w,SP_ALIGN_NONE)
{
    ctor_init(label);
}
#endif

void CControl::flags(unsigned flags)
{
    // Only allow to change user flags
    m_controlFlags &= 0xFF00;
    m_controlFlags |= flags & 0xFF;
}

unsigned CControl::labelHeight() const
{
    fl_font(labelfont(), labelsize());
    int cw = m_labelWidth - 3, ch = 0;
    if (m_labelWidth != 0)
        fl_measure(m_label.c_str(), cw, ch);
    return (unsigned) ch;
}

void CControl::resize(int x, int y, int w, int h)
{
    Fl_Group::resize(x, y, w, h);
    if (m_control != nullptr) {
        int hh = h;
        if (!(m_controlFlags & FGE_MULTILINEENTRY))
            hh = textSize() + 8;
        m_control->resize(x + m_labelWidth, y, w - m_labelWidth, hh);
    }
    if (m_menuButton != nullptr)
        m_menuButton->resize(x + m_labelWidth, y, w - m_labelWidth, h);
}

Fl_Color CControl::labelColor() const
{
    return m_labelColor;
}

void CControl::labelColor(Fl_Color clr)
{
    m_labelColor = clr;
}

void CControl::draw()
{
    Fl_Group::label(nullptr);
    Fl_Group::draw();
    if (m_labelWidth > 5) {
        int hh = labelHeight();
        if (hh != 0) {
            fl_font(labelfont(), labelsize());
            fl_color(m_labelColor);
            fl_draw(m_label.c_str(), x() + 1, y() + 1, m_labelWidth - 3, hh,
                    Fl_Align(FL_ALIGN_TOP | FL_ALIGN_RIGHT | FL_ALIGN_WRAP));
        }
    }
}

void CControl::label(const String& label)
{
    if (m_label != label) {
        m_label = label;
        damage(FL_DAMAGE_ALL);
        draw();
    }
}

void CControl::labelWidth(unsigned newWidth)
{
    if (m_labelWidth != newWidth) {
        m_labelWidth = newWidth;
        resize(x(), y(), w(), h());
    }
}

Fl_Font CControl::textFont() const
{
    return m_textFont;
}

void CControl::textFont(Fl_Font f)
{
    m_textFont = f;
}

uchar CControl::textSize() const
{
    return m_textSize;
}

void CControl::textSize(uchar s)
{
    m_textSize = s;
}

Fl_Color CControl::textColor() const
{
    return m_textColor;
}

void CControl::textColor(Fl_Color n)
{
    m_textColor = n;
}

Fl_Color CControl::color() const
{
    return m_control->color();
}

void CControl::color(Fl_Color clr)
{
    m_control->color(clr);
}

uchar CControl::labelSize() const
{
    return (uchar) labelsize();
}

void CControl::labelSize(uchar sz)
{
    labelsize(sz);
}

void CControl::onEnter()
{
}

void CControl::onExit()
{
}

bool sptk::checkFieldName(const String& fieldName)
{
    size_t len = fieldName.length();
    if (len > 80)
        return false;
    if (len == 0)
        return true;

    for (size_t i = 0; i < len; i++) {
        if (isalnum(fieldName[i]) == 0)
            if (fieldName[i] != '_')
                return false;
    }
    return isalpha(fieldName[0]) != 0;
}

bool CControl::containsFocus() const
{
    return contains(Fl::focus()) != 0;
}

void CControl::notifyFocus(bool gotFocus)
{
    if (gotFocus) {
        if (!m_hasFocus) {
            m_hasFocus = true;
            fireEvent(CE_FOCUS, 0);
        }
    } else {
        if (m_hasFocus && !contains(Fl::focus())) {
            m_hasFocus = false;
            fireEvent(CE_UNFOCUS, 0);
        }
    }
}

int CControl::handle(int event)
{
    switch (event) {
        case FL_SHOW:
            fireEvent(CE_SHOW, 0);
            break;
        case FL_HIDE:
            fireEvent(CE_HIDE, 0);
            break;
        default:
            break;
    }
    int rc = Fl_Group::handle(event);
    if (event == FL_UNFOCUS) {
        bool newFocus = containsFocus();
        if (!newFocus) {
            m_hasFocus = false;
            fireEvent(CE_UNFOCUS, 0);
        }
    }
    return rc;
}

void CControl::fieldName(const String& s)
{
    m_fieldName = s;
}

CControlKind CControl::controlNameToType(const String& typeName, int& maxLength, const String& values)
{
    CControlKind controlType = DCV_UNKNOWN;
    auto c1 = (char) toupper(typeName[1]);
    maxLength = 0;
    switch (toupper(typeName[0])) {
        case 'A':   // AREA CODE
            maxLength = 3;
            return DCV_INTEGER;

        case 'C':   // CURRENCY, CREDIT CARD, COMBO BOX, CHOICE, CHECK BUTTONS
            switch (c1) {
                case 'U':
                    controlType = DCV_FLOAT;
                    break;
                case 'R':
                    controlType = DCV_STRING;
                    maxLength = 16;
                    break;
                case 'H':
                    controlType = DCV_COMBO;
                    if (toupper(typeName[2]) == 'E')
                        controlType = DCV_CHECKBUTTONS;
                    break;
                case 'O':
                    controlType = DCV_COMBO;
                    break;
                default:
                    break;
            }
            break;

        case 'D':   // DATE, DATE AND TIME
            if (typeName.length() > 4)
                controlType = DCV_DATETIME;
            else
                controlType = DCV_DATE;
            break;

        case 'F':   // float #
            controlType = DCV_FLOAT;
            break;

        case 'H':  // HTML
            controlType = DCV_HTMLBOX;
            break;

        case 'G':  // GROUP
            controlType = DCV_GROUP;
            break;

        case 'I':   // INTEGER
            if (trim(values).length())
                controlType = DCV_INTVALUECOMBO;
            else
                controlType = DCV_INTEGER;
            break;

        case 'L':   // LABEL, LONG ZIP CODE
            switch (c1) {
                case 'A':
                    controlType = DCV_BOX;
                    break;
                case 'O':
                    controlType = DCV_STRING;
                    maxLength = 10;
                    break;
                default:
                    break;
            }
            break;

        case 'M':   // MASK, MULTIPLE CHOICE, MONEY
            switch (c1) {
                case 'A':
                    controlType = DCV_STRING;
                    break;
                case 'U':
                    controlType = DCV_CHECKBUTTONS;
                    break;
                case 'O':
                    controlType = DCV_FLOAT;
                    break;
                default:
                    break;
            }
            break;

        case 'P':   // PHONE, PHONE EXT, PLAIN TEXT
            switch (c1) {
                case 'H':
                    if (typeName.find("ext") == STRING_NPOS)
                        controlType = DCV_PHONE; // phone number
                    else {
                        maxLength = 4;
                        controlType = DCV_INTEGER;  // phone extension
                    }
                    break;
                case 'R':
                    controlType = DCV_PROGRESS;  // progress bar
                    break;
                default:
                    controlType = DCV_BOX;
                    break;
            }
            break;

        case 'S':   // SIMPLE, STRING, SSN
            controlType = DCV_STRING;
            break;

        case 'T':   // TIME
            controlType = DCV_TIME;
            break;

        case 'Z':   // Zip code
            maxLength = 10;
            controlType = DCV_STRING;
            break;

        default:
            break;
    }
    return controlType;
}

void CControl::menu(const Fl_Menu_Item* newMenu)
{
    if (newMenu == nullptr) {
        if (m_menuButton != nullptr) {
            remove(m_menuButton);
            delete m_menuButton;
            m_menuButton = nullptr;
        }
        return;
    }
    if (m_menuButton == nullptr) {
        Fl_Group* currentGroup = Fl_Group::current();
        begin();
        m_menuButton = new Fl_Menu_Button(m_labelWidth, 0, w() - m_labelWidth, h());
        m_menuButton->type(Fl_Menu_Button::POPUP23);
        m_menuButton->menu(newMenu);
        m_menuButton->visible_focus(false);
        end();
        Fl_Group::current(currentGroup);
    } else
        m_menuButton->menu(newMenu);
}

Fl_Menu_* CControl::menu() const
{
    return m_menuButton;
}

void CControl::internalCallback(Fl_Widget* internalWidget, void* data)
{
    for (Fl_Widget* parentWidget = internalWidget->parent();
         parentWidget != nullptr; parentWidget = parentWidget->parent()) {
        auto control = dynamic_cast<CControl*>(parentWidget);
        if (control != nullptr) {
            control->fireEvent(CE_DATA_CHANGED, (int32_t)(long)(data));
            break;
        }
    }
}

void CControl::fireEvent(CEvent ev, int32_t arg)
{
    if (callback() != nullptr) {
        m_event = CEventInfo(ev, arg);
        do_callback();
        m_event = CEventInfo(CE_NONE, 0);
    }
}

void sptk::createControls(const xml::NodeList& xmlControls)
{
    for (auto node: xmlControls) {
        CControlKind controlKind = CControlKindIndex::type(node->name());
        CControl* control = createControl(controlKind, node->getAttribute("label", ""),
                                          node->getAttribute("fieldName", ""),
                                          node->getAttribute("size", "12"));
        if ((int) node->getAttribute("visible", "1") == 0)
            control->hide();
        if ((int) node->getAttribute("enable", "1") == 0)
            control->deactivate();
    }
}

void CControl::load(const xml::Node* node, CLayoutXMLmode xmlMode)
{
    if (xmlMode & LXM_LAYOUT)
        CLayoutClient::load(node, LXM_LAYOUT);
    if (xmlMode & LXM_DATA) {
        Variant v;
        v.load(node);
        data(v);
    }
}

void CControl::save(xml::Node* node, CLayoutXMLmode xmlMode) const
{
    node->name("control");
    if (xmlMode & LXM_LAYOUT)
        CLayoutClient::save(node, LXM_LAYOUT);
    if (xmlMode & LXM_DATA) {
        Variant v;
        v = data();
        v.save(node);
    }
}
