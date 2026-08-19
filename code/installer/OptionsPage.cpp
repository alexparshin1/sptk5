/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       OptionsPage.cpp - installation options page            ║
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

#include "OptionsPage.h"

#include <sptk5/gui/CHtmlBox.h>

using namespace std;
using namespace sptk;

OptionsPage::OptionsPage(InstallerConfig& config)
    : WizardPage(config, "Options")
{
}

void OptionsPage::build()
{
    auto* label = new CHtmlBox("", 50, CLayoutAlign::TOP);
    label->data("<h3>Installation Options</h3>"
                "<p>Select the components you want to install.</p>");

    m_checkButtons = new CCheckButtons("Options:", 20, CLayoutAlign::CLIENT);
    Strings buttonLabels;
    for (const auto& opt: m_config.options)
        buttonLabels.push_back(opt.name);
    m_checkButtons->buttons(buttonLabels);

    // Select all by default
    String allSelected;
    for (size_t i = 0; i < m_config.options.size(); i++)
    {
        if (i > 0)
            allSelected += "|";
        allSelected += m_config.options[i].name;
    }
    m_checkButtons->data(Variant(allSelected));
}

bool OptionsPage::onLeave()
{
    String selected = m_checkButtons->data().getString();
    m_config.selectedOptions = selected.empty() ? Strings() : Strings(selected, "|");
    return true;
}
