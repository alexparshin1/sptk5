/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ProgressPage.cpp - installation progress page          ║
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

#include "ProgressPage.h"

#include <sptk5/gui/CHtmlBox.h>

using namespace std;
using namespace sptk;

ProgressPage::ProgressPage(InstallerConfig& config)
    : WizardPage(config, "Progress", false)
{
}

void ProgressPage::build()
{
    auto* label = new CHtmlBox("", 30, CLayoutAlign::TOP);
    label->data("<h3>Installing...</h3>");

    m_progressBar = new CProgressBar("Progress:", 25, CLayoutAlign::TOP);
    m_progressBar->minimum(0);
    m_progressBar->maximum(100);
    m_progressBar->data(Variant(0.0f));

    m_logView = new CListView("Installation Log:", 10, CLayoutAlign::CLIENT);
    m_logView->addColumn(CColumn("Message", VariantDataType::VAR_STRING, 500));
    m_logView->showGrid(true);
}

void ProgressPage::addLogLine(const String& text)
{
    m_logView->addRow(0, Strings {text});
    m_logView->redraw();
}

void ProgressPage::progress(float value)
{
    m_progressBar->data(Variant(static_cast<double>(value)));
    m_progressBar->redraw();
}
