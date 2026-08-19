/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/net/MailMessageBody.h>

using namespace std;
using namespace sptk;

String MailMessageBody::stripHtml(const String& origHtml)
{
    static const RegularExpression matchHtmlTag(R"(<\S[^>]*>)", "g");
    const auto                     step1 = matchHtmlTag.s(origHtml, " ");
    return trim(String(step1).replace(" +", " "));
}

void MailMessageBody::text(const string& messageText, const bool smtp)
{
    string msg(messageText);
    if (smtp)
    {
        size_t pos = 0;
        while ((pos = msg.find("\n.\n", pos)) != string::npos)
        {
            msg[pos + 1] = ' ';
            pos += 3;
        }
    }

    constexpr auto searchFirstBytes {100};
    if (upperCase(messageText.substr(0, searchFirstBytes)).find("<HTML>") == STRING_NPOS)
    {
        m_type = MailMessageType::PLAIN_TEXT_MESSAGE;
        m_plainText = msg;
        m_htmlText = "";
    }
    else
    {
        m_type = MailMessageType::HTML_MESSAGE;
        m_plainText = stripHtml(msg);
        m_htmlText = msg;
    }
}
