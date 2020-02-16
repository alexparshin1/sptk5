/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/net/URL.h>

using namespace std;
using namespace sptk;

URL::URL(const String& url)
{
    RegularExpression matchUrl(R"(^(([a-z]+)://)?([\w\.\:]+)?(/[\w\.\:/]+)(\?.*)?$)");
    Strings matches;
    if (matchUrl.m(url.trim(), matches)) {
        m_protocol = matches[1];
        m_hostAndPort = matches[2];
        if (matches.size() > 3) {
            m_path = matches[3].empty() ? "" : matches[3];
            if (matches.size() > 4) {
                Buffer buffer(matches[4].substr(1));
                m_params.decode(buffer);
            }
        }
    }
}

String URL::toString() const
{
    stringstream str;

    if (!m_protocol.empty())
        str << m_protocol << "://";

    if (!m_username.empty())
        str << m_username << ":" << m_password << "@";

    if (m_hostAndPort.empty())
        str << "localhost";
    else
        str << m_hostAndPort;

    if (!m_path.empty())
        str << m_path;

    if (!m_params.empty()) {
        Buffer buffer;
        m_params.encode(buffer);
        str << "?" << buffer.c_str();
    }

    return str.str();
}

#if USE_GTEST

static const char* testURL = "http://johnd:secret@www.test.com:8080/daily/report?action=view&id=1";

TEST(SPTK_URL, all)
{
    URL url(testURL);
    EXPECT_STREQ(url.protocol().c_str(), "http");
    EXPECT_STREQ(url.hostAndPort().c_str(), "www.test.com:8080");
    EXPECT_STREQ(url.username().c_str(), "johnd");
    EXPECT_STREQ(url.password().c_str(), "secret");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");

    EXPECT_EQ(url.params().size(), size_t(2));
    EXPECT_STREQ(url.params().get("action").c_str(), "view");
    EXPECT_STREQ(url.params().get("id").c_str(), "1");

    EXPECT_STREQ(url.toString().c_str(), testURL);
}

#endif