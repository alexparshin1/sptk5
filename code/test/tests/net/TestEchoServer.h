#pragma once
#include <cstdint>
#include <sptk5/net/TCPServer.h>

namespace sptk {

class TestEchoServer : public sptk::TCPServer
{
public:
    TestEchoServer(uint16_t port);
private:
    static void echoFunction(ServerConnection& serverConnection);
};

}