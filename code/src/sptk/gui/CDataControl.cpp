/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDataControl.cpp - description                         ║
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

#include <sptk5/Strings.h>
#include <sptk5/gui/CDataControl.h>

#include <sptk5/gui/CRadioButtons.h>
#include <sptk5/gui/CCheckButtons.h>
#include <sptk5/gui/CComboBox.h>

using namespace sptk;

CControl* sptk::createControl(int controlKind, const String& label, const String& fieldName, int size)
{
    CControl* control = nullptr;

    const char* lbl = label.c_str();

    switch (controlKind) {
        case DCV_BOX:
            control = new CBox(lbl, size);
            break;
        case DCV_HTMLBOX:
            control = new CHtmlBox(lbl, size);
            break;
        case DCV_RADIOBUTTONS:
            control = new CRadioButtons(lbl, size);
            break;
        case DCV_STRING:
            control = new CInput(lbl, size);
            break;
        case DCV_MEMO:
            control = new CMemoInput(lbl, size);
            break;
        case DCV_INTEGER:
            control = new CIntegerInput(lbl);
            break;
        case DCV_FLOAT:
            control = new CFloatInput(lbl);
            break;
        case DCV_DATETIME:
            control = new CDateTimeInput(lbl);
            break;
        case DCV_DATEINTERVAL:
            control = new CDateIntervalInput(lbl);
            break;
        case DCV_DATE:
            control = new CDateInput(lbl);
            break;
        case DCV_TIME:
            control = new CTimeInput(lbl);
            break;
        case DCV_PHONE:
            control = new CPhoneNumberInput(lbl);
            break;
        case DCV_LISTBOX:
            control = new CListBox(lbl, size);
            break;
        case DCV_CHECKBUTTONS:
            control = new CCheckButtons(lbl);
            break;
        case DCV_COMBO:
            control = new CComboBox(lbl, size);
            break;
        case DCV_INTVALUECOMBO:
            control = new CComboBox(lbl, size);
            break;
        default:
            throw Exception("Unsupported control type, field name '" + fieldName + "'.");
    }

    control->fieldName(fieldName);

    return control;
}
