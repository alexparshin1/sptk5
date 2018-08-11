/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFileDialog.cpp - description                          ║
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

#include <sys/stat.h>

#include <FL/fl_ask.H>

#include <sptk5/gui/CFileDialog.h>
#include <sptk5/Registry.h>

#ifdef WIN32

#include <direct.h>
const char sptk::CFileDialog::slashChar  = '\\';
const char sptk::CFileDialog::slashStr[] = "\\";
#else
const char sptk::CFileDialog::slashChar  = '/';
const char sptk::CFileDialog::slashStr[] = "/";
#endif

using namespace std;
using namespace sptk;

string CFileDialog::removeTrailingSlash(std::string dirname)
{
    size_t dlen = dirname.length();

    if (dlen && (dirname[dlen - 1] == '/' || dirname[dlen - 1] == '\\'))
        return dirname.substr(0, dlen - 1);

    return dirname;
}

void CFileDialog::new_folder_cb(Fl_Widget* w, void*)
{
    auto fileDialog = (CFileDialog*) w->window();
    fileDialog->createFolder();
}

void CFileDialog::home_cb(Fl_Widget* w, void*)
{
    auto fileDialog = (CFileDialog*) w->window();
    String homeDirectory = Registry::homeDirectory();
    fileDialog->directory(homeDirectory);
    fileDialog->refreshDirectory();
}

void CFileDialog::up_cb(Fl_Widget* w, void*)
{
    auto fileDialog = (CFileDialog*) w->window();
    fileDialog->directory(fileDialog->directory() + "..");
    fileDialog->refreshDirectory();
}

void CFileDialog::dirview_cb(Fl_Widget* w, void*)
{
    bool directoryClicked = false;

    auto fileDialog = (CFileDialog*) w->window();
    auto listView = (CListView*) w;

    if (listView->selectedRow() == nullptr)
        return;

    CPackedStrings& row = *listView->selectedRow();

    if (strncmp(row[3], "Directory", 9) == 0)
        directoryClicked = true;

    switch (listView->eventType()) {
    case CE_MOUSE_CLICK:
        if (!directoryClicked) {
            Strings fileNames;
            const CSelection& selection = fileDialog->m_directoryView->selection();

            for (unsigned i = 0; i < selection.size(); i++) {
                CPackedStrings& srow = selection[i];

                if (strncmp(row[3], "Directory", 9) != 0) {
                    fileNames.push_back(srow[1]);
                }
            }

            fileDialog->m_fileNameInput->data(fileNames.asString("; "));
        }

        break;

    case CE_MOUSE_DOUBLE_CLICK: {
            if (directoryClicked) {
                String fullPath = fileDialog->m_directory.directory() + slashStr + row[1];
                fileDialog->directory(fullPath.replace("[\\/\\\\]{2}", slashStr));
                fileDialog->refreshDirectory();
            } else {
                fileDialog->m_fileNameInput->data(row[1]);
                fileDialog->m_okButton->do_callback();
            }
        }
        break;

    default
            :
        break;
    }
}

void CFileDialog::lookin_cb(Fl_Widget* w, void*)
{
    auto fileDialog = (CFileDialog*) w->window();
    auto comboBox = (CComboBox*) w;

    if (comboBox->eventType() != CE_DATA_CHANGED)
        return;

    if (comboBox->selectedRow() == nullptr)
        return;

    CPackedStrings& ps = *comboBox->selectedRow();

    if (fileDialog->m_directory.directory() != ps[0]) {
        fileDialog->refreshDirectory(ps[0]);
    }
}

void CFileDialog::pattern_cb(Fl_Widget* w, void*)
{
    auto comboBox = (CComboBox*) w;

    if (comboBox->eventType() != CE_DATA_CHANGED)
        return;

    auto fileDialog = (CFileDialog*) w->window();

    if (fileDialog->pattern() != fileDialog->m_directory.pattern())
        fileDialog->refreshDirectory();
}

CFileDialog::CFileDialog(const string& label, bool saveMode)
    : CDialog(450, 400, label.c_str())
{
    CButton* btn;

    auto grp = new CGroup;
    m_lookInCombo = new CComboBox("Look in:", 10, SP_ALIGN_CLIENT);
    m_lookInCombo->labelWidth(60);
    m_lookInCombo->addColumn("Path", VAR_STRING, 250);

    if (saveMode) {
        btn = new CButton("", SP_ALIGN_RIGHT);
        btn->buttonImage(CThemes::getIconImage("fd_new_folder", IS_LARGE_ICON));
        btn->callback(CFileDialog::new_folder_cb);
    }

    /*
       btn = new CButton("",SP_ALIGN_RIGHT);
       btn->buttonImage(&pixmap_favorites);
    */
    btn = new CButton("", SP_ALIGN_RIGHT);
    btn->buttonImage(CThemes::getIconImage("fd_level_up", IS_LARGE_ICON));
    btn->callback(CFileDialog::up_cb);

    btn = new CButton("", SP_ALIGN_RIGHT);
    btn->buttonImage(CThemes::getIconImage("fd_home_page", IS_LARGE_ICON));
    btn->callback(CFileDialog::home_cb);

    grp->end();

    m_patternCombo = new CComboBox("Files of type:", 10, SP_ALIGN_BOTTOM);
    m_patternCombo->addColumn("file type", VAR_STRING, 150);
    m_patternCombo->addColumn("pattern", VAR_STRING, 100);
    m_patternCombo->addRow(1, Strings("All Files|*.*", "|"));
    m_patternCombo->data(1);
    m_patternCombo->callback(pattern_cb);

    m_fileNameInput = new CInput("File name:", 10, SP_ALIGN_BOTTOM);

    m_directoryView = new CListView("", 200, SP_ALIGN_CLIENT);
    m_directoryView->callback(CFileDialog::dirview_cb);
    end();
    directory(".");
}

bool CFileDialog::execute()
{
    m_directory.showPolicy(DDS_HIDE_DOT_FILES);

    if (m_patternCombo->selectedRow()) {
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
    CInput folderNameInput("Folder Name:");
    folderNameInput.labelWidth(90);

    if (dialog.showModal()) {
        String folderName = m_directory.directory() + slashStr + folderNameInput.data().asString();
        folderName = folderName.replace("[\\/\\\\]{2}", slashStr);
#ifdef WIN32
        int rc = mkdir(folderName.c_str());
#else
        int rc = mkdir(folderName.c_str(), S_IRWXU);
#endif

        if (rc == 0) {
            directory(folderName);
            refreshDirectory();
        } else {
            fl_alert("%s", ("Can't create directory " + folderName).c_str());
        }
    }
}

#ifdef WIN32

static void makeDriveList(Strings& driveList)
{
    char buffer[128];

    driveList.clear();
    int nLen = GetLogicalDriveStrings(128, buffer);
    int nDrives = nLen / 4;

    for (int d = 0; d < nDrives; d++)
        driveList.push_back(upperCase(buffer + (d * 4)));
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

    for (unsigned d = 0; d < driveList.size(); d++) {
        pseudoID++;
        m_lookInCombo->addRow(Strings(driveList[d], "|"), pseudoID);
    }

#endif

    m_directory.directory(p);

    Strings pathItems(m_directory.directory().c_str(), slashStr);
    string incrementalPath;

    for (unsigned i = 0; i < pathItems.size(); i++) {
        incrementalPath += pathItems[i];

        if (i == 0)
            incrementalPath += slashStr;

#ifdef WIN32

        if (i)
#endif
        {
            pseudoID++;
            m_lookInCombo->addRow(pseudoID, Strings(incrementalPath, "|"));
        }

        if (i != 0)
            incrementalPath += slashStr;
    }

    m_lookInCombo->callback(lookin_cb);
    string dirName = m_directory.directory();
    m_lookInCombo->data(dirName);

    int estimatedColumnWidth = (int) m_lookInCombo->textSize() * (int) incrementalPath.length() * 2 / 3;
    int minColWidth = 280;

    if (estimatedColumnWidth < minColWidth)
        estimatedColumnWidth = minColWidth;

    m_lookInCombo->columns() [0].width((int16_t) estimatedColumnWidth);

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

void CFileDialog::setPattern(string patternName)
{
    m_patternCombo->sortColumn(0);
    m_patternCombo->findString(patternName);
}

void CFileDialog::addPattern(string patternName, string pattern)
{
    m_patternCombo->addRow(0, Strings(patternName + "|" + pattern, "|"));
}

string CFileDialog::pattern() const
{
    if (m_patternCombo->selectedRow()) {
        CPackedStrings& ps = *m_patternCombo->selectedRow();
        return ps[1];
    }

    return "*.*";
}

void CFileDialog::refreshDirectory(string dir)
{
    if (dir.length())
        m_directory.directory(dir);

    m_directory.pattern(pattern());
    m_directoryView->fill(m_directory, "N/A");
    //m_fileNameInput->data("");
}

void CFileDialog::fileName(string fn)
{
    m_fileNameInput->data(fn);
}

String CFileDialog::fullFileName() const
{
    String fileNamesStr = m_fileNameInput->data();
    Strings fileNames(fileNamesStr, ";");

    for (auto& fileName : fileNames) {
        String fname = m_directory.directory() + slashStr + fileName;
        fileName = trim(fname.replace("[\\/\\\\]{2}", slashStr));
    }

    return fileNames.asString(";");
}
