/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "TestImapServer.h"

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

#include <sptk5/cnet>

using namespace std;
using namespace sptk;

namespace {

enum class Command
{
    Capability,
    Login,
    Logout,
    Select,
    Examine,
    Close,
    Subscribe,
    Unsubscribe,
    Create,
    Delete,
    Rename,
    List,
    Append,
    Fetch
};

const map<string, Command, less<>> commandMap = {
    {"capability", Command::Capability},
    {"login", Command::Login},
    {"logout", Command::Logout},
    {"select", Command::Select},
    {"examine", Command::Examine},
    {"close", Command::Close},
    {"subscribe", Command::Subscribe},
    {"unsubscribe", Command::Unsubscribe},
    {"create", Command::Create},
    {"delete", Command::Delete},
    {"rename", Command::Rename},
    {"list", Command::List},
    {"append", Command::Append},
    {"fetch", Command::Fetch},
};

/**
 * @brief Connection that carries a per-connection SocketReader.
 *
 * The reactor delivers events level-triggered, so the reader must persist between events: bytes it
 * buffers (a pipelined or partially-received command) have already left the kernel socket and would
 * otherwise be lost when a fresh reader is created for the next event.
 */
class ImapConnection final : public ServerConnection
{
public:
    ImapConnection(const Type type, const sockaddr_in* peer)
        : ServerConnection(type, peer)
    {
    }

    std::shared_ptr<SocketReader> reader;
};

} // namespace

TestImapServer::TestImapServer(uint16_t port)
    : FastTCPServer("Test IMAP server")
{
    addListener(ServerConnection::Type::TCP, {"localhost", port});
}

TestImapServer::~TestImapServer()
{
    // FastTCPServer requires derived classes to stop the reactor before their own members go away.
    stop();
}

SServerConnection TestImapServer::createConnection(const ServerConnection::Type connectionType,
                                                   const SocketType connectionSocket, const sockaddr_in* peer)
{
    const auto socket = createConnectionSocket(connectionType, connectionSocket);

    auto connection = make_shared<ImapConnection>(connectionType, peer);
    connection->setSocket(socket);
    connection->reader = make_shared<SocketReader>(socket);

    // IMAP server greeting, sent as soon as the connection is accepted.
    socket->write("* OK IMAP4rev1 Service Ready\r\n");

    return connection;
}

void TestImapServer::socketEventCallback(const shared_ptr<ServerConnection>& connection, const SocketEventType eventType)
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

    const auto imapConnection = dynamic_pointer_cast<ImapConnection>(connection);
    if (!imapConnection)
    {
        return;
    }

    const auto& reader = imapConnection->reader;
    try
    {
        // Drain every complete command currently available: pipelined commands buffered in the
        // reader will not produce another reactor event. A partial command throws here (no data yet)
        // and is left buffered for the next event.
        while (connection->getSocket()->active() && reader->readyToRead(chrono::milliseconds(0)))
        {
            readIncomingData(reader);
        }
    }
    catch (const Exception&)
    {
        // No more data ready, or the client went away mid-command.
    }

    if (!connection->getSocket()->active())
    {
        closeConnection(connection);
    }
}

void TestImapServer::readIncomingData(const shared_ptr<SocketReader>& socketReader)
{
    if (String incomingData;
        socketReader->readLine(incomingData))
    {
        static const RegularExpression matchCommand(R"(^(a\d+) (\w+)\s?([^\r]+)?[\r]*$)");
        COUT("\n>  " << incomingData);
        if (const auto matches = matchCommand.m(incomingData))
        {
            const auto command = commandMap.find(lowerCase(matches[1].value));
            if (command == commandMap.end())
            {
                reply(socketReader, format("{} BAD Unknown command\r\n", matches[1].value));
                return;
            }

            const auto ident = matches[0].value;
            switch (command->second)
            {
                using enum Command;
                case Capability:
                    handle_cmd_capability(socketReader, ident);
                    break;
                case Login:
                    handle_cmd_login(socketReader, ident, matches[2].value);
                    break;
                case Logout:
                    handle_cmd_logout(socketReader, ident);
                    break;
                case Select:
                    handle_cmd_select(socketReader, ident);
                    break;
                case Examine:
                    handle_cmd_examine(socketReader, ident);
                    break;
                case Close:
                    handle_cmd_close(socketReader, ident);
                    break;
                case Subscribe:
                    handle_cmd_subscribe(socketReader, ident, matches[2].value);
                    break;
                case Unsubscribe:
                    handle_cmd_unsubscribe(socketReader, ident, matches[2].value);
                    break;
                case Create:
                    handle_cmd_create(socketReader, ident, matches[2].value);
                    break;
                case Delete:
                    handle_cmd_delete(socketReader, ident, matches[2].value);
                    break;
                case Rename:
                    handle_cmd_rename(socketReader, ident, matches[2].value);
                    break;
                case List:
                    handle_cmd_list(socketReader, ident, matches[2].value);
                    break;
                case Append:
                    handle_cmd_append(socketReader, ident, matches[2].value);
                    break;
                case Fetch:
                    handle_cmd_fetch(socketReader, ident, matches[2].value);
                    break;
            }
        }
    }
}

void TestImapServer::reply(const shared_ptr<SocketReader>& socketReader, const std::string& response)
{
    socketReader->socket()->write(response + "\r\n");
    COUT("<  " << trim(response));
}

void TestImapServer::handle_cmd_capability(const shared_ptr<SocketReader>& socketReader, const string& ident)
{
    reply(socketReader, "* CAPABILITY IMAP4rev1 AUTH=PLAIN AUTH=LOGIN");
    reply(socketReader, format("{} OK CAPABILITY completed", ident));
}

void TestImapServer::handle_cmd_login(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    auto userAndPassword = data.split(" ");
    for (auto& s: userAndPassword)
    {
        if (s.starts_with('"'))
        {
            s = s.substr(1, s.size() - 2);
        }
    }

    if (userAndPassword.size() != 2 || userAndPassword[0] != "user" || userAndPassword[1] != "password")
    {
        reply(socketReader, format("{} NO Authentication failed: Invalid credentials", ident));
        return;
    }
    reply(socketReader, format("{} OK LOGIN completed", ident));
}

void TestImapServer::handle_cmd_select(const shared_ptr<SocketReader>& socketReader, const string& ident)
{
    reply(socketReader, "* 5 EXISTS");
    reply(socketReader, "* 1 RECENT");
    reply(socketReader, "* OK [UNSEEN 1] First unseen message");
    reply(socketReader, "* OK [UIDVALIDITY 1] UIDs valid");
    reply(socketReader, "* OK [UIDNEXT 6] Predicted next UID");
    reply(socketReader, R"(* FLAGS (\Answered \Flagged \Deleted \Seen \Draft))");
    reply(socketReader, R"(* OK [PERMANENTFLAGS (\Deleted \Seen \*)] Flags permitted.)");
    reply(socketReader, format("{} OK [READ-WRITE] SELECT completed", ident));
}

void TestImapServer::handle_cmd_examine(const shared_ptr<SocketReader>& socketReader, const string& ident)
{
    reply(socketReader, "* 5 EXISTS");
    reply(socketReader, "* 1 RECENT");
    reply(socketReader, "* OK [UNSEEN 1] First unseen message");
    reply(socketReader, "* OK [UIDVALIDITY 1] UIDs valid");
    reply(socketReader, "* OK [PERMANENTFLAGS ()] No permanent flags permitted");
    reply(socketReader, R"(* FLAGS (\Answered \Flagged \Deleted \Seen \Draft))");
    reply(socketReader, format("{} OK [READ-ONLY] EXAMINE completed", ident));
}

void TestImapServer::handle_cmd_close(const shared_ptr<SocketReader>& socketReader, const std::string& ident)
{
    reply(socketReader, format("{} OK CLOSE completed", ident));
    socketReader->close();
}

void TestImapServer::handle_cmd_subscribe(const shared_ptr<SocketReader>& socketReader, const string& ident, const String&)
{
    reply(socketReader, format("{} OK SUBSCRIBE completed", ident));
}

void TestImapServer::handle_cmd_unsubscribe(const shared_ptr<SocketReader>& socketReader, const string& ident, const String&)
{
    reply(socketReader, format("{} OK UNSUBSCRIBE completed", ident));
}

void TestImapServer::handle_cmd_create(const shared_ptr<SocketReader>& socketReader, const string& ident, const String&)
{
    reply(socketReader, format("{} OK CREATE completed", ident));
}

void TestImapServer::handle_cmd_delete(const shared_ptr<SocketReader>& socketReader, const string& ident, const String&)
{
    reply(socketReader, format("{} OK DELETE completed", ident));
}

void TestImapServer::handle_cmd_rename(const shared_ptr<SocketReader>& socketReader, const string& ident, const String&)
{
    reply(socketReader, format("{} OK RENAME completed", ident));
}

void TestImapServer::handle_cmd_list(const shared_ptr<SocketReader>& socketReader, const string& ident, const String&)
{
    reply(socketReader, R"(* LIST (\HasNoChildren) "." "INBOX")");
    reply(socketReader, R"(* LIST (\HasNoChildren) "." "Sent")");
    reply(socketReader, R"(* LIST (\HasNoChildren) "." "Drafts")");
    reply(socketReader, format("{} OK LIST completed", ident));
}

void TestImapServer::handle_cmd_append(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    static const RegularExpression matchAppend(R"(\"?(\w+)\"?\s+.*\{(\d+)\})");
    if (auto matches = matchAppend.m(data))
    {
        reply(socketReader, "+ Ready for literal data");
        const auto bytes = stoi(matches[1].value);
        Buffer     message(bytes);
        socketReader->read(message, bytes);
        COUT(message.c_str());
        reply(socketReader, format("{} OK APPEND completed", ident));
    }
    else
    {
        reply(socketReader, format("{} NO Invalid APPEND command", ident));
    }
}

void TestImapServer::handle_cmd_fetch(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    if (data.toUpperCase().contains("(BODY[HEADER] "))
    {
        reply(socketReader, "* 1 FETCH (BODY[HEADER] {88}");
        reply(socketReader, "From: sender@example.com");
        reply(socketReader, "To: recipient@example.com");
        reply(socketReader, "Subject: Test Message");
        reply(socketReader, "");
        reply(socketReader, ")");
        reply(socketReader, format("{} OK FETCH completed", ident));
    }

    if (data.toUpperCase().contains("(BODY[] "))
    {
        reply(socketReader, "* 1 FETCH (BODY[] {142}");
        reply(socketReader, "From: sender@example.com");
        reply(socketReader, "To: recipient@example.com");
        reply(socketReader, "Subject: Test Message");
        reply(socketReader, "");
        reply(socketReader, "This is a test message body.");
        reply(socketReader, ")");
        reply(socketReader, format("{} OK FETCH completed", ident));
        return;
    }

    if (data.toUpperCase().contains("(FLAGS "))
    {
        reply(socketReader, "* 1 FETCH (FLAGS (\\Seen \\Answered))");
        reply(socketReader, format("{} OK FETCH completed", ident));
    }
}

void TestImapServer::handle_cmd_logout(const std::shared_ptr<SocketReader>& socketReader, const std::string& ident)
{
    socketReader->close();
}
