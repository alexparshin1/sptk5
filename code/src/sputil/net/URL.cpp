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

static bool nextToken(const String& url, size_t& start, size_t end, const String& delimiter, String& value)
{
    value = "";
    end = url.find(delimiter, start);
    if (end != string::npos) {
        value = url.substr(start, end - start);
        start = end + delimiter.length();
        return true;
    }
    return false;
}

URL::URL(const String& url)
{
    //static const RegularExpression matchUrl(R"(^((?<protocol>[a-z]+)://)?((?<username>[\w\.\-@]+):(?<password>[^@]*)@)?(?<host>[\w\.\:]+)?(?<path>/[\w\.\:/]+)?(?<parameters>\?.*)?$)");

    size_t start = 0;
    size_t end = 0;
    nextToken(url, start, end, "://", m_protocol);

    String credentials;
    m_username = "";
    m_password = "";
    m_params.clear();
    nextToken(url, start, end, "@", credentials);
    if (!credentials.empty()) {
        auto pos = credentials.find(":");
        if (pos == string::npos)
            m_username = credentials;
        else {
            m_username = credentials.substr(0, pos);
            m_password = credentials.substr(pos + 1);
        }
    }

    Buffer buffer;
    if (!nextToken(url, start, end, "/", m_hostAndPort))
        m_hostAndPort = url.substr(start);
    else {
        --start;
        if (nextToken(url, start, end, "?", m_path)) {
            buffer.set(url.substr(start));
            m_params.decode(buffer);
        }
        else {
            m_path = url.substr(start);
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

static const String testURL0 = "http://www.test.com:8080/daily/report";
static const String testURL1 = "/daily/report?action=view&id=1";
static const String testURL2 = "http://johnd:secret@www.test.com:8080/daily/report?action=view&id=1";

TEST(SPTK_URL, minimal)
{
    URL url(testURL0);
    EXPECT_STREQ(url.protocol().c_str(), "http");
    EXPECT_STREQ(url.hostAndPort().c_str(), "www.test.com:8080");
    EXPECT_STREQ(url.username().c_str(), "");
    EXPECT_STREQ(url.password().c_str(), "");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");

    EXPECT_EQ(url.params().size(), size_t(0));

    EXPECT_STREQ(url.toString().c_str(), testURL0.c_str());
}

TEST(SPTK_URL, local)
{
    URL url(testURL1);
    EXPECT_STREQ(url.protocol().c_str(), "");
    EXPECT_STREQ(url.hostAndPort().c_str(), "");
    EXPECT_STREQ(url.username().c_str(), "");
    EXPECT_STREQ(url.password().c_str(), "");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");

    EXPECT_EQ(url.params().size(), size_t(2));
    EXPECT_STREQ(url.params().get("action").c_str(), "view");
    EXPECT_STREQ(url.params().get("id").c_str(), "1");

    EXPECT_STREQ(url.toString().c_str(), testURL1.c_str());
}

TEST(SPTK_URL, all)
{
    URL url(testURL2);
    EXPECT_STREQ(url.protocol().c_str(), "http");
    EXPECT_STREQ(url.hostAndPort().c_str(), "www.test.com:8080");
    EXPECT_STREQ(url.username().c_str(), "johnd");
    EXPECT_STREQ(url.password().c_str(), "secret");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");

    EXPECT_EQ(url.params().size(), size_t(2));
    EXPECT_STREQ(url.params().get("action").c_str(), "view");
    EXPECT_STREQ(url.params().get("id").c_str(), "1");

    EXPECT_STREQ(url.toString().c_str(), testURL2.c_str());
}

TEST(SPTK_URL, loop)
{
    size_t numIterations = 100;
    for (size_t i = 0; i < numIterations; ++i)
        URL url(testURL0);
}

#endif
