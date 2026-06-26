#include "FastEchoServer.h"

using namespace std;
using namespace sptk;

namespace {

/**
 * @brief Connection that remembers a short tail of the data echoed so far.
 *
 * The reactor delivers data event by event, so the "<EOF>" marker may span two events; the tail
 * lets it be detected without re-reading already-echoed bytes.
 */
class EchoConnection final : public ServerConnection
{
public:
    EchoConnection(const Type type, const sockaddr_in* peer)
        : ServerConnection(type, peer)
    {
    }

    String accumulated;
};

} // namespace

FastEchoServer::FastEchoServer(uint16_t port)
    : FastTCPServer("TestServer")
{
    addListener(ServerConnection::Type::TCP, {"localhost", port});
}

FastEchoServer::~FastEchoServer()
{
    // FastTCPServer requires derived classes to stop the reactor before their own members go away.
    stop();
}

SServerConnection FastEchoServer::createConnection(const ServerConnection::Type connectionType,
                                                   const SocketType connectionSocket, const sockaddr_in* peer)
{
    const auto socket = createConnectionSocket(connectionType, connectionSocket);

    auto connection = make_shared<EchoConnection>(connectionType, peer);
    connection->setSocket(socket);

    return connection;
}

void FastEchoServer::socketEventCallback(const shared_ptr<ServerConnection>& connection, const SocketEventType eventType)
{
    if (eventType.m_hangup || eventType.m_error)
    {
        closeConnection(connection);
        return;
    }

    if (!eventType.m_data)
    {
        return;
    }

    const auto echoConnection = dynamic_pointer_cast<EchoConnection>(connection);
    if (!echoConnection)
    {
        return;
    }

    const auto echoSocket = connection->getSocket();
    try
    {
        const auto bytes = echoSocket->socketBytes();
        if (bytes == 0)
        {
            // Readable with no data: the peer has hung up.
            closeConnection(connection);
            return;
        }

        String message;
        echoSocket->read(message, bytes);
        echoSocket->write(message);

        COUT("Echo: [" << message << "]");

        echoConnection->accumulated += message;
        if (echoConnection->accumulated.contains("<EOF>"))
        {
            COUT("Test server hangup");
            closeConnection(connection);
            return;
        }

        // Keep the accumulator from growing unbounded in long sessions.
        if (constexpr size_t keepTail = 16;
            echoConnection->accumulated.length() > keepTail)
        {
            echoConnection->accumulated = echoConnection->accumulated.substr(echoConnection->accumulated.length() - keepTail);
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
        closeConnection(connection);
    }
}
