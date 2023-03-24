/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/Socket.h>

#include <utility>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

static constexpr uint16_t sshPort = 22;
static constexpr uint16_t telnetPort = 23;
static constexpr uint16_t httpPort = 80;
static const String testHost("www.google.com:80");

TEST(SPTK_Host, ctorHostname)
{
    const Host google1(testHost);
    EXPECT_STREQ(testHost.c_str(), google1.toString(false).c_str());
    EXPECT_STREQ("www.google.com", google1.hostname().c_str());
    EXPECT_EQ(httpPort, google1.port());

    const Host google(google1.toString(true));
    EXPECT_TRUE(google1 == google);
}

TEST(SPTK_Host, ctorAddress)
{
    const Host host("11.22.33.44", sshPort);
    EXPECT_STREQ("11.22.33.44", host.hostname().c_str());
    EXPECT_EQ(sshPort, host.port());
}

TEST(SPTK_Host, ctorAddressStruct)
{
    const String testHostAndPort {"bitbucket.com:80"};
    const Host host1(testHostAndPort);

    sockaddr_in address {};
    host1.getAddress(address);
    const Host host2(&address);

    EXPECT_STREQ(host1.toString(true).c_str(), host2.toString(true).c_str());
    EXPECT_STREQ(testHostAndPort.c_str(), host2.toString(false).c_str());
    EXPECT_EQ(host1.port(), host2.port());
}

TEST(SPTK_Host, ctorCopy)
{
    const Host host1("11.22.33.44", sshPort);
    const Host host2(host1);
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(sshPort, host2.port());
}

TEST(SPTK_Host, ctorMove)
{
    Host host1("11.22.33.44", sshPort);
    const Host host2(std::move(host1));
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(sshPort, host2.port());
}

TEST(SPTK_Host, assign)
{
    const Host host1("11.22.33.44", sshPort);
    const Host host2 = host1;
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(sshPort, host2.port());
}

TEST(SPTK_Host, move)
{
    Host host1("11.22.33.44", sshPort);
    const Host host2 = std::move(host1);
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(sshPort, host2.port());
}

TEST(SPTK_Host, compare)
{
    const Host host1("11.22.33.44", sshPort);
    const Host host2(host1);
    const Host host3("11.22.33.45", sshPort);
    const Host host4("11.22.33.44", telnetPort);

    EXPECT_TRUE(host1 == host2);
    EXPECT_FALSE(host1 != host2);

    EXPECT_FALSE(host1 == host3);
    EXPECT_TRUE(host1 != host3);

    EXPECT_FALSE(host1 == host4);
    EXPECT_TRUE(host1 != host4);
}
