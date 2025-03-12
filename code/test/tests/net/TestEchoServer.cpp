#include "TestEchoServer.h"

#include <sptk5/net/SocketReader.h>

using namespace std;
using namespace sptk;

TestEchoServer::TestEchoServer(uint16_t port)
    : TCPServer("TestServer")
{
    onConnection(echoFunction);
    addListener(ServerConnection::Type::TCP, port);
}

void TestEchoServer::echoFunction(ServerConnection& serverConnection)
{
    auto echoSocket = serverConnection.getSocket();

    try
    {
        while (true)
        {
            if (echoSocket->readyToRead(100ms))
            {
                auto bytes = echoSocket->socketBytes();
                if (bytes == 0)
                {
                    // Client hangup
                    break;
                }

                String message;
                echoSocket->read(message, bytes);
                echoSocket->write(message);
                COUT("Echo: [" << message << "]");
                if (message.endsWith("<EOF>"))
                {
                    break;
                }
            }
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
    echoSocket->close();
}
