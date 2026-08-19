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

#include <gtest/gtest.h>
#include <sptk5/net/Proxy.h>

using namespace std;
using namespace sptk;

class TestProxy final : public Proxy
{
public:
    using Proxy::Proxy;

    SocketType connect(const Host& /*destination*/, bool /*blockingMode*/,
                       const std::chrono::milliseconds& /*timeout*/) override
    {
        return SocketType {};
    }

    const Host& host() const
    {
        return m_host;
    }

    const String& username() const
    {
        return m_username;
    }

    const String& password() const
    {
        return m_password;
    }
};
namespace sptk {

TEST(ProxyTests,storesHostAndDefaultCredentials)
{
    // A web server is expected to run locally
    const Host      host("localhost", 80);
    const TestProxy proxy(host);

    EXPECT_TRUE(proxy.host() == host);
    EXPECT_TRUE(proxy.username().empty());
    EXPECT_TRUE(proxy.password().empty());
}

TEST(ProxyTests,storesProvidedCredentials)
{
    // A web server is expected to run locally
    const Host      host("localhost", 80);
    const TestProxy proxy(host, "user1", "pass1");

    EXPECT_TRUE(proxy.host() == host);
    EXPECT_STREQ(proxy.username().c_str(), "user1");
    EXPECT_STREQ(proxy.password().c_str(), "pass1");
}

} // namespace sptk_test
