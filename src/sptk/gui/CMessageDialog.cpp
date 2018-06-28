/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CMessageDialog.cpp - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/sptk.h>

#include <FL/Fl.H>
#include <sptk5/gui/CMessageDialog.h>

#include <cmath>

using namespace std;
using namespace sptk;

CAskDialog::CAskDialog(const char* label, int w)
        : CDialog(w, 100, label)
{
    auto textSize = std::round(Fl::h() / 64.0);
    if (textSize < 14) textSize = 14;

    auto* grp = new CGroup("", 65, SP_ALIGN_LEFT);
    m_imageBox = new Fl_Box(8, 8, 50, 50);
    m_imageBox->box(FL_NO_BOX);
    grp->end();

    m_textBox = new CHtmlBox("", 30, SP_ALIGN_CLIENT);
    m_textBox->labelWidth(0);
    m_textBox->textSize((uchar) textSize);
    m_textBox->color(FL_GRAY);

    m_inputBox = new CInput("", 10, SP_ALIGN_BOTTOM);
    m_inputBox->labelWidth(0);
    m_inputBox->textSize((uchar) textSize);
    m_inputBox->hide();

    layoutGrowMode(LGM_AUTO_GROW);

    end();
}

bool CAskDialog::execute(const String& msg)
{
    String htmlMessage(msg);
    if (upperCase(htmlMessage).find("<HTML>") == STRING_NPOS)
        htmlMessage = "<HTML><BODY>" + htmlMessage.replace("\\n", "<BR>") + "</BODY></HTML>";
    m_textBox->data(htmlMessage);
    relayout();
    return showModal();
}

bool CInputDialog::execute(const String& msg, String& inputText)
{
    m_inputBox->data(inputText.c_str());
    if (CAskDialog::execute(msg)) {
        inputText = m_inputBox->data().asString().c_str();
        return true;
    }
    return false;
}

int sptk::spAsk(const String& message)
{
    CAskDialog ask("Please, confirm");
    ask.icon(CThemes::getIconImage("question", IS_DIALOG_ICON));
    return ask.execute(message);
}

int sptk::spInformation(const String& message)
{
    CMessageDialog ask("Information");
    ask.icon(CThemes::getIconImage("info", IS_DIALOG_ICON));
    return ask.execute(message);
}

int sptk::spWarning(const String& message)
{
    CMessageDialog ask("Warning");
    ask.icon(CThemes::getIconImage("warning", IS_DIALOG_ICON));
    return ask.execute(message);
}

int sptk::spError(const String& message)
{
    CMessageDialog ask("Error");
    ask.icon(CThemes::getIconImage("error", IS_DIALOG_ICON));
    return ask.execute(message);
}

int sptk::spInput(const String& message, String& inputText)
{
    CInputDialog ask("Please, input");
    ask.icon(CThemes::getIconImage("question", IS_DIALOG_ICON));
    return ask.execute(message, inputText);
}
