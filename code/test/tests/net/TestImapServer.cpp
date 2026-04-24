/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include "TestImapServer.h"

#include <netinet/tcp.h>
#include <sptk5/cnet>

using namespace std;
using namespace sptk;

namespace {

enum class Command
{
    Capability,
    Login,
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

} // namespace

TestImapServer::TestImapServer(uint16_t port)
    : TCPServer("Test IMAP server")
{
    addListener(ServerConnection::Type::TCP, {"localhost", port});
    onConnection([](const ServerConnection& socket)
                 {
                     imapSession(socket);
                 });
}

void TestImapServer::readIncomingData(const shared_ptr<SocketReader>& socketReader)
{
    if (String incomingData;
        socketReader->readLine(incomingData))
    {
        static const RegularExpression matchCommand(R"(^(a\d+) (\w+)\s?([^\r]+)?[\r]*$)");
        COUT("\nReceived: " << incomingData);
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
    socketReader->socket()->write(response);
    COUT("Reply:    " << trim(response));
}

void TestImapServer::handle_cmd_capability(const shared_ptr<SocketReader>& socketReader, const string& ident)
{
    reply(socketReader, "* CAPABILITY IMAP4rev1 AUTH=PLAIN AUTH=LOGIN\r\n");
    reply(socketReader, format("{} OK CAPABILITY completed\r\n", ident));
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
        reply(socketReader, format("{} NO Authentication failed: Invalid credentials\r\n", ident));
        return;
    }
    reply(socketReader, format("{} OK LOGIN completed\r\n", ident));
}

void TestImapServer::handle_cmd_select(const shared_ptr<SocketReader>& socketReader, const string& ident)
{
    reply(socketReader, "* 5 EXISTS\r\n");
    reply(socketReader, "* 1 RECENT\r\n");
    reply(socketReader, "* OK [UNSEEN 1] First unseen message\r\n");
    reply(socketReader, "* OK [UIDVALIDITY 1] UIDs valid\r\n");
    reply(socketReader, "* OK [UIDNEXT 6] Predicted next UID\r\n");
    reply(socketReader, "* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)\r\n");
    reply(socketReader, "* OK [PERMANENTFLAGS (\\Deleted \\Seen \\*)] Flags permitted.\r\n");
    reply(socketReader, format("{} OK [READ-WRITE] SELECT completed\r\n", ident));
}

void TestImapServer::handle_cmd_examine(const shared_ptr<SocketReader>& socketReader, const string& ident)
{
    reply(socketReader, "* 5 EXISTS\r\n");
    reply(socketReader, "* 1 RECENT\r\n");
    reply(socketReader, "* OK [UNSEEN 1] First unseen message\r\n");
    reply(socketReader, "* OK [UIDVALIDITY 1] UIDs valid\r\n");
    reply(socketReader, "* OK [PERMANENTFLAGS ()] No permanent flags permitted\r\n");
    reply(socketReader, "* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)\r\n");
    reply(socketReader, format("{} OK [READ-ONLY] EXAMINE completed\r\n", ident));
}

void TestImapServer::handle_cmd_close(const shared_ptr<SocketReader>& socketReader, const std::string& ident)
{
    reply(socketReader, format("{} OK CLOSE completed\r\n", ident));
    socketReader->close();
}

void TestImapServer::handle_cmd_subscribe(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    reply(socketReader, format("{} OK SUBSCRIBE completed\r\n", ident));
}

void TestImapServer::handle_cmd_unsubscribe(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    reply(socketReader, format("{} OK UNSUBSCRIBE completed\r\n", ident));
}

void TestImapServer::handle_cmd_create(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    reply(socketReader, format("{} OK CREATE completed\r\n", ident));
}

void TestImapServer::handle_cmd_delete(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    reply(socketReader, format("{} OK DELETE completed\r\n", ident));
}

void TestImapServer::handle_cmd_rename(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    reply(socketReader, format("{} OK RENAME completed\r\n", ident));
}

void TestImapServer::handle_cmd_list(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    reply(socketReader, "* LIST (\\HasNoChildren) \".\" \"INBOX\"\r\n");
    reply(socketReader, "* LIST (\\HasNoChildren) \".\" \"Sent\"\r\n");
    reply(socketReader, "* LIST (\\HasNoChildren) \".\" \"Drafts\"\r\n");
    reply(socketReader, format("{} OK LIST completed\r\n", ident));
}

void TestImapServer::handle_cmd_append(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    reply(socketReader, format("{} OK APPEND completed\r\n", ident));
}

void TestImapServer::handle_cmd_fetch(const shared_ptr<SocketReader>& socketReader, const string& ident, const String& data)
{
    if (data.toUpperCase().contains("(BODY[HEADER] "))
    {
        reply(socketReader, "* 1 FETCH (BODY[HEADER] {88}\r\n");
        reply(socketReader, "From: sender@example.com\r\n");
        reply(socketReader, "To: recipient@example.com\r\n");
        reply(socketReader, "Subject: Test Message\r\n");
        reply(socketReader, "\r\n");
        reply(socketReader, ")\r\n");
        reply(socketReader, format("{} OK FETCH completed\r\n", ident));
    }

    if (data.toUpperCase().contains("(BODY[] "))
    {
        reply(socketReader, "* 1 FETCH (BODY[] {142}\r\n");
        reply(socketReader, "From: sender@example.com\r\n");
        reply(socketReader, "To: recipient@example.com\r\n");
        reply(socketReader, "Subject: Test Message\r\n");
        reply(socketReader, "\r\n");
        reply(socketReader, "This is a test message body.\r\n");
        reply(socketReader, ")\r\n");
        reply(socketReader, format("{} OK FETCH completed\r\n", ident));
        return;
    }

    if (data.toUpperCase().contains("(FLAGS "))
    {
        reply(socketReader, "* 1 FETCH (FLAGS (\\Seen \\Answered))\r\n");
        reply(socketReader, format("{} OK FETCH completed\r\n", ident));
    }
}

void TestImapServer::imapSession(const ServerConnection& socket)
{
    const auto clientSocket = make_shared<TCPSocket>();
    const auto clientReader = make_shared<SocketReader>(clientSocket);

    clientSocket->attach(socket.getSocket()->detach(), false);
    clientSocket->setOption(IPPROTO_TCP, TCP_NODELAY, 1);
    clientSocket->blockingMode(false);

    // IMAP server greeting
    clientSocket->write("* OK IMAP4rev1 Service Ready\r\n");

    while (clientSocket->active())
    {
        if (clientReader->readyToRead(1s))
        {
            if (clientReader->availableBytes() == 0)
            {
                clientReader->close();
                break;
            }
            readIncomingData(clientReader);
        }
    }
}
