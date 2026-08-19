/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/sptk.h>

#include <sptk5/Strings.h>
#include <sptk5/gui/CDataControl.h>

#include <sptk5/gui/CCheckButtons.h>
#include <sptk5/gui/CComboBox.h>

using namespace sptk;

CControl* sptk::createControl(CControlKind controlKind, const String& label, const String& fieldName, int size)
{
    CControl* control = nullptr;

    const char* lbl = label.c_str();

    switch (controlKind)
    {
        case CControlKind::BOX:
            control = new CBox(lbl, size);
            break;
        case CControlKind::HTMLBOX:
            control = new CHtmlBox(lbl, size);
            break;
        case CControlKind::RADIOBUTTONS:
            control = new CRadioButtons(lbl, size);
            break;
        case CControlKind::STRING:
            control = new CInput(lbl, size);
            break;
        case CControlKind::MEMO:
            control = new CMemoInput(lbl, size);
            break;
        case CControlKind::INTEGER:
            control = new CIntegerInput(lbl);
            break;
        case CControlKind::FLOAT:
            control = new CFloatInput(lbl);
            break;
        case CControlKind::DATETIME:
            control = new CDateTimeInput(lbl);
            break;
        case CControlKind::DATEINTERVAL:
            control = new CDateIntervalInput(lbl);
            break;
        case CControlKind::DATE:
            control = new CDateInput(lbl);
            break;
        case CControlKind::TIME:
            control = new CTimeInput(lbl);
            break;
        case CControlKind::PHONE:
            control = new CPhoneNumberInput(lbl);
            break;
        case CControlKind::LISTBOX:
            control = new CListBox(lbl, size);
            break;
        case CControlKind::CHECKBUTTONS:
            control = new CCheckButtons(lbl);
            break;
        case CControlKind::COMBO:
        case CControlKind::INTVALUECOMBO:
            control = new CComboBox(lbl, size);
            break;
        default:
            throw Exception("Unsupported control type, field name '" + fieldName + "'.");
    }

    control->fieldName(fieldName);

    return control;
}
