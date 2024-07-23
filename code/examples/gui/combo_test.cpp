/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       combo_test.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cgui>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

void theme_cb(Fl_Widget* w, void*)
{
    try
    {
        auto*  themesCombo = (CComboBox*) w;
        String themeName = themesCombo->data().asString();

        if (themesCombo->eventType() == CEvent::DATA_CHANGED)
        {
            CThemes::set(themeName);

            auto* window = (CWindow*) w->window();
            window->relayout();
            window->redraw();
        }
    }
    catch (const Exception& e)
    {
        spError(e.what());
    }
}

void exit_cb(Fl_Widget* w, void*)
{
    w->window()->hide();
}

void combo_cb(Fl_Widget* w, void*)
{
    auto* control = dynamic_cast<CControl*>(w);
    if (!control)
    {
        return;
    }
    switch (control->eventType())
    {
        case CEvent::FOCUS:
            COUT("Got focus");
            break;
        case CEvent::UNFOCUS:
            COUT("Lost focus");
            break;
        case CEvent::DATA_CHANGED:
            COUT("Data Changed");
            break;
        case CEvent::ADD_ITEM:
            COUT("Add Item Command");
            break;
        case CEvent::EDIT_ITEM:
            COUT("Edit Item Command");
            break;
        case CEvent::DELETE_ITEM:
            COUT("Delete Item Command");
            break;
        case CEvent::MOUSE_CLICK:
            COUT("Mouse Click");
            break;
        case CEvent::MOUSE_DOUBLE_CLICK:
            COUT("Mouse Double Click");
            break;
        case CEvent::KEYBOARD:
            COUT("Keyboard Key Pressed");
            break;
        default:
            break;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // Initialize themes
        CThemes allThemes;

        CWindow w(450, 200, "ComboBox Test");

        CComboBox cb("Test Combo Box:");

        CColumnList columns;
        columns.push_back(CColumn("name", VariantDataType::VAR_STRING, 45));
        columns.push_back(CColumn("phone", VariantDataType::VAR_STRING, 140));
        columns.push_back(CColumn("city", VariantDataType::VAR_STRING, 120));
        cb.columns(columns);

        Strings sl1("Alex|(415)-123-45678|SF", "|");
        Strings sl2("Eric|(510)-123-45678|Oakland", "|");
        Strings sl3("Gordon|(650)-123-45678|Los Angeles", "|");
        Strings sl4("Mark|(408)-123-45678|San Mateo", "|");
        Strings sl5("Алекс|(408)-123-45678|Сан Франциско", "|");

        cb.addRow(0, sl1);
        cb.addRow(0, sl2);
        cb.addRow(0, sl3);
        cb.addRow(0, sl4);
        cb.addRow(0, sl5);

        cb.buttons(
            (int) CButtonKind::BROWSE_BUTTON | (int) CButtonKind::ADD_BUTTON | (int) CButtonKind::EDIT_BUTTON |
            (int) CButtonKind::DELETE_BUTTON);
        cb.callback(combo_cb);

        // That group keeps togeteher the buttons. These
        // buttons use the default alignment for buttons -
        // CLayoutAlign::RIGHT, and the text/icon defined by the
        // button kind.
        CGroup buttonGroup("", 10, CLayoutAlign::BOTTOM);
        buttonGroup.color(FL_LIGHT1);

        CButton exitButton(CButtonKind::EXIT_BUTTON);
        exitButton.callback(exit_cb);

        CComboBox themesCombo("Theme", 200, CLayoutAlign::LEFT);
        Strings   themes = CThemes::availableThemes();
        themesCombo.addRows("Theme", themes);
        themesCombo.callback(theme_cb);
        themesCombo.data("Default");

        buttonGroup.end();

        w.resizable(w);
        w.end();
        w.show(argc, argv);

        return Fl::run();
    }
    catch (const Exception& e)
    {
        CERR(e.what());
        return 1;
    }
}
