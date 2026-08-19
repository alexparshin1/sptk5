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

#include <sptk5/RegularExpression.h>
#include <sptk5/net/Socket.h>

#include <utility>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

static constexpr uint16_t sshPort = 22;
static constexpr uint16_t telnetPort = 23;
static constexpr uint16_t httpPort = 80;
static const String       testHost("www.google.com:80");
namespace sptk {

TEST(HostTests,ctorHostname)
{
    const Host google1(testHost);
    EXPECT_STREQ(testHost.c_str(), google1.toString(false).c_str());
    EXPECT_STREQ("www.google.com", google1.hostname().c_str());
    EXPECT_EQ(httpPort, google1.port());

    const Host google(google1.toString(true));
    EXPECT_TRUE(google1 == google);
}

TEST(HostTests,ctorAddress)
{
    const Host host("11.22.33.44", sshPort);
    EXPECT_STREQ("11.22.33.44", host.hostname().c_str());
    EXPECT_EQ(sshPort, host.port());
}

TEST(HostTests,ctorCopy)
{
    const Host host1("11.22.33.44", sshPort);
    const Host host2(host1);
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(sshPort, host2.port());
}

TEST(HostTests,ctorMove)
{
    Host       host1("11.22.33.44", sshPort);
    const Host host2(std::move(host1));
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(sshPort, host2.port());
}

TEST(HostTests,assign)
{
    const Host host1("11.22.33.44", sshPort);
    const Host host2 = host1;
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(sshPort, host2.port());
}

TEST(HostTests,move)
{
    Host       host1("11.22.33.44", sshPort);
    const Host host2 = std::move(host1);
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(sshPort, host2.port());
}

TEST(HostTests,compare)
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

TEST(HostTests,ctorHostOnly)
{
    ASSERT_NO_THROW({
        const Host host("127.0.0.1");
        EXPECT_STREQ("127.0.0.1", host.hostname().c_str());
        EXPECT_EQ(0, host.port());
        EXPECT_STREQ("127.0.0.1:0", host.toString(false).c_str());
    });
}

#ifndef _WIN32
TEST(HostTests,ctorIpv6Bracketed)
{
    const Host host("[::1]:443");
    EXPECT_STREQ("::1", host.hostname().c_str());
    EXPECT_EQ(443, host.port());
    EXPECT_STREQ("[::1]:443", host.toString(false).c_str());
}

TEST(HostTests,ctorIpv6WithoutPort)
{
    ASSERT_NO_THROW({
        const Host host("[::1]");
        EXPECT_STREQ("::1", host.hostname().c_str());
        EXPECT_EQ(0, host.port());
        EXPECT_STREQ("[::1]:0", host.toString(false).c_str());
    });
}
#endif

} // namespace sptk_test
