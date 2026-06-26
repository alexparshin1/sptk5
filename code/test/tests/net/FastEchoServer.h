#pragma once
#include <cstdint>
#include <sptk5/net/FastTCPServer.h>

namespace sptk {
/**
 * A very primitive event-driven test server that echoes back what's sent to it.
 *
 * Built on FastTCPServer: a single reactor monitors all connections and delivers data events to
 * socketEventCallback(), which echoes the received bytes back to the client. A session ends when
 * the accumulated data contains the "<EOF>" marker or the peer hangs up.
 */
class FastEchoServer : public sptk::FastTCPServer
{
public:
    /**
     * Constructor
     * @param port              Listener port number
     */
    explicit FastEchoServer(uint16_t port);

    ~FastEchoServer() override;

    /**
     * Create an echo connection that carries its own accumulated-data tail.
     */
    SServerConnection createConnection(ServerConnection::Type connectionType, SocketType connectionSocket,
                                       const sockaddr_in* peer) override;

protected:
    /**
     * Echo callback.
     * Sends back to the client the same data it receives, and closes the connection once the "<EOF>"
     * marker is seen or the peer hangs up.
     * @param connection        Server connection that received the event.
     * @param eventType         Event type.
     */
    void socketEventCallback(const std::shared_ptr<ServerConnection>& connection, SocketEventType eventType) override;
};

} // namespace sptk
