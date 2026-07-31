/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WizardPage.cpp - wizard page base class                ║
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

#include "WizardPage.h"

using namespace std;
using namespace sptk;

WizardPage::WizardPage(InstallerConfig& config, String title, bool scrollable)
    : m_config(config)
    , m_title(move(title))
    , m_scrollable(scrollable)
{
}

void WizardPage::create(CTabs& tabs)
{
    Fl_Group* page = m_scrollable ? tabs.newScroll(m_title.c_str(), false)
                                  : tabs.newPage(m_title.c_str(), false);
    build();
    page->end();
}

void WizardPage::onEnter()
{
}

bool WizardPage::onLeave()
{
    return true;
}
