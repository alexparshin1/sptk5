/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DirectoryPage.h - installation directory page          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include "WizardPage.h"

#include <sptk5/gui/CHtmlBox.h>
#include <sptk5/gui/CInput.h>

/**
 * @brief Installation directory selection.
 *
 * Offers a directory input with a 'Browse' button, reports the state of the
 * chosen path while it is being typed, and stores the accepted directory into
 * the installer configuration.
 */
class DirectoryPage
    : public WizardPage
{
public:
    explicit DirectoryPage(InstallerConfig& config);

    void onEnter() override;
    bool onLeave() override;

protected:
    void build() override;

private:
    /**
     * @brief Picks the installation directory with a directory dialog
     */
    void doBrowse();

    /**
     * @brief Describes the state of the currently entered directory
     */
    void updateStatus();

    static void cb_browse(Fl_Widget*, void* data);
    static void cb_changed(Fl_Widget*, void* data);

    sptk::CInput*   m_dirInput {nullptr};
    sptk::CHtmlBox* m_statusHtml {nullptr};
};
