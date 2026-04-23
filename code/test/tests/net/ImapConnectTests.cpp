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
#include "sptk5/net/SocketEvents.h"
#include "sptk5/net/SocketReader.h"
#include <gtest/gtest.h>
#include <sptk5/cutils>
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>

#ifndef _WIN32
#include <netinet/tcp.h>
#endif

using namespace std;
using namespace sptk;

constexpr auto testImapServerPort = 12346;

namespace {

shared_ptr<TestImapServer> testImapServer;

} // namespace

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

    this_thread::sleep_for(1s);
}

} // namespace sptk
