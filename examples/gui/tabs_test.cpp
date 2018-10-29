/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       tabs_test.cpp - description                            ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>

#include <sptk5/cgui>

using namespace std;
using namespace sptk;

void cb_OK(Fl_Widget* widget, void*)
{
    widget->window()->hide();
}

// Example dialog. Loads & saves changes automatically.
class CDataDialog : public CDialog
{
public:
    explicit CDataDialog(PoolDatabaseConnection* db)
            :
            CDialog(300, 180, "Example Data Dialog")
    {
        database(db);
        table("companies");
        keyField("comp_id");

        CInput* inp;
        inp = new CInput("Company Name:");
        inp->fieldName("comp_name");

        inp = new CIntegerInput("Employees:");
        inp->fieldName("comp_employees");

        end();
    }

};

CDataDialog* dataDialog;

struct CCompanyInfo
{
    int id;
    const char* name;
    int number;
};

CCompanyInfo companies[] = {{1, "Red Hat",      1000},
                            {2, "MandrakeSoft", 2000},
                            {3, "Oracle",       30000},
                            {4, "Microsoft",
                                                300000}};

CListView* eventsListView;

void printMessage(const String& label, const String& className, const String& event)
{
    if (eventsListView) {
        Strings sl;
        sl.push_back(label);
        sl.push_back(className);
        sl.push_back(event);
        eventsListView->addRow(0, sl);
        int cnt = eventsListView->size();
        if (cnt > 0) {
            eventsListView->displayRow(unsigned(cnt - 1));
            eventsListView->redrawRow(unsigned(cnt - 1));
        }
    }
}

void printMessage(CControl* control, const String& event)
{
    printMessage(control->label(), control->className(), event);
}

void general_cb(Fl_Widget* w, void*)
{
    auto control = dynamic_cast<CControl*>(w);
    if (control) {
        switch (control->eventType()) {
            case CE_FOCUS:
                printMessage(control, "Got focus");
                break;
            case CE_UNFOCUS:
                printMessage(control, "Lost focus");
                break;
            case CE_DATA_CHANGED:
                printMessage(control, "Data Changed");
                break;
            case UC_ADD_ITEM:
                printMessage(control, "Add Item Command");
                break;
            case UC_EDIT_ITEM:
                printMessage(control, "Edit Item Command");
                break;
            case UC_DELETE_ITEM:
                printMessage(control, "Delete Item Command");
                break;
            case CE_MOUSE_CLICK:
                printMessage(control, "Mouse Click");
                break;
            case CE_MOUSE_DOUBLE_CLICK:
                printMessage(control, "Mouse Double Click");
                break;
            case CE_KEYBOARD:
                printMessage(control, "Keyboard Key Pressed");
                break;
            default:
                printMessage(control, "Other Event");
                break;
        }
        return;
    }

    auto button = dynamic_cast<CButton*>(w);
    if (button) {
        printMessage(button->label(), "CButton", "Button Pressed");
        return;
    }
}

void theme_cb(Fl_Widget* w, void*)
{
    auto themesCombo = (CComboBox*) w;
    std::string themeName = themesCombo->data();

    CThemes::set(themeName);

    auto window = (CWindow*) w->window();
    window->redraw();
    //window->relayout();
    //Fl::check();
}

int main(int argc, char** argv)
{
    try {
        // Initialize themes
        CThemes allThemes;

        CWindow w(550, 450, "SPTK general test");

        new CBox("Title box", 20, SP_ALIGN_TOP);

        // This data will be used in Combo Box and List View demos.
        Strings sl1("Alex|(415)-123-45678|SF", "|");
        Strings sl2("Eric|(510)-123-45678|Oakland", "|");
        Strings sl3("Kon|(415)-123-45678|SF", "|");
        Strings sl4("Karina|(415)-123-45678|SF", "|");
        Strings sl5("Marko|(510)-123-45678|Oakland", "|");
        Strings sl6("Jayson|(415)-123-45678|SF", "|");

        {
            auto tabs = new CTabs("", 10, SP_ALIGN_CLIENT);
            tabs->selection_color(15);

            // Buttons demo, see cbutton.h for button_kind constants.
            // Button label is optional.
            {
                auto group = (CGroup*) tabs->newPage(" CButton class ", true);
                group->box(FL_THIN_DOWN_BOX);

                new CBox(
                        "All the buttons in application are sharing the pixmaps.\nThere are no duplicate button pixmaps in the memory.\nThe size of the button is defined by both the pixmap and the label.");
                int button_kind = 1;
                for (int col = 0; col < 5 && button_kind < SP_MAX_BUTTON; col++) {
                    auto columnGroup = new CGroup("", 10, SP_ALIGN_LEFT); // Reserving some space on the page
                    for (int row = 0; row < 6 && button_kind < SP_MAX_BUTTON; row++) {
                        auto btn = new CButton((CButtonKind) button_kind, SP_ALIGN_TOP);
                        btn->callback(general_cb);
                        button_kind *= 2;
                    }
                    columnGroup->end();
                }
            }

            // Widgets demo, shows how useful may be the auto-layout.
            // BTW, you you can continue to use traditional FLTK
            // widget positioning, just define __COMPATIBILITY_MODE__
            // and recompile SPTK.
            {   // It will be many widgets on this page, so creating a scroll
                Fl_Group* t = tabs->newScroll(" Data controls 1 ", true);
                t->labeltype(FL_ENGRAVED_LABEL);

                auto box =
                        new CBox(
                                "All the data controls can be connected to the dataset\nto read or save data. It's done automatically\n if they are created in CDialog window.");
                box->flags(0); // blocking FGE_USEPARENTCOLOR - default value
                box->color(t->color());

                auto htmlBox = new CHtmlBox("HTML output:");
                std::string htmlTxt = " This is a <b>bold</b> <i>italic</i> read-only text in HTML. It is very useful sometimes. ";
                htmlTxt = htmlTxt + htmlTxt;
                htmlTxt = htmlTxt + htmlTxt;
                htmlBox->data(htmlTxt);

                auto memoInput = new CMemoInput("Memo input:", 100);
                memoInput->data(
                        "This is multiline text input.\nIt can be bound with ODBC data for TEXT fields \n(text of unlimited length).");
                memoInput->callback(general_cb);

                auto radioButtons = new CRadioButtons("Radio Buttons:");
                radioButtons->buttons(Strings("Red|Blue|Green|Unknown", "|"));
                radioButtons->callback(general_cb);

                auto checkBoxList = new CCheckButtons("Check Buttons:");
                checkBoxList->buttons(Strings("first,second,third,*", ","));
                checkBoxList->callback(general_cb);

                auto textInput = new CInput("Text Input:");
                textInput->data("some text");
                textInput->callback(general_cb);

                auto integerInput = new CIntegerInput("Integer Input:");
                integerInput->data(12345);
                integerInput->callback(general_cb);

                auto dateIntervalInput = new CDateIntervalInput("Date Interval Input:");
                dateIntervalInput->beginOfInterval(DateTime::Now());
                dateIntervalInput->endOfInterval(DateTime::Now());
                dateIntervalInput->callback(general_cb);

                auto phoneNumberInput = new CPhoneNumberInput("Phone Number Input:");
                phoneNumberInput->data("(415)-123-4567");
                phoneNumberInput->callback(general_cb);

                auto floatInput = new CFloatInput("Float Number Input:");
                floatInput->data(1.2345);
                floatInput->callback(general_cb);

                auto passwordInput = new CPasswordInput("Password Input:");
                passwordInput->data("password");
                passwordInput->callback(general_cb);

                // If the conversion from string to date doesn't work
                // on you system - you can use the correct date/time format
                // or use the DateTime to construct time
                auto dateInput = new CDateInput("Date Input:");
                dateInput->data("10/02/2002");
                dateInput->callback(general_cb);

                auto timeInput = new CTimeInput("Time Input:");
                timeInput->data("10:25AM");
                timeInput->callback(general_cb);

                auto dateTimeInput = new CDateTimeInput("Date and Time Input:");
                dateTimeInput->data("10/02/2002 10:25AM");
                dateTimeInput->callback(general_cb);
            }

            {   // It will be many widgets on this page, so creating a scroll
                Fl_Group* t = tabs->newScroll(" Data controls 2 ", true);
                t->labeltype(FL_ENGRAVED_LABEL);

                auto box = new CBox(
                        "Some of the data controls have list of values\n that can be filled directly or from the database.");
                box->flags(0); // blocking FGE_USEPARENTCOLOR - default value
                box->color(t->color());

                // The example of filling in the combo box without data connection
                auto comboBox1 = new CComboBox("Combo box:");
                CColumnList columns;
                columns.push_back(CColumn("name", VAR_STRING, 50));
                columns.push_back(CColumn("phone", VAR_STRING, 120));
                columns.push_back(CColumn("city", VAR_STRING));
                comboBox1->columns(columns);

                comboBox1->buttons(SP_BROWSE_BUTTON | SP_ADD_BUTTON | SP_EDIT_BUTTON | SP_DELETE_BUTTON);

                comboBox1->callback(general_cb);

                comboBox1->addRow(0, sl1);
                comboBox1->addRow(0, sl2);
                comboBox1->addRow(0, sl3);
                comboBox1->addRow(0, sl4);
                comboBox1->addRow(0, sl5);
                comboBox1->addRow(0, sl6);

                // The example of filling in the combo box from the database.
                // Please, notice (1): It only works if you have ODBC support in SPDB/
                // Please, notice (2): the key field column of query is not shown.
                auto comboBox2 = new CComboBox("DB Combo box:");
                comboBox2->callback(general_cb);

                comboBox2->addColumn("company", VAR_STRING, 70);
                comboBox2->addColumn("comp_employees", VAR_INT, 70);
                comboBox2->addRow(1, Strings("Joy Inc.|123", "|"));
                comboBox2->addRow(2, Strings("Red Cap Inc.|1234", "|"));

                auto listView1 = new CListView("List View 1:", 150);
                listView1->columns(columns);
                listView1->addRow(0, sl1);
                listView1->addRow(0, sl2);
                listView1->addRow(0, sl3);
                listView1->addRow(0, sl4);
                listView1->addRow(0, sl5);
                listView1->addRow(0, sl6);
                listView1->callback(general_cb);

                auto listView2 = new CDBListView("List View 2:", 150);
                listView2->callback(general_cb);
            }

            {
                auto group = (CGroup*) tabs->newPage(" CMemoInput class ", false);
                group->box(FL_THIN_DOWN_BOX);

                auto memoInput = new CMemoInput("Text Editor:", 10, SP_ALIGN_CLIENT);
                memoInput->data(
                        "This is a plain text editor that uses FL_Multiline_Input. By default, it sets wrap(true).");
                memoInput->callback(general_cb);
            }

            {
                auto group = (CGroup*) tabs->newPage(" Nested Tabs ", false);
                group->box(FL_THIN_DOWN_BOX);

                auto nestedTabs = new CTabs("", 10, SP_ALIGN_CLIENT);
                nestedTabs->selection_color(15);

                {
                    auto group1 = (CGroup*) nestedTabs->newPage("Nested Tab 1", true);
                    group1->box(FL_THIN_DOWN_BOX);
                    new CButton("Button 1", SP_ALIGN_LEFT);
                }

                {
                    auto group2 = (CGroup*) nestedTabs->newPage("Nested Tab 2", true);
                    group2->box(FL_THIN_DOWN_BOX);
                    new CButton("Button 2", SP_ALIGN_LEFT);
                }

                group->end();
            }

            tabs->end();
        }

        CGroup statusGroup("", 130, SP_ALIGN_BOTTOM);

        CColumnList columns;
        columns.push_back(CColumn("Label", VAR_STRING, 100));
        columns.push_back(CColumn("Class", VAR_STRING, 100));
        columns.push_back(CColumn("Event", VAR_STRING, 150));
        eventsListView = new CListView("Events in the system:", 400, SP_ALIGN_LEFT);
        eventsListView->columns(columns);

        CGroup buttonGroup("", 10, SP_ALIGN_BOTTOM);

        CComboBox themesCombo("", 10, SP_ALIGN_TOP);
        Strings themes = CThemes::availableThemes();
        themesCombo.addRows("Theme", themes);
        themesCombo.callback(theme_cb);
        themesCombo.labelWidth(0);

        themesCombo.data("Default");

        auto exitButton = new CButton(SP_EXIT_BUTTON, SP_ALIGN_TOP);
        exitButton->callback((Fl_Callback*) cb_OK);

        buttonGroup.end();

        w.end();
        w.resizable(w);

        w.show(argc, argv);
        return Fl::run();
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
