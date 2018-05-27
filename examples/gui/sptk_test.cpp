/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       sptk_test.cpp - description                            ║
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
#include <FL/Fl_Clock.H>
#include <FL/Fl_Return_Button.H>
#include <FL/fl_ask.H>

#include <sptk5/sptk.h>
#include <sptk5/cgui>

#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace sptk;

#if HAVE_ODBC == 1
#define ODBC_DEMO
#include <sptk5/db/CODBCConnection.h>
#endif

CTabs *tabs;

static void cb_OK(Fl_Return_Button*, void*)
{
    exit(0);
}

// Example dialog. Loads & saves changes automatically.
class CDataDialog: public CDialog
{
public:
    CDataDialog(DatabaseConnection *db) :
            CDialog(300, 180, "Example Data Dialog")
    {
        database(db);

        table("companies");
        if (db->driverDescription() == "SQL Server") // For MS SQL Server
            table("##companies");
        keyField("comp_id");

        CInput *inp;
        inp = new CInput("Company Name:");
        inp->fieldName("comp_name");

        inp = new CIntegerInput("Employees:");
        inp->fieldName("comp_employees");

        end();
    }
};

CDataDialog *dataDialog;

static void cb_show_dialog(void*, void*)
{
    // Select a record with id = 3.
    // Where are different ways to pass this value from outside,
    // but it's out of scope for this example. So I simply set
    // the value :).
    if (!dataDialog)
        return;
    dataDialog->keyValue(3);
    if (dataDialog->showModal()) {
        fl_alert("Ok pressed, data changes are stored.");
    } else {
        fl_alert("Cancel pressed, data changes are ignored.");
    }
}

struct CCompanyInfo
{
    int id;
    const char* name;
    int number;
};

CCompanyInfo companies[] = { { 1, "Red Hat", 1000 }, { 2, "MandrakeSoft", 2000 }, { 3, "Oracle", 30000 }, { 4, "Microsoft",
        300000 } };

CListView *eventsListView;

void printMessage(string label, string className, string event)
{
    if (eventsListView) {
        Strings sl;
        sl.push_back(label);
        sl.push_back(className);
        sl.push_back(event);
        eventsListView->addRow(sl);
        int cnt = eventsListView->size();
        eventsListView->displayRow(cnt - 1);
        eventsListView->redrawRow(cnt - 1);
    }
}

void printMessage(CControl *control, string event)
{
    printMessage(control->label(), control->className(), event);
}

void general_cb(Fl_Widget *w, void *data)
{
    CControl *control = dynamic_cast<CControl *>(w);
    if (control) {
        switch (control->eventType())
        {
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

    CButton *button = dynamic_cast<CButton *>(w);
    if (button) {
        printMessage(button->label(), "CButton", "Button Pressed");
        return;
    }
}

void theme_cb(Fl_Widget *w, void *)
{
    CComboBox *themesCombo = (CComboBox *) w;
    std::string themeName = themesCombo->data();

    CThemes::set(themeName);

    CWindow *window = (CWindow *) w->window();
    window->relayout();
    window->redraw();
}

int main(int argc, char **argv)
{

#ifdef ODBC_DEMO
    // If you want to test the database abilities of the data controls
    // you have to setup the ODBC database connection.
    // Typical connect string is something like: "DSN=odbc_demo;UID=user;PWD=password".
    // If UID or PWD are omitted they are read from the datasource settings.
    CODBCConnection db("DSN=odbc_demo");
    string tableName("companies");
    string isTemp("TEMP");

    try {
        /// Connecting to the database
        db.open();

        if (db.driverDescription() == "SQL Server") { // For MS SQL Server
            tableName = "##companies";
            isTemp = "";
        }
        CQuery query1(&db, "CREATE " + isTemp + " TABLE " + tableName + "(comp_id INT,comp_name CHAR(20),comp_employees INT)");
        CQuery query2(&db, "INSERT INTO " + tableName + "(comp_id,comp_name,comp_employees) VALUES (:id,:name,:emp)");

        // First, create a temporary table.
        query1.exec();

        // Now, fill it with some abstract data.
        // The method used is not effective but very readable.
        // To speed it up use references to parameters.
        for (unsigned i = 0; i < 4; i++) {
            query2.param("id") = companies[i].id;
            query2.param("name") = companies[i].name;
            query2.param("emp") = companies[i].number;
            query2.exec();
        }
    } catch (exception& e) {
        spError("<b>" + string(e.what()) + "</b><br/>Sorry, you have to fix your database connection.");
        return 4;
    }
#endif

    CWindow w(550, 450, "SPTK general test");

    // This data will be used in Combo Box and List View demos.
    Strings sl1("Alex|(415)-123-45678|SF", "|");
    Strings sl2("Eric|(510)-123-45678|Oakland", "|");
    Strings sl3("Kon|(415)-123-45678|SF", "|");
    Strings sl4("Karina|(415)-123-45678|SF", "|");
    Strings sl5("Marko|(510)-123-45678|Oakland", "|");
    Strings sl6("Jayson|(415)-123-45678|SF", "|");

    {
        tabs = new CTabs("", 10, SP_ALIGN_CLIENT);
        tabs->selection_color(15);

        // Buttons demo, see cbutton.h for button_kind constants.
        // Button label is optional.
        {
            CGroup *group = (CGroup *) tabs->newPage("CButton class", true);
            group->box(FL_THIN_DOWN_BOX);

            new CBox(
                    "All the buttons in application are sharing the pixmaps.\nThere are no duplicate button pixmaps in the memory.\nThe size of the button is defined by both the pixmap and the label.");
            int button_kind = 1;
            for (int col = 0; col < 5 && button_kind < SP_MAX_BUTTON; col++) {
                CGroup *columnGroup = new CGroup("", 10, SP_ALIGN_LEFT); // Reserving some space on the page
                for (int row = 0; row < 6 && button_kind < SP_MAX_BUTTON; row++) {
                    CButton *btn = new CButton((CButtonKind) button_kind, SP_ALIGN_TOP);
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
            Fl_Group *t = tabs->newScroll("Data controls 1", true);
            t->labeltype(FL_ENGRAVED_LABEL);

            CBox *box =
                    new CBox(
                            "All the data controls can be connected to the dataset\nto read or save data. It's done automatically\n if they are created in CDialog window.");
            box->flags(0); // blocking FGE_USEPARENTCOLOR - default value
            box->color(t->color());

            CHtmlBox *htmlBox = new CHtmlBox("HTML output:");
            std::string htmlTxt = " This is a <b>bold</b> <i>italic</i> read-only text in HTML. It is very useful sometimes. ";
            htmlTxt = htmlTxt + htmlTxt;
            htmlTxt = htmlTxt + htmlTxt;
            htmlBox->data(htmlTxt);

            CMemoInput *memoInput = new CMemoInput("Memo input:", 100);
            memoInput->data("This is multiline text input.\n"
                            "It can be bound with ODBC data for TEXT fields \n"
                            "(text of unlimited length).");
            memoInput->callback(general_cb);

            CRadioButtons *radioButtons = new CRadioButtons("Radio Buttons:");
            radioButtons->buttons(Strings("Red|Blue|Green|Unknown", "|"));
            radioButtons->callback(general_cb);

            CCheckButtons *checkBoxList = new CCheckButtons("Check Buttons:");
            checkBoxList->buttons(Strings("first,second,third,*", ","));
            checkBoxList->callback(general_cb);

            CInput *textInput = new CInput("Text Input:");
            textInput->data("some text");
            textInput->callback(general_cb);

            CIntegerInput *integerInput = new CIntegerInput("Integer Input:");
            integerInput->data(12345);
            integerInput->callback(general_cb);

            CDateIntervalInput *dateIntervalInput = new CDateIntervalInput("Date Interval Input:");
            dateIntervalInput->beginOfInterval(DateTime::Now());
            dateIntervalInput->endOfInterval(DateTime::Now());
            dateIntervalInput->callback(general_cb);

            CPhoneNumberInput *phoneNumberInput = new CPhoneNumberInput("Phone Number Input:");
            phoneNumberInput->data("(415)-123-4567");
            phoneNumberInput->callback(general_cb);

            CFloatInput *floatInput = new CFloatInput("Float Number Input:");
            floatInput->data(1.2345);
            floatInput->callback(general_cb);

            CPasswordInput *passwordInput = new CPasswordInput("Password Input:");
            passwordInput->data("password");
            passwordInput->callback(general_cb);

            // If the conversion from string to date doesn't work
            // on you system - you can use the correct date/time format
            // or use the DateTime to construct time
            CDateInput *dateInput = new CDateInput("Date Input:");
            dateInput->data("10/02/2002");
            dateInput->callback(general_cb);

            CTimeInput *timeInput = new CTimeInput("Time Input:");
            timeInput->data("10:25AM");
            timeInput->callback(general_cb);

            DateTimeInput *dateTimeInput = new DateTimeInput("Date and Time Input:");
            dateTimeInput->data("10/02/2002 10:25AM");
            dateTimeInput->callback(general_cb);
        }

        {   // It will be many widgets on this page, so creating a scroll
            Fl_Group *t = tabs->newScroll("Data controls 2", true);
            t->labeltype(FL_ENGRAVED_LABEL);

            CBox *box = new CBox(
                    "Some of the data controls have list of values\n that can be filled directly or from the database.");
            box->flags(0); // blocking FGE_USEPARENTCOLOR - default value
            box->color(t->color());

            // The example of filling in the combo box without data connection
            CComboBox *comboBox1 = new CComboBox("Combo box:");
            CColumnList columns;
            columns.push_back(CColumn("name", VAR_STRING, 50));
            columns.push_back(CColumn("phone", VAR_STRING, 120));
            columns.push_back(CColumn("city", VAR_STRING));
            comboBox1->columns(columns);

            comboBox1->buttons(SP_BROWSE_BUTTON | SP_ADD_BUTTON | SP_EDIT_BUTTON | SP_DELETE_BUTTON);

            comboBox1->callback(general_cb);

            comboBox1->addRow(sl1);
            comboBox1->addRow(sl2);
            comboBox1->addRow(sl3);
            comboBox1->addRow(sl4);
            comboBox1->addRow(sl5);
            comboBox1->addRow(sl6);

            // The example of filling in the combo box from the database.
            // Please, notice (1): It only works if you have ODBC support in SPDB/
            // Please, notice (2): the key field column of query is not shown.
            CComboBox *comboBox2 = new CComboBox("DB Combo box:");
            comboBox2->callback(general_cb);

#ifdef ODBC_DEMO

            comboBox2->database(&db);
            comboBox2->sql(("SELECT comp_id, comp_name as company, comp_employees FROM " + tableName).c_str());
            comboBox2->keyField("comp_id");
            try {
                comboBox2->refreshData();
            } catch (exception& e) {
                printf("Exception: %s\n", e.what());
            }
#else
            comboBox2->addColumn("company",VAR_STRING,70);
            comboBox2->addColumn("comp_employees", VAR_INT, 70);
            comboBox2->addRow(1, "Joy Inc.", "123");
            comboBox2->addRow(2, "Red Cap Inc.", "1234");
#endif

            CListView *listView1 = new CListView("List View 1:", 150);
            listView1->columns(columns);
            listView1->addRow(sl1);
            listView1->addRow(sl2);
            listView1->addRow(sl3);
            listView1->addRow(sl4);
            listView1->addRow(sl5);
            listView1->addRow(sl6);
            listView1->callback(general_cb);

            CDBListView *listView2 = new CDBListView("List View 2:", 150);
            listView2->callback(general_cb);

#ifdef ODBC_DEMO

            listView2->database(&db);
            listView2->sql("SELECT comp_id, comp_name as company, comp_employees FROM companies");
            listView2->keyField("comp_id");
            try {
                listView2->refreshData();
            } catch (exception& e) {
                printf("Exception: %s\n", e.what());
            }
#endif
        }

        {
            CGroup *group = (CGroup *) tabs->newPage("CMemoInput class");
            group->box(FL_THIN_DOWN_BOX);

            CMemoInput *memoInput = new CMemoInput("Text Editor:", 10, SP_ALIGN_CLIENT);
            memoInput->data("This is a plain text editor that uses FL_Multiline_Input. By default, it sets wrap(true).");
            memoInput->callback(general_cb);
        }

#ifdef ODBC_DEMO
        {
            CGroup *group = (CGroup *) tabs->newPage("CDialog class");
            group->box(FL_THIN_DOWN_BOX);

            CGroup *grp = new CGroup("", 150);
            CButton *b = new CButton("Click me", SP_ALIGN_LEFT);
            b->callback((Fl_Callback*) cb_show_dialog);
            dataDialog = new CDataDialog(&db);
            grp->end();

            group->end();
        }
#endif

        {
            CGroup *group = (CGroup *) tabs->newPage("Nested Tabs");
            group->box(FL_THIN_DOWN_BOX);

            CTabs *nestedTabs = new CTabs("", 10, SP_ALIGN_CLIENT);
            nestedTabs->selection_color(15);

            {
                CGroup *group = (CGroup *) nestedTabs->newPage("Nested Tab 1", true);
                group->box(FL_THIN_DOWN_BOX);
                new CButton("Button 1", SP_ALIGN_LEFT);
            }

            {
                CGroup *group = (CGroup *) nestedTabs->newPage("Nested Tab 2", true);
                group->box(FL_THIN_DOWN_BOX);
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

    CButton *button = new CButton(SP_EXIT_BUTTON, SP_ALIGN_TOP);
    button->callback((Fl_Callback*) cb_OK);

    buttonGroup.end();

    w.end();
    w.resizable(w);

    w.show(argc, argv);
    return Fl::run();
}
