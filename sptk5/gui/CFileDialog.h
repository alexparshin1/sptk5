/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFileDialog.h - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CFILEDIALOG_H__
#define __CFILEDIALOG_H__

#include <sptk5/gui/CDialog.h>
#include <sptk5/DirectoryDS.h>
#include <sptk5/gui/CListView.h>
#include <sptk5/gui/CDataControl.h>
#include <sptk5/gui/CComboBox.h>
#include <sptk5/Strings.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Base class for CFileOpenDialog and CFileSaveDialog.
 *
 * Actually implements most of these classes functionality.
 * Can't be used by itself, only as a base class.
 */
class CFileDialog : public CDialog {
protected:
    /**
     * Directory Dataset
     */
    DirectoryDS   m_directory;

    /**
     * File list in the directory
     */
    CListView     *m_directoryView;

    /**
     * The combo with the list of the lookup directories
     */
    CComboBox     *m_lookInCombo;

    /**
     * The file name input box
     */
    CInput        *m_fileNameInput;

    /**
     * The file mask (pattern) selection combo
     */
    CComboBox     *m_patternCombo;


    /**
     * @brief Removes a trailing slash in the directory name
     *
     * @param dirname std::string, a directory name
     */
    static std::string removeTrailingSlash(std::string dirname);

    /**
     * Internal create folder button callback
     */
    static void new_folder_cb(Fl_Widget *,void *);

    /**
     * Internal home button callback
     */
    static void home_cb(Fl_Widget *,void *);

    /**
     * Internal go up button callback
     */
    static void up_cb(Fl_Widget *,void *);

    /**
     * Internal directory double-clicked callback
     */
    static void dirview_cb(Fl_Widget *w,void *);

    /**
     * Internal look-in combo callback
     */
    static void lookin_cb(Fl_Widget *w,void *);

    /**
     * Internal pattern combo callback
     */
    static void pattern_cb(Fl_Widget *w,void *);

    /**
     * Creates a new folder. The folder name is asked interactively.
     */
    void createFolder();
protected:

    /**
     * Default slash character, '\\' on Windows and '/' on Unux.
     */
    static const char slashChar;

    /**
     * Default slash character string, "\\" on Windows and "/" on Unux.
     */
    static const char slashStr[];

    /**
     * Constructor
     * @param label std::string, window caption
     * @param saveMode bool, true for File Save Dialog and false for File Open Dialog.
     */
    CFileDialog(const std::string& label, bool saveMode);

public:

    /**
     * Returns the dialog's current directory
     */
    std::string directory() const {
        return m_directory.directory();
    }

    /**
     * Sets the dialog's current directory
     */
    void directory(const String& p);

    /**
     * Removes all the defined file name paterns
     */
    void clearPatterns();

    /**
     * Sets the file name patern in pattern combo box
     * @param patternName std::string, the name of the pattern as defined in call of addPattern()
     */
    void setPattern(std::string patternName);

    /**
     * Adds the file name patern to the pattern combo box
     * @param patternName std::string, the name of the pattern, like "All files"
     * @param pattern std::string, one or more the patterns, separated with semicolon like "*.cpp;*.cxx"
     */
    void addPattern(std::string patternName,std::string pattern);

    /**
     * Returns currently selected pattern
     */
    std::string pattern() const;

    /**
     * Sets the new directory, if parameter dir is defined, and reloads the file list.
     * @param dirName std::string, the name of the directory
     */
    void refreshDirectory(std::string dirName="");

    /**
     * Returns the list of currently selected files without the full path, as a simicolon-separated string.
     * @returns The list of the selected file names.
     */
    std::string fileName() const {
        return m_fileNameInput->data();
    }

    /**
     * Returns the list of currently selected files with the full path, as a simicolon-separated string
     * @returns The list of the selected full file names.
     */
    String fullFileName() const;

    /**
     * Set the currently selected file name in edit entry
     * @param fileName std::string, the name of the file.
     */
    void fileName(std::string fileName);

    /**
     * Initializes the dialog and shows it in the modal mode
     */
    bool execute();
};
/**
 * @}
 */
}
#endif
