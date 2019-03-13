/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SMQServer.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 23 2018                                ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <smq/server/SMQServer.h>
#include <smq/clients/SMQClient.h>
#include <smq/protocols/MQTTProtocol.h>

using namespace std;
using namespace sptk;
using namespace chrono;

SMQServer::SMQServer(MQProtocolType protocol, const String& username, const String& password, LogEngine& logEngine)
: TCPServer("SMQServer", 16, &logEngine),
  m_protocol(protocol),
  m_username(username), m_password(password),
  m_socketEvents("SMQ Server", SMQServer::socketEventCallback, milliseconds(100))
{
}

SMQServer::~SMQServer()
{
    clear();
}

void SMQServer::stop()
{
    m_socketEvents.terminate();
	TCPServer::stop();
    m_socketEvents.stop();
    log(LP_NOTICE, "Server stopped");
}

ServerConnection* SMQServer::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    auto* newConnection = new SMQConnection(*this, connectionSocket, peer);

    lock_guard<mutex> lock(m_mutex);
    m_connections.insert(newConnection);

    return newConnection;
}

void SMQServer::closeConnection(ServerConnection* connection, bool brokenConnection)
{
    auto* smqConnection = dynamic_cast<SMQConnection*>(connection);
    if (smqConnection != nullptr) {
        String clientId = smqConnection->clientId();
        lock_guard<mutex> lock(m_mutex);
        m_clientIds.erase(clientId);
        m_connections.erase(smqConnection);

        if (brokenConnection) {
            SMessage lastWillMessage = smqConnection->getLastWillMessage();
            if (lastWillMessage)
                distributeMessage(lastWillMessage);
        }
    }
}

static map<String,sptk::QOS> parseDestinations(const String& destinations)
{
    map<String,sptk::QOS> destinationsAndQOS;
    for (auto& destination: Strings(destinations,",")) {
        Strings parts(destination,"%");
        if (parts.size() == 2)
            destinationsAndQOS[ parts[0] ] = sptk::QOS(string2int(parts[1]) & 1);
        else
            destinationsAndQOS[ parts[0] ] = QOS_1;
    }
    return destinationsAndQOS;
}

void SMQServer::socketEventCallback(void *userData, SocketEventType eventType)
{
    auto* connection = (SMQConnection*) userData;
    auto* smqServer = dynamic_cast<SMQServer*>(&connection->server());

    if (eventType == ET_CONNECTION_CLOSED) {
        smqServer->closeConnection(connection, true);
        delete connection;
        return;
    }

    try {
        while (connection != nullptr && connection->socket().socketBytes() > 0) {

            SMessage msg;
            MQProtocol& protocol = connection->protocol();
            protocol.readMessage(msg);

            switch (msg->type()) {
                case Message::CONNECT:
                    if (!smqServer->authenticate((*msg)["client_id"], (*msg)["username"], (*msg)["password"])) {
                        smqServer->closeConnection(connection, false);
                        delete connection;
                        connection = nullptr;
                    } else {
                        const String& lastWillDestination = (*msg)["last_will_destination"];
                        const String& lastWillMessage = (*msg)["last_will_message"];
                        connection->setupClient((*msg)["client_id"], lastWillDestination, lastWillMessage);
                        if (!lastWillDestination.empty()) {
                            msg->headers().erase("last_will_destination");
                            msg->headers().erase("last_will_message");
                        }
                    }
                    protocol.ack(Message::CONNECT, "");
                    break;
                case Message::SUBSCRIBE:
                    smqServer->subscribe(connection, parseDestinations(msg->destination()));
                    break;
                case Message::UNSUBSCRIBE:
                    smqServer->unsubscribe(connection, msg->destination());
                    break;
                case Message::MESSAGE:
                    smqServer->distributeMessage(msg);
                    break;
                case Message::DISCONNECT:
                    smqServer->closeConnection(connection, false);
                    delete connection;
                    connection = nullptr;
                    break;
                default:
                    break;
            }
        }
    }
    catch (const Exception& e) {
        if (connection != nullptr) {
            smqServer->closeConnection(connection, true);
            delete connection;
        }
        smqServer->log(LP_ERROR, e.message());
    }
}

void SMQServer::distributeMessage(SMessage message)
{
    m_subscriptions.deliverMessage(message->destination(), message);
}

bool SMQServer::authenticate(const String& clientId, const String& username, const String& password)
{
    lock_guard<mutex> lock(m_mutex);

    String logPrefix("(" + clientId + ") ");

    if (m_clientIds.find(clientId) != m_clientIds.end()) {
        log(LP_ERROR, logPrefix + "Duplicate client id");
        return false;
    }

    if (username != m_username || password != m_password) {
        log(LP_ERROR, logPrefix + "Invalid username or password");
        return false;
    }

    log(LP_INFO, logPrefix + "Authenticated");

    return true;
}

void SMQServer::watchSocket(TCPSocket& socket, void* userData)
{
    m_socketEvents.add(socket, userData);
}

void SMQServer::forgetSocket(TCPSocket& socket)
{
    m_socketEvents.remove(socket);
}

void SMQServer::run()
{
    Thread::run();
    log(LP_NOTICE, "Server started");
}

void SMQServer::subscribe(SMQConnection* connection, const map<String,sptk::QOS>& destinations)
{
    m_subscriptions.subscribe(connection, destinations);

    Strings destinationNames;
    for (auto& itor: destinations)
        destinationNames.push_back(itor.first);

    log(LP_INFO, "(" + connection->clientId() + ") Subscribed to " + destinationNames.join(", "));
}

void SMQServer::unsubscribe(SMQConnection* connection, const String& destination)
{
    m_subscriptions.unsubscribe(connection, destination);
}

void SMQServer::clear()
{
    m_subscriptions.clear();
    lock_guard<mutex> lock(m_mutex);
    for (auto* connection: m_connections)
        delete connection;
    m_connections.clear();
    m_clientIds.clear();
}

void SMQServer::execute(Runable*)
{
    // SMQServer doesn't use tasks model
}

MQProtocolType SMQServer::protocol() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_protocol;
}

#include "unit_tests/SMQServer_UT.cpp"