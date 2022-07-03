/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Base64.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/cutils>
#include <sptk5/net/HttpConnect.h>
#include <sptk5/net/HttpProxy.h>
#include <sptk5/net/SSLSocket.h>

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

#ifdef _WIN32
#include <winhttp.h>
#endif


using namespace std;
using namespace sptk;
using namespace chrono;

SOCKET HttpProxy::connect(const Host& destination, bool blockingMode, std::chrono::milliseconds timeout)
{
    auto socket = make_shared<TCPSocket>();

    Strings methods("CONNECT|GET", "|");
    bool proxyConnected = false;
    for (const auto& method: methods)
    {
        try
        {
            socket->open(m_host, BaseSocket::OpenMode::CONNECT, blockingMode, timeout);
            sendRequest(destination, socket, method);

            String error("Proxy connection timeout");

            if (constexpr seconds readTimeout(10);
                socket->readyToRead(readTimeout))
            {
                proxyConnected = readResponse(socket);
            }

            if (proxyConnected)
            {
                break;
            }
        }
        catch (const Exception& e)
        {
            throw ConnectionException("Can't connect to proxy " + m_host.toString() + ": " + String(e.what()));
        }
    }

    SOCKET handle = socket->detach();
    socket.reset();

    return handle;
}

bool HttpProxy::readResponse(const shared_ptr<TCPSocket>& socket)
{
    bool proxyConnected {false};
    Buffer buffer;
    socket->readLine(buffer);

    RegularExpression matchProxyResponse(R"(^HTTP\S+ (\d+) (.*)$)");
    if (auto responseMatches = matchProxyResponse.m(buffer.c_str()); responseMatches)
    {
        constexpr int minimalHttpError = 400;
        int rc = responseMatches[0].value.toInt();
        if (rc < minimalHttpError)
        {
            proxyConnected = true;
        }
    }

    // Read all headers
    RegularExpression matchResponseHeader(R"(^(\S+): (.*)$)");
    int contentLength = -1;
    while (buffer.bytes() > 1)
    {
        socket->readLine(buffer);
        auto matches = matchResponseHeader.m(buffer.c_str());
        if (matches)
        {
            if (matches[0].value.toLowerCase() == "content-length")
            {
                contentLength = matches[1].value.toInt();
            }
        }
        else
        {
            break;
        }
    }

    // Read response body (if any)
    if (contentLength > 0)
    {
        socket->read(buffer, (size_t) contentLength);
    }
    else
    {
        constexpr milliseconds timeout {100};
        while (socket->readyToRead(timeout))
        {
            socket->read(buffer, socket->socketBytes());
        }
    }

    return proxyConnected;
}

void HttpProxy::sendRequest(const Host& destination, const shared_ptr<TCPSocket>& socket, const String& method) const
{
    socket->write(method + " " + destination.toString() + " HTTP/1.1\r\n");
    socket->write("Proxy-Connection: keep-alive\r\n");

    socket->write("User-agent: SPTK\r\n");
    if (!m_username.empty())
    {
        Buffer usernameAndPassword(m_username + ":" + m_password);
        Buffer encodedUsernameAndPassword;
        Base64::encode(encodedUsernameAndPassword, (const uint8_t*) usernameAndPassword.c_str(),
                       usernameAndPassword.bytes());
        socket->write("Proxy-Authorization: Basic " + String(encodedUsernameAndPassword.c_str()) + "\r\n");
    }
    socket->write("\r\n");
}

#ifdef _WIN32
static bool windowsGetDefaultProxy(Host& host, String& username, String& password)
{
    WINHTTP_AUTOPROXY_OPTIONS AutoProxyOptions {};
    WINHTTP_PROXY_INFO ProxyInfo {};

    HINTERNET hHttpSession = WinHttpOpen(L"WinHTTP AutoProxy",
                                         WINHTTP_ACCESS_TYPE_NO_PROXY,
                                         WINHTTP_NO_PROXY_NAME,
                                         WINHTTP_NO_PROXY_BYPASS,
                                         0);

    if (!hHttpSession)
        throw Exception("Can't initialize WinHTTP");

    // Use auto-detection because the Proxy
    // Auto-Config URL is not known.
    AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;

    // Use DHCP and DNS-based auto-detection.
    AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;

    // If obtaining the PAC script requires NTLM/Negotiate
    // authentication, then automatically supply the client
    // domain credentials.
    AutoProxyOptions.fAutoLogonIfChallenged = TRUE;

    // Call WinHttpGetProxyForUrl with our target URL
    char proxy[256] {};
    char userName[256] {};
    char passWord[256] {};
    DWORD size = sizeof(password);
    if (WinHttpGetProxyForUrl(hHttpSession,
                              L"https://www.microsoft.com/ms.htm",
                              &AutoProxyOptions,
                              &ProxyInfo))
    {
        if (ProxyInfo.lpszProxy == nullptr)
            return false;

        wcstombs(proxy, ProxyInfo.lpszProxy, sizeof(proxy));
        host = Host(proxy);

        if (WinHttpQueryOption(hHttpSession, WINHTTP_OPTION_PROXY_USERNAME, userName, &size))
            username = userName;

        if (WinHttpQueryOption(hHttpSession, WINHTTP_OPTION_PROXY_PASSWORD, passWord, &size))
            password = passWord;
    }

    if (ProxyInfo.lpszProxy != nullptr)
        GlobalFree(ProxyInfo.lpszProxy);

    if (ProxyInfo.lpszProxyBypass != nullptr)
        GlobalFree(ProxyInfo.lpszProxyBypass);

    WinHttpCloseHandle(hHttpSession);

    return true;
}
#endif

bool HttpProxy::getDefaultProxy(Host& proxyHost, String& proxyUser, String& proxyPassword)
{
#ifdef _WIN32
    return windowsGetDefaultProxy(proxyHost, proxyUser, proxyPassword);
#else
    RegularExpression matchProxy(R"(^(http://)?((\S+)(:\S+)@)?(\S+:\d+)$)");
    const char* proxyEnv = getenv("http_proxy");
    if (proxyEnv == nullptr)
    {
        proxyEnv = getenv("HTTP_PROXY");
    }

    if (proxyEnv == nullptr)
    {
        return false;
    }

    if (auto parts = matchProxy.m(proxyEnv); parts)
    {
        proxyUser = parts[2].value;
        proxyPassword = parts[3].value.empty() ? "" : parts[3].value.substr(1);
        proxyHost = Host(parts[4].value);
        return true;
    }

    return false;
#endif
}

#ifdef USE_GTEST

TEST(SPTK_HttpProxy, connect)
{
    // Check if proxy is defined in environment variable.
    // It's typical for Linux, and rare for Windows.
    // If proxy is not defined or defined in wrong format, this test is exiting.
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
    auto matches = matchProxy.m(proxy);
    if (!matches)
    {
        CERR("Can't parse proxy from environment variable" << endl)
        return;
    }

    String proxyUser(matches[1].value);
    String proxyPassword(matches[2].value);
    Host proxyHost(matches[3].value);

    auto httpProxy = make_shared<HttpProxy>(proxyHost, proxyUser, proxyPassword);
    String error;
    try
    {
        constexpr int httpPort {80};
        Host ahost("www.sptk.net:80");

        shared_ptr<TCPSocket> socket;
        if (ahost.port() == httpPort)
        {
            socket = make_shared<TCPSocket>();
        }
        else
        {
            socket = make_shared<SSLSocket>();
        }

        socket->setProxy(move(httpProxy));
        constexpr seconds connectTimeout {5};
        socket->open(ahost, BaseSocket::OpenMode::CONNECT, true, connectTimeout);

        HttpConnect http(*socket);

        Buffer output;
        constexpr int minimalHttpError = 400;

        if (auto statusCode = http.cmd_get("/", HttpParams(), output);
            statusCode >= minimalHttpError)
        {
            throw Exception(http.statusText());
        }

        COUT(output.c_str() << endl)
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_HttpProxy, getDefaultProxy)
{
    Host proxyHost;
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
