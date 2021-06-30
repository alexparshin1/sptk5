/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/gui/CFileDialog.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief File Save Window.
 *
 * Implements the dialog to save a file.
 * @see CDialog, CFileDialog, CFileOpenDialog
 */
class SP_EXPORT CFileSaveDialog
    : public CFileDialog
{
protected:
    /**
     * @brief 'Ok' reaction
     *
     * Method okPressed() overloads the default CDialog reaction on pressing
     * 'Save' (former 'Ok') button.
     * @returns success (true) if the file could be created or opened for
     * writing
     */
    bool okPressed() override;

public:

    /**
     * @brief Default constructor
     *
     * @param caption window caption
     */
    explicit CFileSaveDialog(const std::string& caption = "Save File");

    /**
     * @brief Destructor
     */
    ~CFileSaveDialog() override = default;
};
/**
 * @}
 */
}
