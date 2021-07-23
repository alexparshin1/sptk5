#pragma once

#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerConnection.h>

namespace sptk {

/**
 * Not encrypted connection to echo server
 */
class EchoConnection
    : public TCPServerConnection
{
public:
    EchoConnection(TCPServer& server, SOCKET connectionSocket, const sockaddr_in* connectionAddress);

    ~EchoConnection() override = default;

    /**
     * Terminate connection thread
     */
    void terminate() override;

    /**
     * Connection thread function
     */
    void run() override;
};

/**
 * @brief Echo server used in unit tests
 */
class EchoServer
    : public TCPServer
{
public:

    EchoServer();

protected:

    SServerConnection createConnection(SOCKET connectionSocket, sockaddr_in* peer) override;
};

}
