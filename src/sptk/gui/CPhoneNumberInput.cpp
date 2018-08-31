/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPhoneNumberInput.cpp - description                    ║
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

#include <FL/fl_draw.H>

#include <sptk5/gui/CPhoneNumberInput.h>

using namespace sptk;

void CPhoneNumberInput::ctor_init()
{
    ((CInput_*) m_control)->mask("(999)-999-9999");
    maxLength(14);
}

CPhoneNumberInput::CPhoneNumberInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CInput(label, layoutSize, layoutAlignment, true)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CPhoneNumberInput::CPhoneNumberInput(int x,int y,int w,int h,const char * label)
        : CInput(x,y,w,h,label) {
    ctor_init();
}
#endif

CLayoutClient* CPhoneNumberInput::creator(xml::Node* node)
{
    auto widget = new CPhoneNumberInput("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

void CPhoneNumberInput::preferredWidth(int& w) const
{
    w = (int) fl_width("(000)-000-00000") + labelWidth();
}

Variant CPhoneNumberInput::data() const
{
    return CInput::data();
}

void CPhoneNumberInput::data(const Variant ss)
{
    char* s1 = strdup(ss.asString().c_str());
    auto p1 = new char[15];

    int pos = 0;
    char* s2 = s1;
    char* p2 = p1;
    while (*s2 && pos < 14) {
        switch (pos) {
            case 0:
                *p2 = '(';
                p2++;
                break;
            case 4:
                *p2 = ')';
                p2++;
                break;
            case 5:
            case 9:
                *p2 = '-';
                p2++;
                break;
            default:
                while (*s2 && !isdigit(*s2))
                    s2++;
                if (isdigit(*s2)) {
                    *p2 = *s2;
                    p2++;
                    s2++;
                }
                break;
        }
        pos++;
    }
    *p2 = 0;
    CInput::data(p1);
    delete[] p1;
    free(s1);
}

bool CPhoneNumberInput::valid() const
{
    return true;
}
