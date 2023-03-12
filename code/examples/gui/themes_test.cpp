/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       themes_test.cpp - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>
#include <sptk5/cgui>

#include <FL/Fl.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/Fl_Group.H>

using namespace std;
using namespace sptk;

void exit_cb(Fl_Widget* w, void*)
{
    w->window()->hide();
}

void theme_cb(Fl_Widget* w, void*)
{
    auto* themesCombo = (CComboBox*) w;
    String themeName = themesCombo->data().asString();

    CThemes::set(themeName);

    auto* window = (CWindow*) w->window();
    window->relayout();
    window->redraw();
}

int main(int argc, char** argv)
{
    try
    {
        // Initialize themes
        CThemes allThemes;

        Fl::visual(FL_DOUBLE | FL_INDEX);

        xdoc::Document doc;
        CWindow w(550, 450, "SPTK themes test");

        w.layoutSpacing(10);

        CGroup group1("", 10);
        group1.box(FL_DOWN_BOX);
        CButton testButton1(CButtonKind::OK_BUTTON, CLayoutAlign::LEFT);
        CButton testButton2(CButtonKind::CANCEL_BUTTON, CLayoutAlign::LEFT);
        testButton1.defaultButton(true);
        group1.end();

        CCheckButtons checkButtons("Check buttons");
        checkButtons.buttons(Strings("button 1|button 2", "|"));
        checkButtons.data("button 1");

        CRadioButtons radioButtons("Radio buttons");
        radioButtons.buttons(Strings("button 1|button 2", "|"));
        radioButtons.data("button 1");

        CInput inp1("input 1");
        CInput inp2("input 2");

        CDBListView listView("List View:", 10, CLayoutAlign::CLIENT);
        listView.columns().push_back(CColumn("column 1", VariantDataType::VAR_INT, 70));
        listView.columns().push_back(CColumn("column 2", VariantDataType::VAR_INT, 70));
        listView.columns().push_back(CColumn("column 3", VariantDataType::VAR_STRING, 200));
        listView.columns().push_back(CColumn("column 4", VariantDataType::VAR_STRING, 300));

        listView.showGrid(false);

        char buffer1[20];
        char buffer2[20];
        int maxItems = 20;
        for (int a = 0; a < maxItems; a++)
        {
            snprintf(buffer1, sizeof(buffer1) - 1, "%i", a);
            snprintf(buffer2, sizeof(buffer2) - 1, "%i", maxItems - a);
            cpchar rowData[] = {buffer1, buffer2, "Column 2", "-----------Long column-----------"};
            auto* ps = new CPackedStrings(4, rowData);
            listView.addRow(ps);
        }

        CProgressBar progressBar("Progress", 20, CLayoutAlign::TOP);
        progressBar.data(50);

        CGroup buttonGroup("", 10, CLayoutAlign::BOTTOM);
        buttonGroup.box(FL_DOWN_BOX);

        CComboBox themesCombo("Theme:", 350, CLayoutAlign::LEFT);
        Strings themes = CThemes::availableThemes();
        themesCombo.addRows("Theme", themes);
        themesCombo.callback(theme_cb);
        themesCombo.labelWidth(70);

        auto* exitButton = new CButton(CButtonKind::EXIT_BUTTON, CLayoutAlign::RIGHT);
        exitButton->callback((Fl_Callback*) exit_cb);
        exitButton->defaultButton(true);

        buttonGroup.end();

        themesCombo.data("Default");

        w.end();
        w.resizable(w);

        w.show(argc, argv);

        w.relayout();

        return Fl::run();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
        return 1;
    }
}
