/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ProgressPage.h - installation progress page            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include "WizardPage.h"

#include <sptk5/gui/CListView.h>
#include <sptk5/gui/CProgressBar.h>

/**
 * @brief Progress bar and log of the running installation.
 *
 * The installation runs in a worker thread, so both methods below must only be
 * called from the GUI thread, from the wizard's Fl::awake() handler.
 */
class ProgressPage
    : public WizardPage
{
public:
    explicit ProgressPage(InstallerConfig& config);

    /**
     * @brief Appends a line to the installation log
     */
    void addLogLine(const sptk::String& text);

    /**
     * @brief Moves the progress bar
     * @param value             Progress, 0 to 100
     */
    void progress(float value);

protected:
    void build() override;

private:
    sptk::CProgressBar* m_progressBar {nullptr};
    sptk::CListView*    m_logView {nullptr};
};
