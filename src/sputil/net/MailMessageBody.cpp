/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MailMessageBody.cpp - description                      ║
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

#include <sptk5/Strings.h>
#include <sptk5/net/MailMessageBody.h>

using namespace std;
using namespace sptk;

String MailMessageBody::stripHtml(const String& origHtml)
{
    Strings html(origHtml, "<");
    unsigned i = 0;

    // Remove comments and scripts
    for (i = 0; i < html.size(); i++) {
        string& str = html[i];
        size_t pos = str.find('>');
        if (pos == STRING_NPOS)
            continue;
        str = str.substr(pos + 1);
        if (str.empty()) {
            html.erase(html.begin() + i);
            i--;
        }
    }

    return trim(html.asString(" ").replace(" +", " "));
}

void MailMessageBody::text(const string& messageText, bool smtp)
{
    string msg(messageText);
    if (smtp) {
        size_t pos = 0;
        while ( (pos = msg.find("\n.\n", pos)) != string::npos) {
            msg[pos + 1] = ' ';
            pos += 3;
        }
    }
    if (upperCase(messageText.substr(0, 100)).find("<HTML>") == STRING_NPOS) {
        m_type = MMT_PLAIN_TEXT_MESSAGE;
        m_plainText = msg;
        m_htmlText = "";
    } else {
        m_type = MMT_HTML_MESSAGE;
        m_plainText = stripHtml(msg);
        m_htmlText = msg;
    }
}
