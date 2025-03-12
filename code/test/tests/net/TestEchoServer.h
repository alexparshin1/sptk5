#pragma once
#include <cstdint>
#include <sptk5/net/TCPServer.h>

namespace sptk {

/**
 * A very primitive test server that echoes back what's sent to it.
 */
class TestEchoServer : public sptk::TCPServer
{
public:
    TestEchoServer(uint16_t port);
private:
    static void echoFunction(ServerConnection& serverConnection);
};

}