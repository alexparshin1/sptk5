#include "TestEchoServer.h"

#include <sptk5/net/SocketReader.h>

using namespace std;
using namespace sptk;

TestEchoServer::TestEchoServer(uint16_t port)
    :    TCPServer("TestServer")
{
    onConnection(echoFunction);
    addListener(ServerConnection::Type::TCP, port);
}

void TestEchoServer::echoFunction(ServerConnection& serverConnection)
{
    auto* echoSocket = serverConnection.getSocket().get();

    SocketReader socketReader(*echoSocket);
    Buffer       data;
    bool         terminated = false;
    while (!terminated)
    {
        try
        {
            if (socketReader.readyToRead(3s))
            {
                if (socketReader.socket().socketBytes() == 0)
                {
                    break;
                }
                string str(data.c_str());
                str += "\n";
                echoSocket->write(str);
                COUT("Echo: " << str);
            }
            else
            {
                terminated = true;
            }
        }
        catch (const Exception&)
        {
            terminated = true;
        }
    }
    echoSocket->close();
    COUT("Echo thread exited.");
}

