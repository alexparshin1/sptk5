/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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

#include <FL/fl_ask.H>
#include <sptk5/HomeDirectory.h>
#include <sptk5/gui/CFileDialog.h>

#ifdef WIN32

#include <direct.h>
const char sptk::CFileDialog::slashChar = '\\';
const char sptk::CFileDialog::slashStr[] = "\\";
#else
const char sptk::CFileDialog::slashChar = '/';
const char sptk::CFileDialog::slashStr[] = "/";
#endif

using namespace std;
using namespace sptk;

String CFileDialog::removeTrailingSlash(const String& dirname)
{
    const size_t dlen = dirname.length();

    if (dlen && (dirname[dlen - 1] == '/' || dirname[dlen - 1] == '\\'))
    {
        return dirname.substr(0, dlen - 1);
    }

    return dirname;
}

void CFileDialog::new_folder_cb(Fl_Widget* dialog, void*)
{
    auto* fileDialog = (CFileDialog*) dialog->window();
    fileDialog->createFolder();
}

void CFileDialog::home_cb(Fl_Widget* dialog, void*)
{
    auto*  fileDialog = (CFileDialog*) dialog->window();
    String homeDirectory = HomeDirectory::location();
    fileDialog->directory(homeDirectory);
    fileDialog->refreshDirectory();
}

void CFileDialog::up_cb(Fl_Widget* dialog, void*)
{
    auto* fileDialog = (CFileDialog*) dialog->window();
    fileDialog->directory(fileDialog->directory() + "..");
    fileDialog->refreshDirectory();
}

void CFileDialog::dirview_cb(Fl_Widget* dialog, void*)
{
    bool directoryClicked = false;

    auto* fileDialog = (CFileDialog*) dialog->window();
    auto* listView = (CListView*) dialog;

    if (listView->selectedRow() == nullptr)
    {
        return;
    }

    CPackedStrings& row = *listView->selectedRow();

    if (strncmp(row[3], "Directory", 9) == 0)
    {
        directoryClicked = true;
    }

    switch (listView->eventType())
    {
        case CEvent::MOUSE_CLICK:
            if (!directoryClicked)
            {
                Strings           fileNames;
                const CSelection& selection = fileDialog->m_directoryView->selection();

                for (unsigned i = 0; i < selection.size(); i++)
                {
                    if (strncmp(row[3], "Directory", 9) != 0)
                    {
                        CPackedStrings& srow = selection[i];
                        fileNames.push_back(srow[1]);
                    }
                }

                fileDialog->m_fileNameInput->data(fileNames.join("; "));
            }

            break;

        case CEvent::MOUSE_DOUBLE_CLICK: {
            if (directoryClicked)
            {
                String fullPath = fileDialog->m_directory.directory() + slashStr + row[1];
                fileDialog->directory(fullPath.replace("[\\/\\\\]{2}", slashStr));
                fileDialog->refreshDirectory();
            }
            else
            {
                fileDialog->m_fileNameInput->data(row[1]);
                fileDialog->m_okButton->do_callback();
            }
        }
        break;

        default:
            break;
    }
}

void CFileDialog::lookin_cb(Fl_Widget* dialog, void*)
{
    auto* fileDialog = (CFileDialog*) dialog->window();
    auto* comboBox = (CComboBox*) dialog;

    if (comboBox->eventType() != CEvent::DATA_CHANGED)
    {
        return;
    }

    if (comboBox->selectedRow() == nullptr)
    {
        return;
    }

    CPackedStrings& ps = *comboBox->selectedRow();

    if (fileDialog->m_directory.directory() != ps[0])
    {
        fileDialog->refreshDirectory(ps[0]);
    }
}

void CFileDialog::pattern_cb(Fl_Widget* combobox, void*)
{
    auto* comboBox = (CComboBox*) combobox;

    if (comboBox->eventType() != CEvent::DATA_CHANGED)
    {
        return;
    }

    auto* fileDialog = (CFileDialog*) combobox->window();
    fileDialog->refreshDirectory();
}

CFileDialog::CFileDialog(const String& label, bool saveMode)
    : CDialog(450, 400, label.c_str())
{
    CButton* btn;

    auto* grp = new CGroup;
    m_lookInCombo = new CComboBox("Look in:", 10, CLayoutAlign::CLIENT);
    m_lookInCombo->labelWidth(60);
    m_lookInCombo->addColumn("Path", VariantDataType::VAR_STRING, 250);

    if (saveMode)
    {
        btn = new CButton("", CLayoutAlign::RIGHT);
        btn->buttonImage(CThemes::getIconImage("fd_new_folder", CIconSize::IS_LARGE_ICON));
        btn->callback(CFileDialog::new_folder_cb);
    }

    btn = new CButton("", CLayoutAlign::RIGHT);
    btn->buttonImage(CThemes::getIconImage("fd_level_up", CIconSize::IS_LARGE_ICON));
    btn->callback(CFileDialog::up_cb);

    btn = new CButton("", CLayoutAlign::RIGHT);
    btn->buttonImage(CThemes::getIconImage("fd_home_page", CIconSize::IS_LARGE_ICON));
    btn->callback(CFileDialog::home_cb);

    grp->end();

    m_patternCombo = new CComboBox("Files of type:", 10, CLayoutAlign::BOTTOM);
    m_patternCombo->addColumn("file type", VariantDataType::VAR_STRING, 150);
    m_patternCombo->addColumn("pattern", VariantDataType::VAR_STRING, 100);
    m_patternCombo->addRow(1, Strings("All Files|*.*", "|"));
    m_patternCombo->data(1);
    m_patternCombo->callback(pattern_cb);

    m_fileNameInput = new CInput("File name:", 10, CLayoutAlign::BOTTOM);

    m_directoryView = new CListView("", 200, CLayoutAlign::CLIENT);
    m_directoryView->callback(CFileDialog::dirview_cb);
    end();
    directory(".");
}

bool CFileDialog::execute()
{
    m_directory.showPolicy(DDS_HIDE_DOT_FILES);

    if (m_patternCombo->selectedRow())
    {
        CPackedStrings& selectedPattern = *m_patternCombo->selectedRow();
        m_directory.pattern(selectedPattern[1]);
    }

    m_directoryView->fill(m_directory, "N/A");
    bool rc = showModal();

    return rc;
}

void CFileDialog::createFolder()
{
    CDialog dialog(350, 85, "Create a New Folder");
    CInput  folderNameInput("Folder Name:");
    folderNameInput.labelWidth(90);

    if (dialog.showModal())
    {
        String folderName = m_directory.directory() + slashStr + folderNameInput.data().asString();
        folderName = folderName.replace("[\\/\\\\]{2}", slashStr);
        try
        {
            filesystem::create_directories(folderName.c_str());
            directory(folderName);
            refreshDirectory();
        }
        catch (const filesystem::filesystem_error& e)
        {
            fl_alert("%s", ("Can't create directory " + folderName + ": " + String(e.what())).c_str());
        }
    }
}

#ifdef WIN32

static void makeDriveList(Strings& driveList)
{
    array<char, 128> buffer;

    driveList.clear();
    int nLen = GetLogicalDriveStrings(128, buffer.data());
    int nDrives = nLen / 4;

    for (int d = 0; d < nDrives; d++)
        driveList.push_back(upperCase(buffer.data() + (d * 4)));
}

#endif

void CFileDialog::directory(const String& p)
{
    m_lookInCombo->callback((Fl_Callback*) nullptr);
    m_lookInCombo->clear();

    int pseudoID = 0;

#ifdef WIN32
    Strings driveList;
    makeDriveList(driveList);

    for (unsigned d = 0; d < driveList.size(); d++)
    {
        pseudoID++;
        m_lookInCombo->addRow(pseudoID, Strings(driveList[d], "|"));
    }
#endif

    m_directory.directory(p);

    Strings pathItems(m_directory.directory().c_str(), slashStr);
    string  incrementalPath;

    for (size_t i = 0; i < pathItems.size(); i++)
    {
        incrementalPath += pathItems[i];

        if (i == 0)
        {
            incrementalPath += slashStr;
        }

#ifdef WIN32
        if (i)
#endif
        {
            pseudoID++;
            m_lookInCombo->addRow(pseudoID, Strings(incrementalPath, "|"));
        }

        if (i != 0)
        {
            incrementalPath += slashStr;
        }
    }

    m_lookInCombo->callback(lookin_cb);
    String dirName = m_directory.directory();
    m_lookInCombo->data(dirName);

    int estimatedColumnWidth = (int) m_lookInCombo->textSize() * (int) incrementalPath.length() * 2 / 3;
    int minColWidth = 280;

    if (estimatedColumnWidth < minColWidth)
    {
        estimatedColumnWidth = minColWidth;
    }

    m_lookInCombo->columns()[0].width((int16_t) estimatedColumnWidth);

    m_lookInCombo->sortColumn(0);

    m_lookInCombo->redraw();
}

void CFileDialog::clearPatterns()
{
    m_patternCombo->callback((Fl_Callback*) nullptr);
    m_patternCombo->clear();
    m_patternCombo->addRow(1, Strings("All Files|*.*", "|"));
    m_patternCombo->callback((Fl_Callback*) pattern_cb);
    m_patternCombo->data(1);
    refreshDirectory();
}

void CFileDialog::setPattern(const String& patternName)
{
    m_patternCombo->sortColumn(0);
    m_patternCombo->findString(patternName);
}

void CFileDialog::addPattern(const String& patternName, const String& pattern)
{
    m_patternCombo->addRow(0, Strings(patternName + "|" + pattern, "|"));
}

String CFileDialog::pattern() const
{
    if (m_patternCombo->selectedRow())
    {
        CPackedStrings& ps = *m_patternCombo->selectedRow();
        return ps[1];
    }

    return "*.*";
}

void CFileDialog::refreshDirectory(const String& dir)
{
    if (dir.length())
    {
        m_directory.directory(dir);
    }

    m_directory.pattern(pattern());
    m_directoryView->fill(m_directory, "N/A");
}

void CFileDialog::fileName(const String& fn)
{
    m_fileNameInput->data(fn);
}

String CFileDialog::fullFileName() const
{
    String  fileNamesStr = m_fileNameInput->data().asString();
    Strings fileNames(fileNamesStr, ";");

    for (auto& fileName: fileNames)
    {
        String fname = m_directory.directory() + slashStr + fileName;
        fileName = trim(fname.replace("[\\/\\\\]{2}", slashStr));
    }

    return fileNames.join(";");
}
