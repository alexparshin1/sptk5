/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CompletedPage.cpp - installation result page           ║
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

#include "CompletedPage.h"

using namespace std;
using namespace sptk;

CompletedPage::CompletedPage(InstallerConfig& config)
    : WizardPage(config, "Completed")
{
}

void CompletedPage::build()
{
    m_completedHtml = new CHtmlBox("", 10, CLayoutAlign::CLIENT);
    m_completedHtml->data("<h3>Installation Complete</h3>"
                          "<p>Click <b>Finish</b> to exit the wizard.</p>");
}

void CompletedPage::showResult(bool success)
{
    String html;
    if (success)
        html = "<h3>Installation Complete</h3>"
               "<p><b>" +
               m_config.application + " " + m_config.version + "</b> has been successfully installed.</p>"
                                                               "<p>Click <b>Finish</b> to exit the wizard.</p>";
    else
        html = "<h3>Installation Failed</h3>"
               "<p>There were errors during installation. "
               "Please check the installation log for details.</p>"
               "<p>Click <b>Finish</b> to exit the wizard.</p>";

    m_completedHtml->data(Variant(html));
}
