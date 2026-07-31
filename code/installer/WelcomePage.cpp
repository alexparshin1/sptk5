/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WelcomePage.cpp - wizard welcome page                  ║
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

#include "WelcomePage.h"

#include <sptk5/gui/CHtmlBox.h>

using namespace std;
using namespace sptk;

WelcomePage::WelcomePage(InstallerConfig& config)
    : WizardPage(config, "Welcome")
{
}

void WelcomePage::build()
{
    auto*  welcomeHtml = new CHtmlBox("", 10, CLayoutAlign::CLIENT);
    String html = "<h2>Welcome to " + m_config.application + " Setup</h2>"
                                                             "<p>" +
                  m_config.description + "</p>"
                                         "<p>This wizard will guide you through the installation of <b>" +
                  m_config.application + " " + m_config.version + "</b>.</p>"
                                                                  "<p>Click Next to continue.</p>";
    welcomeHtml->data(html);
}
