/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpProxy.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday June 19 2019                                 ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/net/HttpProxy.h>
#include <sptk5/net/SSLSocket.h>
#include <sptk5/Base64.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/net/HttpConnect.h>

#ifdef _WIN32
#include <wininet.h>
#endif


using namespace std;
using namespace sptk;
using namespace chrono;

SOCKET HttpProxy::connect(const Host& destination, bool blockingMode, std::chrono::milliseconds timeout)
{
    auto socket = make_shared<TCPSocket>();
    try {
        socket->open(m_host, BaseSocket::SOM_CONNECT, blockingMode, timeout);
    }
    catch (const Exception& e) {
        throw ConnectionException("Can't connect to proxy " + m_host.toString() + ": " + String(e.what()));
    }

    if (destination.port() == 80) {
        socket->write("GET " + destination.toString() + " HTTP/1.1\r\n");
    } else {
        socket->write("CONNECT " + destination.toString() + " HTTP/1.1\r\n");
        socket->write("Proxy-Connection: keep-alive\r\n");
    }

    socket->write("User-agent: SPTK\r\n");
    if (!m_username.empty()) {
        Buffer usernameAndPassword(m_username + ":" + m_password);
        Buffer encodedUsernameAndPassword;
        Base64::encode(encodedUsernameAndPassword, usernameAndPassword.c_str(), usernameAndPassword.bytes());
        socket->write("Proxy-Authorization: Basic " + String(encodedUsernameAndPassword.c_str()) + "\r\n");
    }
    socket->write("\r\n");

    String error("Proxy connection timeout");
    if (socket->readyToRead(seconds(10))) {
        Buffer buffer;
        socket->readLine(buffer);

        RegularExpression matchProxyResponse(R"(^HTTP\S+ (\d+) (.*)$)");
        Strings matches;
        if (matchProxyResponse.m(buffer.c_str(), matches)) {
            int rc = matches[0].toInt();
            if (rc >= 400) {
                socket->close();
                throw Exception("Proxy connection error: " + matches[1]);
            }
        }

        // Read all headers
        RegularExpression matchResponseHeader(R"(^(\S+): (.*)$)");
        int contentLength = -1;
        while (buffer.bytes() > 1) {
            socket->readLine(buffer);
            if (matchResponseHeader.m(buffer.c_str(), matches)) {
                if (matches[0].toLowerCase() == "content-length")
                    contentLength = matches[1].toInt();
            } else
                break;
        }

        // Read response body (if any)
        if (contentLength > 0) {
            socket->read(buffer, contentLength);
        } else {
            while (socket->readyToRead(milliseconds(100)))
                socket->read(buffer, socket->socketBytes());
        }
    }

    SOCKET handle = socket->detach();
    socket.reset();

    return handle;
}

#ifdef _WIN32
Buffer windowsQueryInternetOption(int option)
{
    // Determine the required buffer size.
    DWORD dwSize{ 0 };
    InternetQueryOption(NULL, option, NULL, &dwSize);

    Buffer output(dwSize);

    // Call InternetQueryOption again with the provided buffer.
    InternetQueryOption(NULL, option, output.data(), &dwSize);

    return output;
}
#endif

bool HttpProxy::getDefaultProxy(Host& proxyHost, String& proxyUser, String& proxyPassword)
{
#ifdef _WIN32
    auto buffer = windowsQueryInternetOption(INTERNET_OPTION_PROXY);
    INTERNET_PROXY_INFO* proxy = (INTERNET_PROXY_INFO*)buffer.data();
    if (proxy->dwAccessType != INTERNET_OPEN_TYPE_PROXY)
        return false;

    if (strlen(proxy->lpszProxy) == 0)
        return false;

    proxyHost = Host(proxy->lpszProxy);

    buffer = windowsQueryInternetOption(INTERNET_OPTION_PROXY_USERNAME);
    proxyUser = buffer.c_str();

    buffer = windowsQueryInternetOption(INTERNET_OPTION_PROXY_PASSWORD);
    proxyPassword = buffer.c_str();

    return true;
#else
    RegularExpression matchProxy(R"(^(http://)?((\S+)(:\S+)@)?(\S+:\d+)$)");
    char* proxyEnv = getenv("http_proxy");
    if (proxyEnv == nullptr)
        proxyEnv = getenv("HTTP_PROXY");
    if (proxyEnv == nullptr)
        return false;

    Strings parts;
    if (matchProxy.m(proxyEnv, parts)) {
        proxyUser = parts[2];
        proxyPassword = parts[3].empty() ? "" : parts[3].substr(1);
        proxyHost = Host(parts[4]);
        return true;
    }

    return false;
#endif
}

#if USE_GTEST

TEST(SPTK_HttpProxy, connect)
{
    auto httpProxy = make_unique<HttpProxy>(Host("192.168.1.1:3128"), "auser", "apassword");
    String error;
    try {
        //Host ahost("www.chiark.greenend.org.uk:443");
        Host ahost("www.sptk.net:80");

        shared_ptr<TCPSocket> socket;
        if (ahost.port() == 80)
            socket = make_shared<TCPSocket>();
        else
            socket = make_shared<SSLSocket>();

        socket->setProxy(move(httpProxy));
        socket->open(ahost, BaseSocket::SOM_CONNECT, true, seconds(5));

        HttpConnect http(*socket);

        Buffer output;
        http.cmd_get("/", HttpParams(), output);
        cout << output.c_str() << endl;
    }
    catch (const Exception& e) {
        FAIL() << e.what();
    }
    //EXPECT_NE(sock, INVALID_SOCKET);
}

TEST(SPTK_HttpProxy, getDefaultProxy)
{
    Host   proxyHost;
    String proxyUser;
    String proxyPassword;

#ifdef _WIN32
    HttpProxy::getDefaultProxy(proxyHost, proxyUser, proxyPassword);
#else
    setenv("HTTP_PROXY", "127.0.0.1:3128", 1);
    HttpProxy::getDefaultProxy(proxyHost, proxyUser, proxyPassword);
    EXPECT_STREQ(proxyHost.toString(true).c_str(), "127.0.0.1:3128");
    EXPECT_STREQ(proxyUser.c_str(), "");
    EXPECT_STREQ(proxyPassword.c_str(), "");

    setenv("HTTP_PROXY", "http://Domain\\user:password@127.0.0.1:3128", 1);
    HttpProxy::getDefaultProxy(proxyHost, proxyUser, proxyPassword);
    EXPECT_STREQ(proxyHost.toString(true).c_str(), "127.0.0.1:3128");
    EXPECT_STREQ(proxyUser.c_str(), "Domain\\user");
    EXPECT_STREQ(proxyPassword.c_str(), "password");
#endif
}

#endif