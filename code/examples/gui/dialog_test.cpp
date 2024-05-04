/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       dialog_test.cpp - description                          ║
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

using namespace std;
using namespace sptk;

void example_dialog_cb(Fl_Widget* button, void*)
{
    auto* btn = dynamic_cast<CButton*>(button);
    if (btn)
    {
        spInformation(String(btn->label()) +
                      " button was pressed.\n"
                      "It is different from <B>Ok</B> and <B>Cancel</B> buttons "
                      "that are processed by default.");
    }
}

void exit_cb(Fl_Widget* button, void*)
{
    button->window()->hide();
}

class CExampleDialog
    : public CDialog
{
    xdoc::Document m_state;
    String m_stateFileName {"dialog_test.xml"};

public:
    CExampleDialog()
        : CDialog(300, 260, "Example Dialog")
    {
        CDialog::newPage("Company", true);

        auto* inp = new CInput("Company Name:");
        inp->fieldName("company_name");

        inp = new CIntegerInput("Company Size:");
        inp->fieldName("company_size");

        auto* comboBox = new CComboBox("Business Type:");
        comboBox->fieldName("business_type");
        comboBox->addColumn("business type", VariantDataType::VAR_STRING, 120);
        comboBox->addRow(1, Strings("Agriculture", "|"));
        comboBox->addRow(2, Strings("Education", "|"));
        comboBox->addRow(3, Strings("Hardware", "|"));
        comboBox->addRow(4, Strings("Software", "|"));

        auto* dateInput = new CDateInput("Established");
        dateInput->fieldName("established");

        CDialog::newScroll("Contact Info", true);

        inp = new CInput("First Name:");
        inp->fieldName("first_name");

        inp = new CInput("Last Name:");
        inp->fieldName("last_name");

        inp = new CFloatInput("Age:");
        inp->fieldName("age");

        inp = new CMemoInput("Notes:", 100);
        inp->fieldName("notes");

        addExtraButton(CButtonKind::SEND_BUTTON, "E-mail", example_dialog_cb);

        end();

        CDialog::relayout();
    }

    void loadState()
    {
        try
        {
            /// Try to load the prior values of the dialog controls.
            /// If the XML file doesn't exist yet - this will throw an exception that we trap.
            Buffer buffer;
            buffer.loadFromFile(m_stateFileName.c_str());
            m_state.load(buffer);

            /// If the XML file exists, try to load data into the dialog
            load(m_state.root());
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl);
        }
    }

    void saveState()
    {
        try
        {
            /// Save data from dialog controls into XML file
            save(m_state.root());

            /// Save the XML file.
            Buffer buffer;
            m_state.exportTo(xdoc::DataFormat::XML, buffer, false);
            buffer.saveToFile(m_stateFileName.c_str());
        }
        catch (const Exception& e)
        {
            spError("Can't save dialog data: " + string(e.what()));
        }
    }
};


void dialog_cb(Fl_Widget*, void*)
{
    CExampleDialog dialog;

    /// Setting the default values for the dialog controls.
    /// The controls are addressed by their field names.
    /// The data is converted automatically based on the data type.
    try
    {
        dialog["company_name"] = "Tiny Soft";
        dialog["company_size"] = 2;
        dialog["business_type"] = "Software";
        dialog["established"] = DateTime::Now();
        dialog["first_name"] = "John";
        dialog["last_name"] = "Doe";
        dialog["age"] = 35.2;
        dialog["notes"] = "Heavy duty worker. Family Guy. Good Friend.";
    }
    catch (const Exception& e)
    {
        /// We can get here if the field name is incorrect,
        /// or data conversion isn't possible
        spError(e.what());
    }

    /// This loads the last known state of the dialog,
    /// stored in the XML file
    dialog.loadState();

    if (dialog.showModal())
    {
        COUT(
            (String) dialog["company_name"] << ", has " << (int) dialog["company_size"] << " employees ("
                                            << (String) dialog["business_type"] << "), established "
                                            << (String) dialog["established"] << endl);
    }

    /// This saves the last known state of the dialog
    /// to the XML file
    dialog.saveState();
}

int main(int argc, char* argv[])
{
    try
    {
        // Initialize themes
        const CThemes themes;

        CWindow window(300, 170);

        CHtmlBox textBox;
        textBox.data("<p>This test shows how simple it is"
                     "to create a <i>modal dialog</i> window in <b>SPTK.</b></p>"
                     "<p>The dialog window may have several <i>tabs</i> and every"
                     "widget may have a <i>field name</i> attached to it.</p>");
        textBox.color(FL_GRAY);

        CButton exitButton(CButtonKind::EXIT_BUTTON);
        exitButton.callback(exit_cb);

        CButton showDialogButton(CButtonKind::EDIT_BUTTON, CLayoutAlign::RIGHT,
                                 "Show Dialog");
        showDialogButton.callback(dialog_cb);

        window.end();
        window.show();

        if (argc > 1)
        {
            CThemes::set(argv[1]);
        }

        return Fl::run();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
        return 1;
    }
}
