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

#include "test/TestData.h"

#include "TestImapServer.h"
#include <gtest/gtest.h>
#include <sptk5/net/ImapDS.h>

using namespace std;
using namespace sptk;

/// @brief Ports of this file's test IMAP servers, taken from the kernel on first use.
///
/// Each test gets its own so that a server still winding down cannot block the next one.
/// See TestData::freePort() for why hard-coded numbers are not reliable here.
uint16_t testImapDSOpenPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

uint16_t testImapDSFetchBodyPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

uint16_t testImapDSMessageIDPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

uint16_t testImapDSCallbackPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

uint16_t testImapDSBadLoginPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

uint16_t testImapDSEmptyFolderPort()
{
    static const uint16_t port = TestData::freePort();
    return port;
}

namespace {

size_t countMessages(ImapDS& imapDS)
{
    auto messageCount = 0;
    while (!imapDS.eof())
    {
        ++messageCount;
        imapDS.next();
    }
    return messageCount;
}

} // namespace

namespace sptk {

TEST(ImapDSTests, open_fetch_headers)
{
    TestImapServer testImapServer(testImapDSOpenPort());

    ImapDS imapDS;
    imapDS.host({"localhost", testImapDSOpenPort()});
    imapDS.user("user");
    imapDS.password("password");
    imapDS.folder("INBOX");
    imapDS.fetchBody(false);

    EXPECT_TRUE(imapDS.open());
    EXPECT_FALSE(imapDS.eof());

    // Verify first message has msg_id field
    if (!imapDS.eof())
    {
        const auto msgIdField = imapDS.current()->fieldByName("msg_id");
        EXPECT_NE(nullptr, msgIdField);
        if (msgIdField)
        {
            EXPECT_EQ(1, msgIdField->asInteger());
        }
    }

    // TestImapServer returns 5 messages
    constexpr auto expectedMessageCount = 5;
    EXPECT_EQ(expectedMessageCount, countMessages(imapDS));
}

TEST(ImapDSTests, open_fetch_body)
{
    TestImapServer testImapServer(testImapDSFetchBodyPort());

    ImapDS imapDS;
    imapDS.host({"localhost", testImapDSFetchBodyPort()});
    imapDS.user("user");
    imapDS.password("password");
    imapDS.folder("INBOX");
    imapDS.fetchBody(true);

    EXPECT_TRUE(imapDS.open());
    EXPECT_FALSE(imapDS.eof());

    // TestImapServer returns 5 messages
    constexpr auto expectedMessageCount = 5;
    EXPECT_EQ(expectedMessageCount, countMessages(imapDS));
}

TEST(ImapDSTests, open_specific_message_id)
{
    TestImapServer testImapServer(testImapDSMessageIDPort());

    ImapDS imapDS;
    imapDS.host({"localhost", testImapDSMessageIDPort()});
    imapDS.user("user");
    imapDS.password("password");
    imapDS.folder("INBOX");
    imapDS.messageID(3);
    imapDS.fetchBody(false);

    EXPECT_TRUE(imapDS.open());
    EXPECT_FALSE(imapDS.eof());

    // Verify it's message ID 3
    if (!imapDS.eof())
    {
        const auto msgIdField = imapDS.current()->fieldByName("msg_id");
        EXPECT_NE(nullptr, msgIdField);
        if (msgIdField)
        {
            EXPECT_EQ(3, msgIdField->asInteger());
        }
    }

    // Should fetch only one message (message ID 3)
    EXPECT_EQ(1, countMessages(imapDS));
}

TEST(ImapDSTests, callback_functionality)
{
    TestImapServer testImapServer(testImapDSCallbackPort());

    ImapDS imapDS;
    imapDS.host({"localhost", testImapDSCallbackPort()});
    imapDS.user("user");
    imapDS.password("password");
    imapDS.folder("INBOX");
    imapDS.fetchBody(false);

    int callbackCallCount = 0;
    int lastTotal = 0;
    int lastProgress = 0;

    imapDS.callback([&](const int total, const int progress)
                    {
                        callbackCallCount++;
                        lastTotal = total;
                        lastProgress = progress;
                    });

    EXPECT_TRUE(imapDS.open());

    // Callback should be called: once at start (0), once per message (5), once at end (5) = 7 times
    EXPECT_GT(callbackCallCount, 0);
    EXPECT_EQ(lastTotal, lastProgress); // Last callback should have progress == total
}

TEST(ImapDSTests, bad_login)
{
    TestImapServer testImapServer(testImapDSBadLoginPort());

    ImapDS imapDS;
    imapDS.host({"localhost", testImapDSBadLoginPort()});
    imapDS.user("user");
    imapDS.password("wrongpassword");
    imapDS.folder("INBOX");

    try
    {
        imapDS.open();
        FAIL() << "Must fail: Bad password";
    }
    catch (const Exception& e)
    {
        COUT("Login failed as expected: " << e.message());
    }
}

TEST(ImapDSTests, getters_setters)
{
    ImapDS imapDS;

    const Host testHost("localhost", 143);
    imapDS.host(testHost);
    EXPECT_EQ("localhost", imapDS.host().hostname());
    EXPECT_EQ(143, imapDS.host().port());

    imapDS.user("testuser");
    EXPECT_EQ("testuser", imapDS.user());

    imapDS.password("testpass");
    EXPECT_EQ("testpass", imapDS.password());

    imapDS.folder("TestFolder");
    EXPECT_EQ("TestFolder", imapDS.folder());

    imapDS.messageID(42);
    EXPECT_EQ(42, imapDS.messageID());

    imapDS.fetchBody(true);
    EXPECT_TRUE(imapDS.fetchBody());

    imapDS.fetchBody(false);
    EXPECT_FALSE(imapDS.fetchBody());
}

TEST(ImapDSTests, iteration_through_messages)
{
    TestImapServer testImapServer(testImapDSEmptyFolderPort());

    ImapDS imapDS;
    imapDS.host({"localhost", testImapDSEmptyFolderPort()});
    imapDS.user("user");
    imapDS.password("password");
    imapDS.folder("INBOX");
    imapDS.fetchBody(false);

    EXPECT_TRUE(imapDS.open());

    // Iterate through all messages
    int messageCount = 0;
    for (imapDS.first(); !imapDS.eof(); imapDS.next())
    {
        messageCount++;
        auto msgIdField = imapDS.current()->fieldByName("msg_id");
        EXPECT_NE(nullptr, msgIdField);
        if (msgIdField)
        {
            EXPECT_EQ(messageCount, msgIdField->asInteger());
        }
    }

    constexpr auto expectedMessageCount = 5;
    EXPECT_EQ(expectedMessageCount, messageCount);
}

} // namespace sptk
