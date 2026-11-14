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

#include "test/TestData.h"

#include <gtest/gtest.h>

#include <sptk5/Buffer.h>
#include <sptk5/net/FastTCPServer.h>
#include <sptk5/net/HttpReader.h>
#include <sptk5/net/HttpConnect.h>
#include <sptk5/net/TCPSocket.h>

#include <chrono>
#include <future>
#include <string>

using namespace std;
using namespace sptk;

namespace {

uint16_t getHttpReaderTestPort()
{
    return TestData::freePort();
}

class FixedResponseServer : public FastTCPServer
{
public:
    FixedResponseServer(uint16_t port, String response,
                        chrono::milliseconds delay = chrono::milliseconds(0))
        : FastTCPServer("HttpReader FixedResponseServer")
        , m_response(std::move(response))
        , m_delay(delay)
    {
        addListener(ServerConnection::Type::TCP, {"127.0.0.1", port});
    }

    ~FixedResponseServer() override
    {
        stop();
    }

    /**
     * @brief Write the fixed response to the freshly accepted connection, then close it.
     *
     * Pure push server: the client only reads, so the whole response is produced here, on the
     * listener thread, and the connection is returned closed so the reactor does not monitor it.
     */
    SServerConnection createConnection(ServerConnection::Type connectionType, const SocketType connectionSocket,
                                       const sockaddr_in* /*peer*/) override
    {
        const auto socket = createConnectionSocket(connectionType, connectionSocket);
        try
        {
            std::this_thread::sleep_for(m_delay);
            socket->write(m_response);
        }
        catch (const exception& exception)
        {
            CERR(exception.what());
        }
        socket->close();

        return {};
    }

private:
    String m_response;
    chrono::milliseconds m_delay;
};

} // namespace
namespace sptk {

TEST(HttpReaderTests, getDeadlineBoundsSlowResponseAndClosesSocket)
{
    const auto port = getHttpReaderTestPort();
    FixedResponseServer server(port, "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK",
                               chrono::milliseconds(400));
    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", port});
    HttpConnect connection(socket);
    connection.setResponseLimits(1024, 32);
    Buffer output;
    const auto started = chrono::steady_clock::now();
    EXPECT_THROW((void) connection.cmd_get("/", HttpParams(), output, nullptr,
                                           chrono::milliseconds(80)), Exception);
    EXPECT_LT(chrono::steady_clock::now() - started, chrono::milliseconds(300));
    EXPECT_FALSE(socket->active());
}

TEST(HttpReaderTests, responseLimitsRejectOversizedInput)
{
    const vector<String> responses {
        "HTTP/1.1 200 OK\r\nContent-Length: 1000000000\r\n\r\n",
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n10000000\r\n",
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n-1\r\n",
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\nxyz\r\n",
        "HTTP/1.1 200 OK\r\nContent-Length: 8\r\n\r\nshort",
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n8\r\nshort",
        "HTTP/1.1 200 OK\r\nContent-Length: -1\r\n\r\n",
        "HTTP/1.1 200 OK\r\nContent-Length: 2junk\r\n\r\nOK",
        "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nContent-Length: 3\r\n\r\nOK",
        "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nTransfer-Encoding: chunked\r\n\r\n",
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n2\r\nOKxx0\r\n\r\n",
        "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n",
        "HTTP/1.1 200 OK\r\nX-Large: " + String(300, 'x') + "\r\n\r\n",
        "HTTP/1.1 200 OK\r\nContent-Encoding: gzip\r\nContent-Length: 1\r\n\r\nx"
    };
    for (const auto& response : responses)
    {
        const auto port = getHttpReaderTestPort();
        FixedResponseServer server(port, response);
        const auto socket = make_shared<TCPSocket>();
        socket->open({"127.0.0.1", port});
        Buffer output;
        HttpReader reader(socket, output, HttpReader::ReadMode::RESPONSE);
        reader.setReadLimits(256, 32);
        EXPECT_THROW(reader.readAll(chrono::seconds(2)), Exception);
        EXPECT_LE(output.bytes(), 32);
    }
}

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
    reader.setReadLimits(1024, 32);

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
    reader.setReadLimits(1024, 32);

    const auto status = reader.readAll(chrono::seconds(2));
    EXPECT_EQ(status, 200);
    EXPECT_STREQ(reader.httpHeader("Content-Type").c_str(), "text/plain");

    const String body(out.c_str(), out.bytes());
    EXPECT_STREQ(body.c_str(), "hello world");
}

TEST(HttpReaderTests, requestModeParsesRequestLineAndHeaders)
{
    FastTCPServer server("HttpReader RequestModeServer");

    std::promise<void> gotRequestPromise;
    std::future<void>  gotRequestFuture = gotRequestPromise.get_future();

    std::promise<String> requestTypePromise;
    std::promise<String> requestUrlPromise;
    std::promise<String> hostHeaderPromise;

    server.onSocketEvent([&](const weak_ptr<ServerConnection>& weakConnection, const SocketEventType eventType)
                         {
                             const auto connection = weakConnection.lock();
                             if (!connection || !eventType.m_data)
                             {
                                 return;
                             }

                             const auto s = connection->getSocket();
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

                             server.closeConnection(connection);
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
