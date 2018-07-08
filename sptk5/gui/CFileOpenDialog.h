/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFileOpenDialog.h - description                        ║
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

#ifndef __CFILEOPENDIALOG_H__
#define __CFILEOPENDIALOG_H__

#include <sptk5/gui/CFileDialog.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief File Open Window.
 *
 * Implements the dialog to open an existing file.
 * @see CDialog, CFileDialog, CFileSaveDialog
 */
class CFileOpenDialog : public CFileDialog {
protected:
    /**
     * @brief Custom okPressed() method
     * Method okPressed() overloads the default CDialog reaction on pressing
     * 'Open' (former 'Ok') button.
     * @returns success (true) if the file exists and could be opened for reading
     */
    virtual bool okPressed();

public:
    /**
     * @brief Default constructor
     * @param caption window caption
     */
    explicit CFileOpenDialog(const std::string& caption = "Open File");

    /**
     * @brief Destructor
     */
    virtual ~CFileOpenDialog() {}
}
;

/**
 * @}
 */
}
#endif
