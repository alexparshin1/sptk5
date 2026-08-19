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

#include <gtest/gtest.h>
#include <sptk5/Base64.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/cutils>
#include <sptk5/net/HttpConnect.h>
#include <sptk5/net/HttpProxy.h>
#include <sptk5/net/SSLSocket.h>

using namespace std;
using namespace sptk;
using namespace chrono;
namespace sptk {

TEST(HttpProxyTests, connect)
{
    // Check if the proxy is defined in the environment variable.
    // It's typical for Linux, and rare for Windows.
    // If proxy is not defined or defined in the wrong format, this test is exiting.
    const char* proxy = getenv("HTTP_PROXY");
    if (proxy == nullptr)
    {
        proxy = getenv("http_proxy");
    }
    if (proxy == nullptr)
    {
        return;
    } // No proxy defined, don't test

    RegularExpression matchProxy(R"(^((.*):(.*)@)?(\d+\.\d+\.\d+\.\d+:\d+)$)");
    auto              matches = matchProxy.m(proxy);
    if (!matches)
    {
        CERR("Can't parse proxy from environment variable");
        return;
    }

    String proxyUser(matches[1].value);
    String proxyPassword(matches[2].value);
    Host   proxyHost(matches[3].value);

    auto   httpProxy = make_shared<HttpProxy>(proxyHost, proxyUser, proxyPassword);
    String error;
    try
    {
        Host aHost("www.sptk.net:80");

        shared_ptr<TCPSocket> socket;
        if (constexpr auto httpPort {80};
            aHost.port() == httpPort)
        {
            socket = make_shared<TCPSocket>();
        }
        else
        {
            socket = make_shared<SSLSocket>();
        }

        socket->setProxy(std::move(httpProxy));
        constexpr seconds connectTimeout {5};
        socket->open(aHost, Socket::OpenMode::CONNECT, true, connectTimeout);

        HttpConnect http(socket);

        Buffer        output;
        constexpr int minimalHttpError = 400;

        if (auto statusCode = http.cmd_get("/", HttpParams(), output);
            statusCode >= minimalHttpError)
        {
            throw Exception(http.statusText());
        }

        COUT(output.c_str());
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(HttpProxyTests, getDefaultProxy)
{
    String proxyUser;
    String proxyPassword;

#ifdef _WIN32
    auto proxyHost = HttpProxy::getDefaultProxy(proxyUser, proxyPassword);
#else
    setenv("HTTP_PROXY", "127.0.0.1:3128", 1);
    auto proxyHost = HttpProxy::getDefaultProxy(proxyUser, proxyPassword);
    EXPECT_STREQ(proxyHost->toString(true).c_str(), "127.0.0.1:3128");
    EXPECT_STREQ(proxyUser.c_str(), "");
    EXPECT_STREQ(proxyPassword.c_str(), "");

    setenv("HTTP_PROXY", "http://Domain\\user:password@127.0.0.1:3128", 1);
    proxyHost = HttpProxy::getDefaultProxy(proxyUser, proxyPassword);
    EXPECT_STREQ(proxyHost->toString(true).c_str(), "127.0.0.1:3128");
    EXPECT_STREQ(proxyUser.c_str(), "Domain\\user");
    EXPECT_STREQ(proxyPassword.c_str(), "password");
#endif
}

} // namespace sptk
