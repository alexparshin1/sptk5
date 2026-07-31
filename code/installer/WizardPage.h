/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WizardPage.h - wizard page base class                  ║
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

#include "InstallerConfig.h"

#include <sptk5/gui/CTabs.h>

/**
 * @brief Base class for the installer wizard pages.
 *
 * A page creates and owns its own widgets in build(), refreshes itself in
 * onEnter() right before it becomes visible, and validates its data in
 * onLeave() when the user moves forward.
 */
class WizardPage
{
public:
    /**
     * @brief Constructor
     * @param config            Installer configuration, shared by all pages
     * @param title             Page title, shown when the tab bar is visible
     * @param scrollable        Create the page as a scroll area
     */
    WizardPage(InstallerConfig& config, sptk::String title, bool scrollable = true);

    virtual ~WizardPage() = default;

    WizardPage(const WizardPage&) = delete;
    WizardPage& operator=(const WizardPage&) = delete;

    /**
     * @brief Adds the page to the wizard tabs and populates it with widgets
     * @param tabs              Wizard tabs
     */
    void create(sptk::CTabs& tabs);

    /**
     * @brief Refreshes the page content right before the page becomes visible
     */
    virtual void onEnter();

    /**
     * @brief Validates the page data.
     * @returns false to keep the wizard on this page
     */
    virtual bool onLeave();

protected:
    /**
     * @brief Creates the page widgets, called with the page group open
     */
    virtual void build() = 0;

    InstallerConfig& m_config;

private:
    sptk::String m_title;
    bool         m_scrollable;
};
