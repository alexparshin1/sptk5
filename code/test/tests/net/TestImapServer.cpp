/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       XMQ Message QUEUE                                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "TestImapServer.h"

#include <netinet/tcp.h>
#include <ranges>
#include <sptk5/cnet>

using namespace std;
using namespace sptk;

namespace {

const map<string, TestImapServer::Command, less<>> commandMap = {
    {"login", TestImapServer::Command::Login},
    {"select", TestImapServer::Command::Select},
    {"examine", TestImapServer::Command::Examine},
    {"close", TestImapServer::Command::Close}};

}

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
        COUT("Received: " << incomingData);
        if (const auto matches = matchCommand.m(incomingData))
        {
            const auto command = commandMap.find(matches[1].value);
            if (command == commandMap.end())
            {
                reply(socketReader, format("{} BAD Unknown command\r\n", matches[1].value));
                return;
            }

            switch (command->second)
            {
                using enum Command;
                case Login:
                    handle_cmd_login(socketReader, matches[0].value, matches[2].value);
                    break;
                case Select:
                    handle_cmd_select(socketReader, matches[0].value);
                    break;
                case Examine:
                    handle_cmd_examine(socketReader, matches[0].value);
                    break;
                case Close:
                    handle_cmd_close(matches[0].value, socketReader);
                    break;
            }
        }
    }
}

void TestImapServer::reply(const shared_ptr<SocketReader>& socketReader, const std::string& response)
{
    socketReader->socket()->write(response);
    COUT("Reply:    " << response);
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

void TestImapServer::handle_cmd_close(const std::string& ident, const shared_ptr<SocketReader>& socketReader)
{
    reply(socketReader, format("{} OK CLOSE completed\r\n", ident));
    socketReader->close();
}

void TestImapServer::imapSession(const ServerConnection& socket)
{
    const auto clientSocket = make_shared<TCPSocket>();
    const auto clientReader = make_shared<SocketReader>(clientSocket);

    clientSocket->attach(socket.getSocket()->detach(), false);
    clientSocket->setOption(IPPROTO_TCP, TCP_NODELAY, 1);
    clientSocket->blockingMode(false);

    // IMAP server greeting
    clientSocket->write("* OK TestIMAP4rev1 Service Ready\r\n");

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
