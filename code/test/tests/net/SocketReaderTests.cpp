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
#include <sptk5/net/FastTCPServer.h>
#include <sptk5/net/SocketReader.h>
#include <sptk5/net/TCPSocket.h>

#include <chrono>
#include <future>
#include <string>
#include <thread>

using namespace std;
using namespace sptk;

namespace {

uint16_t getSocketReaderTestPort()
{
    static atomic_uint16_t port = 65000;
    return port++;
}

/**
 * @brief Test server that sends predefined data to clients.
 */
class TestDataServer : public FastTCPServer
{
public:
    TestDataServer(uint16_t port, String data)
        : FastTCPServer("SocketReader TestDataServer")
        , m_data(std::move(data))
    {
        addListener(ServerConnection::Type::TCP, {"127.0.0.1", port});
    }

    ~TestDataServer() override
    {
        stop();
    }

    /**
     * @brief Push the predefined data to the freshly accepted connection, then close it.
     *
     * Pure push server: the client never sends anything, so the whole response is produced here,
     * on the listener thread, and the connection is returned closed so the reactor does not
     * monitor it.
     */
    SServerConnection createConnection(ServerConnection::Type connectionType, const SocketType connectionSocket,
                                       const sockaddr_in* /*peer*/) override
    {
        const auto socket = createConnectionSocket(connectionType, connectionSocket);
        try
        {
            socket->write(m_data);
            this_thread::sleep_for(500ms);
        }
        catch (const exception& e)
        {
            CERR(e.what());
        }
        socket->close();

        return {};
    }

private:
    String m_data;
};

/**
 * @brief Test server that sends data in chunks with delays.
 */
class ChunkedDataServer : public FastTCPServer
{
public:
    ChunkedDataServer(uint16_t port, vector<String> chunks, const chrono::milliseconds delay)
        : FastTCPServer("SocketReader ChunkedDataServer")
        , m_chunks(std::move(chunks))
        , m_delay(delay)
    {
        addListener(ServerConnection::Type::TCP, {"127.0.0.1", port});
    }

    ~ChunkedDataServer() override
    {
        stop();
    }

    /**
     * @brief Push all chunks to the freshly accepted connection, then close it.
     *
     * This is a pure push server: the client never sends anything, so no read event would arrive
     * to drive a socketEventCallback(). The whole response is produced here, on the listener thread,
     * and the connection is returned closed so the reactor does not monitor it.
     */
    SServerConnection createConnection(ServerConnection::Type connectionType, const SocketType connectionSocket,
                                       const sockaddr_in* /*peer*/) override
    {
        const auto socket = createConnectionSocket(connectionType, connectionSocket);
        try
        {
            for (const auto& chunk: m_chunks)
            {
                socket->write(chunk);
                this_thread::sleep_for(m_delay);
            }
        }
        catch (const exception& e)
        {
            // Any error interrupts connection.
            CERR(e.what());
        }
        socket->close();

        // Connection is fully served and closed; nothing for the reactor to monitor.
        return {};
    }

private:
    vector<String>       m_chunks;
    chrono::milliseconds m_delay;
};

} // namespace
namespace sptk {

TEST(SocketReaderTests, constructor)
{
    const auto         socket = make_shared<TCPSocket>();
    const SocketReader reader(socket);

    EXPECT_EQ(reader.availableBytes(), 0);
    EXPECT_FALSE(reader.active());
}

TEST(SocketReaderTests, constructorCustomBufferSize)
{
    const auto         socket = make_shared<TCPSocket>();
    const SocketReader reader(socket, 32768);

    EXPECT_EQ(reader.availableBytes(), 0);
    EXPECT_FALSE(reader.active());
}

TEST(SocketReaderTests, readBinaryData)
{
    const String   testData = "Binary data test 12345";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);
    EXPECT_TRUE(reader.active());

    uint8_t    buffer[256];
    const auto bytesRead = reader.read(buffer);

    EXPECT_EQ(bytesRead, testData.length());
    EXPECT_EQ(string(reinterpret_cast<char*>(buffer), bytesRead), testData);
}

TEST(SocketReaderTests, readIntoBuffer)
{
    const String   testData = "Buffer read test";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    Buffer     buffer;
    const auto bytesRead = reader.read(buffer, testData.length());

    EXPECT_EQ(bytesRead, testData.length());
    EXPECT_STREQ(buffer.c_str(), testData.c_str());
}

TEST(SocketReaderTests, readTemplate)
{
    struct TestStruct
    {
        uint32_t value1;
        uint32_t value2;
    };

    constexpr TestStruct testData = {0x12345678, 0xABCDEF00};
    const String         binaryData(reinterpret_cast<const char*>(&testData), sizeof(TestStruct));

    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, binaryData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    TestStruct received = {0, 0};
    const auto bytesRead1 = reader.read(received.value1);
    const auto bytesRead2 = reader.read(received.value2);
    const auto bytesRead = bytesRead1 + bytesRead2;

    EXPECT_EQ(bytesRead, sizeof(TestStruct));
    EXPECT_EQ(received.value1, testData.value1);
    EXPECT_EQ(received.value2, testData.value2);
}

TEST(SocketReaderTests, readLineSingleLine)
{
    const String   testData = "Single line test\n";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    String     line;
    const auto bytesRead = reader.readLine(line);

    EXPECT_GT(bytesRead, 0);
    EXPECT_STREQ(line.c_str(), "Single line test");
}

TEST(SocketReaderTests, readLineMultipleLines)
{
    const String   testData = "Line 1\nLine 2\nLine 3\n";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    String line1;
    String line2;
    String line3;
    reader.readLine(line1);
    reader.readLine(line2);
    reader.readLine(line3);

    EXPECT_STREQ(line1.c_str(), "Line 1");
    EXPECT_STREQ(line2.c_str(), "Line 2");
    EXPECT_STREQ(line3.c_str(), "Line 3");
}

TEST(SocketReaderTests, readLineCustomDelimiter)
{
    const String   testData = "Item1|Item2|Item3|";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    String item1;
    String item2;
    String item3;
    reader.readLine(item1, '|');
    reader.readLine(item2, '|');
    reader.readLine(item3, '|');

    EXPECT_STREQ(item1.c_str(), "Item1");
    EXPECT_STREQ(item2.c_str(), "Item2");
    EXPECT_STREQ(item3.c_str(), "Item3");
}

TEST(SocketReaderTests, readLineIntoBuffer)
{
    const String   testData = "Buffer line test\n";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    Buffer     buffer;
    const auto bytesRead = reader.readLine(buffer);

    EXPECT_GT(bytesRead, 0);
    EXPECT_STREQ(buffer.c_str(), "Buffer line test");
}

TEST(SocketReaderTests, readLineIntoRawBuffer)
{
    const String   testData = "Raw buffer line test\n";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    array<uint8_t, 256> buffer {};
    const auto          bytesRead = reader.readLine(buffer.data(), buffer.size(), '\n');

    EXPECT_GT(bytesRead, 0);
    EXPECT_STREQ(reinterpret_cast<char*>(buffer.data()), "Raw buffer line test");
}

TEST(SocketReaderTests, close)
{
    const String   testData = "Close test";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);
    EXPECT_TRUE(reader.active());

    reader.close();
    EXPECT_FALSE(reader.active());
}

TEST(SocketReaderTests, availableBytes)
{
    const String   testData = "Available bytes test";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    const SocketReader reader(socket);

    // Wait for data
    if (!reader.readyToRead(100ms))
    {
        FAIL() << "Data is not received within timeout";
    }

    const auto available = reader.availableBytes();
    EXPECT_GT(available, 0);
}

TEST(SocketReaderTests, readyToReadWithData)
{
    const String   testData = "Ready test";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    const SocketReader reader(socket);

    const bool ready = reader.readyToRead(500ms);
    EXPECT_TRUE(ready);
}

TEST(SocketReaderTests, readPartialData)
{
    const String   testData = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    // Read in small chunks
    uint8_t    buffer[10];
    const auto bytes1 = reader.read(buffer, 10);
    EXPECT_EQ(bytes1, 10);
    EXPECT_EQ(string(reinterpret_cast<char*>(buffer), 10), "ABCDEFGHIJ");

    const auto bytes2 = reader.read(buffer, 10);
    EXPECT_EQ(bytes2, 10);
    EXPECT_EQ(string(reinterpret_cast<char*>(buffer), 10), "KLMNOPQRST");
}

TEST(SocketReaderTests, chunkedRead)
{
    const vector<String> chunks = {"Chunk1", "Chunk2", "Chunk3"};
    auto                 socketReaderTestPort = getSocketReaderTestPort();
    ChunkedDataServer    server(socketReaderTestPort, chunks, 50ms);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    array<uint8_t, 256> buffer {};

    size_t totalRead = 0;
    size_t bytesRead = 0;

    do
    {
        bytesRead = reader.read(buffer.data() + totalRead, sizeof(buffer) - totalRead);
        totalRead += bytesRead;
    } while (bytesRead > 0 && totalRead < sizeof(buffer));

    const string result(reinterpret_cast<char*>(buffer.data()), totalRead);
    EXPECT_EQ(result, "Chunk1Chunk2Chunk3");
}

TEST(SocketReaderTests, socketGetter)
{
    const auto         socket = make_shared<TCPSocket>();
    const SocketReader reader(socket);

    EXPECT_EQ(reader.socket(), socket);
}

TEST(SocketReaderTests, readFromClosedSocket)
{
    const auto   socket = make_shared<TCPSocket>();
    SocketReader reader(socket);

    array<uint8_t, 256> buffer {};
    EXPECT_ANY_THROW(reader.read(buffer.data(), buffer.size()));
}

TEST(SocketReaderTests, readLineEmptyLine)
{
    const String   testData = "\nContent";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    String     line;
    const auto bytesRead = reader.readLine(line);

    EXPECT_GT(bytesRead, 0);
    EXPECT_TRUE(line.empty());
}

TEST(SocketReaderTests, readLineCRLF)
{
    const String   testData = "Line with CRLF\r\n";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    String line;
    reader.readLine(line);

    // Should read until \n and include the content before it
    EXPECT_TRUE(line == "Line with CRLF\r" || line == "Line with CRLF");
}

TEST(SocketReaderTests, readMultipleSmallReads)
{
    const String   testData = "1234567890";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    SocketReader reader(socket);

    // Read byte by byte
    for (int i = 0; i < 10; ++i)
    {
        uint8_t    byte;
        const auto bytesRead = reader.read(&byte, 1);
        EXPECT_EQ(bytesRead, 1);
        EXPECT_EQ(byte, '0' + ((i + 1) % 10));
    }
}

TEST(SocketReaderTests, canRead)
{
    const String   testData = "Can read test data";
    auto           socketReaderTestPort = getSocketReaderTestPort();
    TestDataServer server(socketReaderTestPort, testData);

    const auto socket = make_shared<TCPSocket>();
    socket->open({"127.0.0.1", socketReaderTestPort});

    const SocketReader reader(socket);

    // Wait for data to arrive
    if (!reader.readyToRead(100ms))
    {
        FAIL() << "Data is not received within timeout";
    }

    EXPECT_TRUE(reader.canRead(5));
}

} // namespace sptk
