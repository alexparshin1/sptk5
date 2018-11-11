/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CMessageDialog.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CMESSAGEDIALOG_H__
#define __CMESSAGEDIALOG_H__

#include <sptk5/gui/CDialog.h>
#include <sptk5/gui/CDataControl.h>

#include <string>

#include <FL/Fl_Pixmap.H>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief A Yes/No dialog box
 *
 * Creates a dialog with the text or HTML text of question (message) and two buttons (Ok/Cancel)
 */
class CAskDialog : public CDialog {
    /**
     * The icon box
     */
    Fl_Box    *m_imageBox;

    /**
     * The HTML box widget to show the message text
     */
    CHtmlBox  *m_textBox;

protected:
    /**
     * Input box, optionally shown
     */
    CInput   *m_inputBox;

public:
    /**
     * Constructor
     * @param label const char *, window caption
     * @param w int, window width
     */
    CAskDialog(const char *label,int w=400);

    /**
     * Shows message and waits for the user
     * @param msg const char *, message to show
     */
    bool execute(const String& msg);

    /**
     * Sets icon for the window
     * @param image Fl_Pixmap *, icon to use, suggested size is 48x48 XPM
     */
    void icon(Fl_Image *image) {
        m_imageBox->image(image);
    }
};

/**
 * @brief An information dialog box
 *
 * Creates a dialog with the text or HTML text of message and one button (Ok)
 */
class CMessageDialog : public CAskDialog {
public:
    /**
     * Constructor
     * @param label const char *, window caption
     * @param w int, window width
     */
    CMessageDialog(const char *label,int w=400) : CAskDialog(label,w) {
        m_cancelButton->hide();
    }
};

/**
 * @brief A single input Yes/No dialog box
 *
 * Creates a dialog with the text or HTML text of question (message) and two buttons (Ok/Cancel)
 */
class CInputDialog : public CAskDialog {
public:
    /**
     * Constructor
     * @param label const char *, window caption
     * @param w int, window width
     */
    CInputDialog(const char *label,int w=400) : CAskDialog(label,w) {
        m_inputBox->show();
    }

    /**
     * Shows message and waits for the user input
     * @param msg std::string, message to show
     * @param inputText std::string&, string to input
     */
    bool execute(const String& msg, String& inputText);
};

/**
 * @brief A Yes/No dialog box function
 *
 * Creates a dialog with the text or HTML text of question (message) and two buttons (Ok/Cancel)
 */
int spAsk(const String& message);

/**
 * @brief A warning information dialog box warning function
 *
 * Creates a dialog with the text or HTML text of message and one button (Ok)
 */
int spWarning(const String& message);

/**
 * @brief An error information dialog box function
 *
 * Creates a dialog with the text or HTML text of message and one button (Ok)
 */
int spError(const String& message);

/**
 * @brief An information dialog box function
 *
 * Creates a dialog with the text or HTML text of message and one button (Ok)
 */
int spInformation(const String& message);

/**
 * @brief A single input Yes/No dialog box
 *
 * Creates a dialog with the text or HTML text of question (message),
 * two buttons (Ok/Cancel), and input box
 */
int spInput(const String& message, String& inputText);

/**
 * @}
 */
}
#endif
