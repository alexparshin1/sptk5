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

#include <sptk5/Buffer.h>
#include <sptk5/net/HttpReader.h>
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPSocket.h>

#include <chrono>
#include <future>
#include <string>

using namespace std;
using namespace sptk;

namespace {

uint16_t getHttpReaderTestPort()
{
    static atomic_uint16_t port = 20000;
    return port++;
}

class FixedResponseServer : public TCPServer
{
public:
    FixedResponseServer(uint16_t port, String response)
        : TCPServer("HttpReader FixedResponseServer", 1)
        , m_response(std::move(response))
    {
        onConnection([this](const ServerConnection& connection)
                     {
                         const auto s = connection.getSocket();
                         try
                         {
                             s->write(m_response);
                         }
                         catch (const exception& exception)
                         {
                             CERR(exception.what());
                         }
                         s->close();
                     });

        addListener(ServerConnection::Type::TCP, {"127.0.0.1", port});
    }

private:
    String m_response;
};

} // namespace
namespace sptk {

TEST(HttpReaderTests, responseContentLengthReadsBodyAndHeaders)
{
    const String response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 5\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello";

    auto                httpReaderTestPort = getHttpReaderTestPort();
    FixedResponseServer server(httpReaderTestPort, response);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", httpReaderTestPort});

    Buffer     out;
    HttpReader reader(socket, out, HttpReader::ReadMode::RESPONSE);

    const auto status = reader.readAll(chrono::seconds(2));
    EXPECT_EQ(status, 200);
    EXPECT_STREQ(reader.getStatusText().c_str(), "OK");
    EXPECT_STREQ(reader.httpHeader("Content-Type").c_str(), "text/plain");

    const String body(out.c_str(), out.bytes());
    EXPECT_STREQ(body.c_str(), "Hello");
}

TEST(HttpReaderTests, responseChunkedReadsAllChunks)
{
    // "hello world" is 11 bytes = 0x0B
    const String response =
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "B\r\n"
        "hello world\r\n"
        "0\r\n"
        "\r\n";

    auto                httpReaderTestPort = getHttpReaderTestPort();
    FixedResponseServer server(httpReaderTestPort, response);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", httpReaderTestPort});

    Buffer     out;
    HttpReader reader(socket, out, HttpReader::ReadMode::RESPONSE);

    const auto status = reader.readAll(chrono::seconds(2));
    EXPECT_EQ(status, 200);
    EXPECT_STREQ(reader.httpHeader("Content-Type").c_str(), "text/plain");

    const String body(out.c_str(), out.bytes());
    EXPECT_STREQ(body.c_str(), "hello world");
}

TEST(HttpReaderTests, requestModeParsesRequestLineAndHeaders)
{
    TCPServer server("HttpReader RequestModeServer", 1);

    std::promise<void> gotRequestPromise;
    std::future<void>  gotRequestFuture = gotRequestPromise.get_future();

    std::promise<String> requestTypePromise;
    std::promise<String> requestUrlPromise;
    std::promise<String> hostHeaderPromise;

    server.onConnection([&](const ServerConnection& connection)
                        {
                            const auto s = connection.getSocket();
                            try
                            {
                                Buffer     out;
                                HttpReader reader(s, out, HttpReader::ReadMode::REQUEST);

                                reader.readHttpHeaders();

                                requestTypePromise.set_value(reader.getRequestType());
                                requestUrlPromise.set_value(reader.getRequestURL());
                                hostHeaderPromise.set_value(reader.httpHeader("Host"));

                                gotRequestPromise.set_value();

                                s->write(
                                    "HTTP/1.1 204 No Content\r\n"
                                    "Content-Length: 0\r\n"
                                    "\r\n");
                            }
                            catch (const exception& ex)
                            {
                                CERR(ex.what());
                                try
                                {
                                    gotRequestPromise.set_value();
                                }
                                catch (const exception& exc)
                                {
                                    CERR(exc.what());
                                }
                            }

                            s->close();
                        });

    auto httpReaderTestPort = getHttpReaderTestPort();
    server.addListener(ServerConnection::Type::TCP, {"127.0.0.1", httpReaderTestPort});

    TCPSocket client;
    client.open({"127.0.0.1", httpReaderTestPort});

    client.write(
        "GET /unit-test HTTP/1.1\r\n"
        "Host: example.test\r\n"
        "\r\n");

    ASSERT_EQ(gotRequestFuture.wait_for(chrono::seconds(2)), std::future_status::ready);

    EXPECT_STREQ(requestTypePromise.get_future().get().c_str(), "GET");
    EXPECT_STREQ(requestUrlPromise.get_future().get().c_str(), "/unit-test");
    EXPECT_STREQ(hostHeaderPromise.get_future().get().c_str(), "example.test");
}

} // namespace sptk
