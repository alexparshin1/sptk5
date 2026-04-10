/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
namespace sptk {

TEST(HttpConnectTests,get)
{
    const Host google("www.sptk.net:80");

    const auto socket = make_shared<TCPSocket>();

    EXPECT_NO_THROW(socket->open(google));
    EXPECT_TRUE(socket->active());

    HttpConnect http(socket);
    Buffer      output;

    try
    {
        auto statusCode = http.cmd_get("/", HttpParams(), output);
        EXPECT_EQ(200, statusCode);
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
    EXPECT_STREQ("OK", http.statusText().c_str());

    const String data(output.c_str(), output.bytes());
    EXPECT_TRUE(data.toLowerCase().find("</html>") != string::npos);
}

// ... existing code ...

TEST(HttpConnectTests,basicAuthorizationIsBase64UserColonPass)
{
    // "user:pass" -> base64("user:pass") == "dXNlcjpwYXNz"
    const HttpConnect::BasicAuthorization auth("user", "pass");

    EXPECT_EQ("basic", auth.method().toLowerCase());
    EXPECT_EQ("dXNlcjpwYXNz", auth.value());
}

TEST(HttpConnectTests,bearerAuthorizationPreservesToken)
{
    const String                           token("header.payload.signature");
    const HttpConnect::BearerAuthorization auth(token);

    EXPECT_EQ("bearer", auth.method().toLowerCase());
    EXPECT_EQ(token, auth.value());
}

TEST(HttpConnectTests,accessorsBeforeAnyRequestDoNotCrash)
{
    // No connection / no request performed: define expected "safe" behavior.
    auto        socket = make_shared<TCPSocket>();
    HttpConnect http(socket);

    EXPECT_EQ(0, http.statusCode());
    EXPECT_TRUE(http.statusText().empty());

    const auto& headers = http.responseHeaders();
    (void) headers; // just verifying this is safe to call before any request
}

} // namespace sptk_test
