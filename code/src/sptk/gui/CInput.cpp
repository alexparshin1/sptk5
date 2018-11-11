/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CInput.cpp - description                               ║
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
#include <FL/Fl_Input.H>
#include <FL/fl_draw.H>

#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CInput.h>

using namespace std;
using namespace sptk;

// If you define this symbol as zero you will get the peculiar fltk
// behavior where moving off the end of an input field will move the
// cursor into the next field:
// define it as 1 to prevent cursor movement from going to next field:
#define NORMAL_INPUT_MOVE 0

#define ctrl(x) ((x)^0x40)

static const char phoneMask[] = "(999)-999-9999";

static const char maskControlCharacters[] = "@0123456789AaLlZz#Tt";

static string reformatPhoneNumber(const char* st)
{
    string result(st);

    // Leave only digits!
    size_t len = strlen(st);
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (isdigit(st[i])) {
            result[j] = st[i];
            j++;
        }
    }
    result[j] = 0;
    if (result.length() == 11 && result[0] == '1')
        result = result.substr(1, 10); // US long distance starts from 1
    switch (result.length()) {
        case 7:
            result = "(   )-" + result.substr(0, 3) + "-" + result.substr(3, 4);
            break;
        default:
            result = "(" + result.substr(0, 3) + ")-" + result.substr(3, 3) + "-" + result.substr(6, 4);
            break;
    }
    return result;
}

void CInput_::mask(const char* m)
{
    m_mask = "";
    m_inputMask = "";
    m_backgroundMask = "";

    if (m == nullptr)
        return;

    m_mask = m;
    m_inputMask = m;
    m_backgroundMask = m;

    auto bg_mask_ptr = (char*) m_backgroundMask.c_str();
    auto input_mask_ptr = (char*) m_inputMask.c_str();

    size_t l = m_mask.length();
    size_t j = 0;
    for (size_t i = 0; i < l; i++, j++) {
        if (m[i] == '\\' || !strchr(maskControlCharacters, m[i])) {   // background char
            if (m[i] == '\\') i++;
            bg_mask_ptr[j] = m[i];
            input_mask_ptr[j] = ' ';
        } else {
            bg_mask_ptr[j] = ' ';
            input_mask_ptr[j] = m[i];
        }
    }
    bg_mask_ptr[j] = 0;
    input_mask_ptr[j] = 0;
}

bool CInput_::checkCharacterAtPos(int pos, char key)
{
    bool rc;
    if (pos >= (int) m_inputMask.length()) return false;
    switch (m_inputMask[pos]) {
        case '@':   // optional character - digit,letter
            rc = true;
            break;
        case 'L':    // capital character, must
            rc = isalpha(key) && key == toupper(key);
            break;
        case 'l':   // small character, must
            rc = isalpha(key) && key != toupper(key);
            break;
        case 'Z':    // capital character, optional
            rc = (isalpha(key) && key == toupper(key)) || key == ' ';
            break;
        case 'z':   // small character, optional
            rc = (isalpha(key) && key != toupper(key)) || key == ' ';
            break;
        case '#':
            rc = isdigit(key) || key == ' ';    // optional digit
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':   // Digit, must
        case '9':
            rc = isdigit(key) && key <= m_inputMask[pos];
            break;
        case 'T':   // A/P for AM/PM, must
            rc = key == 'A' || key == 'P';
            break;
        case 't':   // a/p for am/pm, must
            rc = key == 'a' || key == 'p';
            break;
        default :   // background mask char?
            rc = m_backgroundMask[pos] == key;
            break;
    }
    return rc;
}

bool CInput_::checkCharacter(int pos, char& key)
{
    if (!m_inputMask[0]) return true;
    char et[] = {0, 0};
    size_t maxPos = m_inputMask.length();
    while (pos < (int) maxPos) {
        if (m_inputMask[pos] == ' ') {
            // use background mask
            et[0] = m_backgroundMask[pos];
            replace(pos, pos + 1, et, 1);
            pos++;
            position(pos, Fl::event_state(FL_SHIFT) ? mark() : pos);
        } else {
            if (checkCharacterAtPos(pos, key)) {
                et[0] = key;
                int rc = replace(pos, pos + 1, et, 1);
                return rc > 0;
            }
            return true;
        }
    }
    return true;
}

int CInput_::handle(int event)
{
    if (event != FL_KEYBOARD) {
        switch (event) {

            case FL_ENTER:
                fl_cursor(FL_CURSOR_INSERT, FL_BLACK, FL_WHITE);
                break;

            case FL_LEAVE:
                fl_cursor(FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE);
                break;

            case FL_FOCUS: {
                auto control = dynamic_cast<CControl*>(parent());
                if (!control && parent())
                    control = dynamic_cast<CControl*>(parent()->parent());
                if (control)
                    control->notifyFocus();
            }
                break;

            default:
                break;
        }

        int rc = Fl_Input::handle(event);

        if (event == FL_PASTE)
            maskValue();

        return rc;
    }

    char ascii = Fl::event_text()[0];

    int del;
    if (Fl::compose(del)) {
        int pos = position();
        char& key = ascii;
        if (m_maxLength && pos >= m_maxLength) return 0;
        if (m_inputMask[0])
            return checkCharacter(pos, key);
        else
            switch (type()) {
                case FL_FLOAT_INPUT:
                    // allow only certain characters
                    if (!strchr("0123456789.eE+-", key)) return 0;
                    // for certain characters allow only one
                    if (strchr(".eE", key)) {
                        if (strchr(value(), key)) return 0;
                    }
                    break;
                case FL_INT_INPUT:
                    key = (char) toupper(key);
                    if (!strchr("0123456789XABCDEF+-", key)) return 0;
                    if (!pos && (key == '+' || key == '-')) return 0;
                    if (key > '9') {
                        // we allow 0xabc style hex numbers to be typed:
                        if (pos == 1 && index(0) == '0' && key == 'X');
                        else if (pos > 1 && index(0) == '0' && index(1) == 'X'
                                 && (key >= 'A' && key <= 'F'));
                        else return 0;
                    }
                    break;

                default:
                    break;
            }

        return replace(position(), mark(), Fl::event_text(), Fl::event_length());
    }

    return Fl_Input::handle(event);
}

CInput_::CInput_(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : Fl_Input(0, 0, layoutSize, layoutSize, label), CLayoutClient(this, layoutSize, layoutAlignment)
{
    m_maxLength = 0;
}

#ifdef __COMPATIBILITY_MODE__
CInput_::CInput_(int x, int y, int w, int h, const char *l)
: Fl_Input(x, y, w, h, l), CLayoutClient(this,w,SP_ALIGN_NONE)
{
   m_maxLength = 0;
}
#endif

bool CInput_::preferredSize(int& w, int& h)
{
    h = textSize() + 6 + Fl::box_dh(box());

    int ml = maxLength();
    if (ml > 0 && ml < 40) {
        fl_font(textFont(), textSize());
        int maxWidth = (int) (ml * fl_width('W')) + 4 + Fl::box_dw(box());
        if (w > maxWidth) w = maxWidth;
    } else if (w < 16) w = 16;

    return false;
}

void CInput_::maskValue()
{
    if (!m_inputMask[0])
        return;

    string val = (char*) value();

    if (strcmp(m_mask.c_str(), phoneMask) == 0)
        val = reformatPhoneNumber(val.c_str());

    size_t cnt = val.length();
    if (cnt > m_inputMask.length())
        cnt = m_inputMask.length();

    for (size_t pos = 0; pos < cnt; pos++) {
        if (!checkCharacterAtPos((int) pos, val[pos]))
            val[pos] = m_backgroundMask[pos];
    }
    val[cnt] = 0;
    value(val.c_str());
}

//=========================================================================================
void CInput::ctor_init(bool autoCreate)
{
    if (autoCreate) {
        m_control = new CInput_("");
        m_control->align(FL_ALIGN_LEFT);
        m_control->callback(CControl::internalCallback);
        m_control->when(FL_WHEN_CHANGED);
        m_control->color(FL_BACKGROUND2_COLOR);
        menu(CControl::defaultControlMenu);
    } else {
        m_control = nullptr;
    }
    end();
}

CInput::CInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CControl(label, layoutSize, layoutAlignment)
{
    ctor_init(true);
}

CInput::CInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment, bool autoCreate)
        : CControl(label, layoutSize, layoutAlignment)
{
    ctor_init(autoCreate);
}

#ifdef __COMPATIBILITY_MODE__
CInput::CInput(int x,int y,int w,int h,const char *label,bool autoCreate)
        : CControl(x,y,w,h,label)
{
    ctor_init(autoCreate);
}
#endif

CInput::~CInput() = default;

CLayoutClient* CInput::creator(xml::Node* node)
{
    auto widget = new CInput("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

int CInput::maxLength() const
{
    auto input = dynamic_cast<CInput_*>(m_control);
    if (input)
        return input->maxLength();
    return 0;
}

void CInput::maxLength(int ml)
{
    auto input = dynamic_cast<CInput_*>(m_control);
    if (input)
        input->maxLength(ml);
}

int CInput::controlType() const
{
    auto input = dynamic_cast<CInput_*>(m_control);
    if (input)
        return input->type();
    return 0;
}

void CInput::controlType(int type)
{
    auto input = dynamic_cast<CInput_*>(m_control);
    if (input)
        input->type((unsigned char) (type));
}

Fl_Font CInput::textFont() const
{
    return ((CInput_*) m_control)->textfont();
}

void CInput::textFont(Fl_Font f)
{
    ((CInput_*) m_control)->textfont(f);
}

uchar CInput::textSize() const
{
    return (uchar) ((CInput_*) m_control)->textsize();
}

void CInput::textSize(uchar s)
{
    ((CInput_*) m_control)->textsize(s);
}

void CInput::load(Query* loadQuery)
{
    if (!m_fieldName.length())
        return;
    Field& fld = (*loadQuery)[m_fieldName.c_str()];
    data(*(Variant*)&fld);
}

void CInput::save(Query* updateQuery)
{
    if (!m_fieldName.length())
        return;
    QueryParameter& param = updateQuery->param(m_fieldName.c_str());
    param = data();
}

Variant CInput::data() const
{
    return ((CInput_*) m_control)->value();
}

void CInput::data(const Variant s)
{
    String strValue = s.asString();
    ((CInput_*) m_control)->value(strValue.c_str());
}

void CInput::preferredWidth(int& w) const
{
    int ml = maxLength();
    if (ml > 0 && ml <= 80) {
        w = (int) (ml * fl_width('W')) + m_labelWidth + 4 + Fl::box_dw(box());
    } else {
        if (w < (int) m_labelWidth + 16)
            w = m_labelWidth + 16;
    }
}

void CInput::preferredHeight(int& h) const
{
    int hh = textSize() + 6 + Fl::box_dh(box());
    if (hh < (int) labelHeight())
        hh = labelHeight();
    if (m_controlFlags & FGE_MULTILINEENTRY) {
        if (h < hh)
            h = hh;
    } else {
        h = hh;
    }
}

bool CInput::preferredSize(int& w, int& h)
{
    fl_font(textFont(), textSize());
    preferredWidth(w);
    preferredHeight(h);
    return false;
}
