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

#include <gtest/gtest.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/net/URL.h>

using namespace std;
using namespace sptk;

static const String testURL0 = "https://www.test.com:8080/daily/report";
static const String testURL1 = "/daily/report?action=view&id=1";
static const String testURL2 = "https://johnd:secret@www.test.com:8080/daily/report?action=view&id=1";
static const String testURL3 = "https://johnd:secret@www.test.com:8080/report?action=view&id=1";

TEST(SPTK_URL, minimal) /* NOLINT */
{
    URL url(testURL0);
    EXPECT_STREQ(url.protocol().c_str(), "https");
    EXPECT_STREQ(url.hostAndPort().c_str(), "www.test.com:8080");
    EXPECT_STREQ(url.username().c_str(), "");
    EXPECT_STREQ(url.password().c_str(), "");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");

    EXPECT_EQ(url.params().size(), size_t(0));

    EXPECT_STREQ(url.toString().c_str(), testURL0.c_str());
}

TEST(SPTK_URL, local) /* NOLINT */
{
    URL url(testURL1);
    EXPECT_STREQ(url.protocol().c_str(), "");
    EXPECT_STREQ(url.hostAndPort().c_str(), "");
    EXPECT_STREQ(url.username().c_str(), "");
    EXPECT_STREQ(url.password().c_str(), "");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");

    EXPECT_EQ(url.params().size(), size_t(2));
    EXPECT_STREQ(url.params().get("action").c_str(), "view");
    EXPECT_STREQ(url.params().get("id").c_str(), "1");

    EXPECT_STREQ(url.toString().c_str(), testURL1.c_str());
}

TEST(SPTK_URL, all) /* NOLINT */
{
    URL url(testURL2);
    EXPECT_STREQ(url.protocol().c_str(), "https");
    EXPECT_STREQ(url.hostAndPort().c_str(), "www.test.com:8080");
    EXPECT_STREQ(url.username().c_str(), "johnd");
    EXPECT_STREQ(url.password().c_str(), "secret");
    EXPECT_STREQ(url.path().c_str(), "/daily/report");
    EXPECT_STREQ(url.location().c_str(), "/daily");

    EXPECT_EQ(url.params().size(), size_t(2));
    EXPECT_STREQ(url.params().get("action").c_str(), "view");
    EXPECT_STREQ(url.params().get("id").c_str(), "1");

    EXPECT_STREQ(url.toString().c_str(), testURL2.c_str());

    URL url3(testURL3);
    EXPECT_STREQ(url3.location().c_str(), "");
}

TEST(SPTK_URL, loop)
{
    constexpr size_t numIterations = 100;
    for (size_t i = 0; i < numIterations; ++i)
    {
        URL url(testURL0);
    }
}
