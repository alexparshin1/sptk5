#include "TestEchoServer.h"

#include "sptk5/Printer.h"

using namespace std;
using namespace sptk;

TestEchoServer::TestEchoServer(uint16_t port)
    : TCPServer("TestServer")
{
    onConnection(echoFunction);
    addListener(ServerConnection::Type::TCP, {"localhost", port});
}

void TestEchoServer::echoFunction(const ServerConnection& serverConnection)
{
    const auto echoSocket = serverConnection.getSocket();

    try
    {
        String accumulated;
        bool   clientHangup = false;

        while (!clientHangup)
        {
            if (!echoSocket->readyToRead(100ms))
            {
                continue;
            }

            const auto bytes = echoSocket->socketBytes();
            if (bytes == 0)
            {
                // Client hangup
                clientHangup = true;
                continue;
            }

            String message;
            echoSocket->read(message, bytes);

            accumulated += message;
            echoSocket->write(message);

            COUT("Echo: [" << message << "]");

            if (accumulated.contains("<EOF>"))
            {
                COUT("Test server hangup");
                break;
            }

            // Keep accumulator from growing unbounded in long sessions
            if (constexpr size_t keepTail = 16;
                accumulated.length() > keepTail)
            {
                accumulated = accumulated.substr(accumulated.length() - keepTail);
            }
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }

    echoSocket->close();
    serverConnection.close();
}
