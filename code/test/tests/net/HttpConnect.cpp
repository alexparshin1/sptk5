/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>
#include <sptk5/Printer.h>
#include <sptk5/md5.h>
#include <sptk5/net/HttpConnect.h>

using namespace std;
using namespace sptk;

TEST(SPTK_HttpConnect, get)
{
    Host google("www.google.com:80");

    auto socket = make_shared<TCPSocket>();

    EXPECT_NO_THROW(socket->open(google));
    EXPECT_TRUE(socket->active());

    HttpConnect http(*socket);
    Buffer output;

    constexpr int minStatusCode {500};
    int statusCode {minStatusCode};
    try
    {
        statusCode = http.cmd_get("/", HttpParams(), output);
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
    EXPECT_EQ(200, statusCode);
    EXPECT_STREQ("OK", http.statusText().c_str());

    String data(output.c_str(), output.bytes());
    EXPECT_TRUE(data.toLowerCase().find("</html>") != string::npos);
}
