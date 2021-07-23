#include "EchoServer.h"

#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;


EchoConnection::EchoConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress)
    : TCPServerConnection(server, connectionSocket, connectionAddress)
{
}

void EchoConnection::terminate()
{
    socket().close();
    TCPServerConnection::terminate();
}

void EchoConnection::run()
{
    Buffer data;
    while (!terminated())
    {
        try
        {
            if (socket().readyToRead(chrono::seconds(1)))
            {
                if (socket().readLine(data) == 0)
                {
                    return;
                }
                string str(data.c_str());
                str += "\n";
                socket().write(str);
            }
            else
            {
                break;
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl)
        }
    }
    socket().close();
}

EchoServer::EchoServer()
    : TCPServer("EchoServer")
{
}

SServerConnection EchoServer::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    return make_shared<EchoConnection>(*this, connectionSocket, peer);
}
