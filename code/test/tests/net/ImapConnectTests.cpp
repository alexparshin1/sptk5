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
#include "sptk5/net/ImapConnect.h"
#include <gtest/gtest.h>

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

using namespace std;
using namespace sptk;

constexpr auto testImapServerPort = 12346;
constexpr auto testImapServerSelectPort = 12347;
constexpr auto testImapServerExaminePort = 12348;
constexpr auto testImapServerCapabilityPort = 12349;
constexpr auto testImapServerSubscribePort = 12350;
constexpr auto testImapServerUnsubscribePort = 12351;
constexpr auto testImapServerCreatePort = 12352;
constexpr auto testImapServerDeletePort = 12353;
constexpr auto testImapServerRenamePort = 12354;
constexpr auto testImapServerListPort = 12355;
constexpr auto testImapServerAppendPort = 12356;
constexpr auto testImapServerFetchMessagePort = 12357;
constexpr auto testImapServerFetchHeadersPort = 12358;
constexpr auto testImapServerFetchFlagsPort = 12359;

namespace sptk {

TEST(ImapConnectTests, login)
{
    TestImapServer testImapServer(testImapServerPort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerPort});

    imap.cmd_login("user", "password");
    imap.cmd_close();

    try
    {
        imap.cmd_login("user", "password2");
        FAIL() << "Must fail: Bad password";
    }
    catch (const Exception& e)
    {
        COUT("Login failed, the response is: " << e.message());
    }
}

TEST(ImapConnectTests, select)
{
    TestImapServer testImapServer(testImapServerSelectPort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerSelectPort});

    imap.cmd_login("user", "password");

    auto           totalMessages = 0;
    constexpr auto expectedMessageCount = 5; // Encoded in TestImapServer class.
    imap.cmd_select("INBOX", totalMessages);
    EXPECT_EQ(expectedMessageCount, totalMessages);

    imap.cmd_close();
}

TEST(ImapConnectTests, capability)
{
    TestImapServer testImapServer(testImapServerCapabilityPort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerCapabilityPort});

    imap.cmd_login("user", "password");
    imap.cmd_capability();

    auto hasImap4rev1 = false;
    for (const auto& line: imap.response())
    {
        if (line.find("IMAP4rev1") != std::string::npos)
        {
            hasImap4rev1 = true;
            break;
        }
    }
    EXPECT_TRUE(hasImap4rev1);

    imap.cmd_close();
}

TEST(ImapConnectTests, examine)
{
    TestImapServer testImapServer(testImapServerExaminePort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerExaminePort});

    imap.cmd_login("user", "password");
    imap.cmd_examine("INBOX");

    auto readOnly = false;
    for (const auto& line: imap.response())
    {
        if (line.find("READ-ONLY") != std::string::npos)
        {
            readOnly = true;
            break;
        }
    }
    EXPECT_TRUE(readOnly);

    imap.cmd_close();
}

TEST(ImapConnectTests, subscribe)
{
    TestImapServer testImapServer(testImapServerSubscribePort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerSubscribePort});

    imap.cmd_login("user", "password");
    imap.cmd_subscribe("TestFolder");

    auto subscribed = false;
    for (const auto& line: imap.response())
    {
        if (line.find("OK SUBSCRIBE completed") != std::string::npos)
        {
            subscribed = true;
            break;
        }
    }
    EXPECT_TRUE(subscribed);

    imap.cmd_close();
}

TEST(ImapConnectTests, unsubscribe)
{
    TestImapServer testImapServer(testImapServerUnsubscribePort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerUnsubscribePort});

    imap.cmd_login("user", "password");
    imap.cmd_unsubscribe("TestFolder");

    auto unsubscribed = false;
    for (const auto& line: imap.response())
    {
        if (line.find("OK UNSUBSCRIBE completed") != std::string::npos)
        {
            unsubscribed = true;
            break;
        }
    }
    EXPECT_TRUE(unsubscribed);

    imap.cmd_close();
}

TEST(ImapConnectTests, create)
{
    TestImapServer testImapServer(testImapServerCreatePort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerCreatePort});

    imap.cmd_login("user", "password");
    imap.cmd_create("NewFolder");

    auto created = false;
    for (const auto& line: imap.response())
    {
        if (line.find("OK CREATE completed") != std::string::npos)
        {
            created = true;
            break;
        }
    }
    EXPECT_TRUE(created);

    imap.cmd_close();
}

TEST(ImapConnectTests, deleteMailbox)
{
    TestImapServer testImapServer(testImapServerDeletePort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerDeletePort});

    imap.cmd_login("user", "password");
    imap.cmd_delete("OldFolder");

    auto deleted = false;
    for (const auto& line: imap.response())
    {
        if (line.find("OK DELETE completed") != std::string::npos)
        {
            deleted = true;
            break;
        }
    }
    EXPECT_TRUE(deleted);

    imap.cmd_close();
}

TEST(ImapConnectTests, rename)
{
    TestImapServer testImapServer(testImapServerRenamePort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerRenamePort});

    imap.cmd_login("user", "password");
    imap.cmd_rename("OldName", "NewName");

    auto renamed = false;
    for (const auto& line: imap.response())
    {
        if (line.find("OK RENAME completed") != std::string::npos)
        {
            renamed = true;
            break;
        }
    }
    EXPECT_TRUE(renamed);

    imap.cmd_close();
}

TEST(ImapConnectTests, list)
{
    TestImapServer testImapServer(testImapServerListPort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerListPort});

    imap.cmd_login("user", "password");
    imap.cmd_list("*");

    auto foundInbox = false;
    for (const auto& line: imap.response())
    {
        if (line.find("INBOX") != std::string::npos)
        {
            foundInbox = true;
            break;
        }
    }
    EXPECT_TRUE(foundInbox);

    imap.cmd_close();
}

TEST(ImapConnectTests, append)
{
    TestImapServer testImapServer(testImapServerAppendPort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerAppendPort});

    imap.cmd_login("user", "password");

    const Buffer message("From: test@example.com\r\nTo: recipient@example.com\r\nSubject: Test\r\n\r\nTest message body");
    imap.cmd_append("INBOX", message);

    auto appended = false;
    for (const auto& line: imap.response())
    {
        if (line.find("OK APPEND completed") != std::string::npos)
        {
            appended = true;
            break;
        }
    }
    EXPECT_TRUE(appended);

    imap.cmd_close();
}

TEST(ImapConnectTests, fetch_message)
{
    TestImapServer testImapServer(testImapServerFetchMessagePort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerFetchMessagePort});

    imap.cmd_login("user", "password");

    FieldList result(true);
    imap.cmd_fetch_message(1, result);

    auto foundMessage = false;
    for (const auto& line: imap.response())
    {
        if (line.find("FETCH") != std::string::npos || line.find("From:") != std::string::npos)
        {
            foundMessage = true;
            break;
        }
    }
    EXPECT_TRUE(foundMessage);

    imap.cmd_close();
}

TEST(ImapConnectTests, fetch_headers)
{
    TestImapServer testImapServer(testImapServerFetchHeadersPort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerFetchHeadersPort});

    imap.cmd_login("user", "password");

    FieldList result(true);
    imap.cmd_fetch_headers(1, result);

    auto foundHeaders = false;
    for (const auto& line: imap.response())
    {
        if (line.find("FETCH") != std::string::npos || line.find("HEADER") != std::string::npos)
        {
            foundHeaders = true;
            break;
        }
    }
    EXPECT_TRUE(foundHeaders);

    imap.cmd_close();
}

TEST(ImapConnectTests, fetch_flags)
{
    TestImapServer testImapServer(testImapServerFetchFlagsPort);

    ImapConnect imap;
    imap.host({"localhost", testImapServerFetchFlagsPort});

    imap.cmd_login("user", "password");

    const String flags = imap.cmd_fetch_flags(1);

    EXPECT_FALSE(flags.empty());
    EXPECT_TRUE(flags.find("Seen") != std::string::npos || flags.find("Answered") != std::string::npos);

    imap.cmd_close();
}

} // namespace sptk
