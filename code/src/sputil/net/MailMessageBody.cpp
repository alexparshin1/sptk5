/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;

String MailMessageBody::stripHtml(const String& origHtml)
{
    static const RegularExpression matchHtmlTag(R"(<\S[^>]*>)", "g");
    auto step1 = matchHtmlTag.s(origHtml, " ");
    return trim(step1.replace(" +", " "));
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

#if USE_GTEST

TEST(SPTK_MailMessageBody, minimal)
{
    MailMessageBody message;

    message.text("<html><b>Hello,</b><i>World!</i></html>", false);
    EXPECT_EQ(message.text(), "Hello, World!");

    message.text("<html><b>Hello,</b><i>World!</i></html>\n.\n", true);
    EXPECT_EQ(message.text(), "Hello, World!");
}

#endif
