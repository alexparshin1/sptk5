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
    /**
     * Constructor
     * @param port              Listener port number
     */
    explicit TestEchoServer(uint16_t port);

private:
    /**
     * Echo function
     * Send back to the client the same data it receives.
     * @param serverConnection  Server connection
     */
    static void echoFunction(ServerConnection& serverConnection);
};

} // namespace sptk
