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
#include <sptk5/RegularExpression.h>
#include <sptk5/net/URL.h>

using namespace std;
using namespace sptk;

static const String testURL0 = "https://www.test.com:8080/daily/report";
static const String testURL1 = "/daily/report?action=view&id=1";
static const String testURL2 = "https://johnd:secret@www.test.com:8080/daily/report?action=view&id=1";
static const String testURL3 = "https://johnd:secret@www.test.com:8080/report?action=view&id=1";
static const String testURL4 = "https://www.test.com?action=view&id=1";
static const String testURL5 = "https://johnd@www.test.com:8080/daily/report?action=view&id=1";
static const String testURL6 = "https://www.test.com/";
namespace sptk {

TEST(URLTests,minimal) /* NOLINT */
{
    URL url(testURL0);
    EXPECT_STREQ(url.protocol().c_str(), "https");

    auto hostAndPort = url.hostAndPort();
    EXPECT_EQ(hostAndPort, tuple(String("www.test.com"), 8080));

    EXPECT_STREQ(url.username().c_str(), "");
    EXPECT_STREQ(url.password().c_str(), "");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");

    EXPECT_EQ(url.params().size(), static_cast<size_t>(0));

    EXPECT_STREQ(url.toString().c_str(), testURL0.c_str());
}

TEST(URLTests,ipv4address) /* NOLINT */
{
    URL url("https://127.0.0.1:8080/daily/report");
    EXPECT_STREQ(url.protocol().c_str(), "https");

    auto [host, port] = url.hostAndPort();
    EXPECT_STREQ(host.c_str(), "127.0.0.1");
    EXPECT_EQ(port, 8080);

    URL url2("https://127.0.0.1/daily/report");
    EXPECT_STREQ(url2.protocol().c_str(), "https");

    tie(host, port) = url2.hostAndPort();
    EXPECT_STREQ(host.c_str(), "127.0.0.1");
    EXPECT_EQ(port, 0);
}

TEST(URLTests,ipv6address) /* NOLINT */
{
    URL url("https://[2001:4860:4827:7700]:8080/daily/report");
    EXPECT_STREQ(url.protocol().c_str(), "https");

    auto [host, port] = url.hostAndPort();
    EXPECT_STREQ(host.c_str(), "2001:4860:4827:7700");
    EXPECT_EQ(port, 8080);

    URL url2("https://2001:4860:4827:7700/daily/report");
    EXPECT_STREQ(url2.protocol().c_str(), "https");

    tie(host, port) = url2.hostAndPort();
    EXPECT_STREQ(host.c_str(), "2001:4860:4827:7700");
    EXPECT_EQ(port, 0);
}

TEST(URLTests,local) /* NOLINT */
{
    URL url(testURL1);
    EXPECT_STREQ(url.protocol().c_str(), "");
    EXPECT_EQ(url.hostAndPort(), tuple("", 0));
    EXPECT_STREQ(url.username().c_str(), "");
    EXPECT_STREQ(url.password().c_str(), "");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");

    EXPECT_EQ(url.params().size(), static_cast<size_t>(2));
    EXPECT_STREQ(url.params().get("action").c_str(), "view");
    EXPECT_STREQ(url.params().get("id").c_str(), "1");

    EXPECT_STREQ(url.toString().c_str(), testURL1.c_str());
}

TEST(URLTests,all) /* NOLINT */
{
    URL url(testURL2);
    EXPECT_STREQ(url.protocol().c_str(), "https");
    EXPECT_EQ(url.hostAndPort(), tuple("www.test.com", 8080));
    EXPECT_STREQ(url.username().c_str(), "johnd");
    EXPECT_STREQ(url.password().c_str(), "secret");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");
    EXPECT_STREQ(url.location().c_str(), "/daily");

    EXPECT_EQ(url.params().size(), static_cast<size_t>(2));
    EXPECT_STREQ(url.params().get("action").c_str(), "view");
    EXPECT_STREQ(url.params().get("id").c_str(), "1");

    EXPECT_STREQ(url.toString().c_str(), testURL2.c_str());

    URL url3(testURL3);
    EXPECT_STREQ(url3.location().c_str(), "");

    URL url4(testURL4);
    EXPECT_STREQ(url4.params().get("action").c_str(), "view");
    EXPECT_STREQ(url4.params().get("id").c_str(), "1");
}

TEST(URLTests,loop)
{
    constexpr size_t numIterations = 100;
    for (size_t i = 0; i < numIterations; ++i)
    {
        URL url(testURL0);
    }
}

TEST(URLTests,usernameWithoutPasswordRoundTrip) /* NOLINT */
{
    URL url(testURL5);
    EXPECT_STREQ(url.protocol().c_str(), "https");
    EXPECT_EQ(url.hostAndPort(), tuple("www.test.com", 8080));
    EXPECT_STREQ(url.username().c_str(), "johnd");
    EXPECT_STREQ(url.password().c_str(), "");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");
    EXPECT_EQ(url.params().size(), static_cast<size_t>(2));
    EXPECT_STREQ(url.params().get("action").c_str(), "view");
    EXPECT_STREQ(url.params().get("id").c_str(), "1");

    EXPECT_STREQ(url.toString().c_str(), testURL5.c_str());
}

TEST(URLTests,rootPath) /* NOLINT */
{
    URL url(testURL6);
    EXPECT_STREQ(url.protocol().c_str(), "https");
    EXPECT_EQ(url.hostAndPort(), tuple("www.test.com", 0));
    EXPECT_STREQ(url.username().c_str(), "");
    EXPECT_STREQ(url.password().c_str(), "");
    EXPECT_STREQ(url.path().c_str(), "/");
    EXPECT_EQ(url.params().size(), static_cast<size_t>(0));
    EXPECT_STREQ(url.toString().c_str(), testURL6.c_str());
}

TEST(URLTests,rejectsTrailingJunk) /* NOLINT */
{
    EXPECT_ANY_THROW(URL("https://www.test.com/path ???"));
}

TEST(URLTests, strictPortsAndBracketedIpv6)
{
    for (const auto* url : {"https://host:65536/", "https://host:0/", "https://host:-1/",
                            "https://host:/", "https://host:12x/", "https://[::1/",
                            "https://[::1]junk/", "https://host/\r\nInjected:yes"})
        EXPECT_THROW((void) URL(url), Exception) << url;
    EXPECT_EQ(URL("https://[::1]/").hostAndPort(), std::make_tuple(std::string("::1"), uint16_t(0)));
    EXPECT_EQ(URL("https://[::1]:65535/").hostAndPort(),
              std::make_tuple(std::string("::1"), uint16_t(65535)));
}

TEST(URLTests, requestTargetPreservesRawQueryAndExcludesFragment)
{
    URL url("https://host/path?x=%2f&x=%2F&space=a+b&empty=&flag#fragment");
    EXPECT_EQ("/path?x=%2f&x=%2F&space=a+b&empty=&flag", url.requestTarget());
    EXPECT_EQ("https://host/path?x=%2f&x=%2F&space=a+b&empty=&flag#fragment", url.toString());
    EXPECT_EQ("/?x=%2f", URL("https://host?x=%2f").requestTarget());
    EXPECT_EQ("/?", URL("https://host?").requestTarget());
    EXPECT_EQ("/", URL("https://host#fragment").requestTarget());
    url.params()["new"] = "value";
    EXPECT_NE(std::string::npos, url.requestTarget().find("new=value"));
}

} // namespace sptk_test
