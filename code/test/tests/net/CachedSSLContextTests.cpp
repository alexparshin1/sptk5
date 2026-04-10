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

#include <gtest/gtest.h>
#include <sptk5/net/CachedSSLContext.h>

using namespace std;
using namespace sptk;
namespace sptk {

TEST(CachedSSLContextTests,sameParamsShareContext)
{
    const SSLKeys keys("", "", "", "", 1, 2);

    const auto context1 = CachedSSLContext::get(keys, "ALL", true);
    const auto context2 = CachedSSLContext::get(keys, "ALL", true);

    ASSERT_NE(nullptr, context1.get());
    EXPECT_EQ(context1.get(), context2.get());
}

TEST(CachedSSLContextTests,tlsOnlyCreatesDistinctContext)
{
    const SSLKeys keys("", "", "", "", 3, 4);

    const auto tlsOnlyContext = CachedSSLContext::get(keys, "ALL", true);
    const auto tlsAndSslContext = CachedSSLContext::get(keys, "ALL", false);

    ASSERT_NE(nullptr, tlsOnlyContext.get());
    ASSERT_NE(nullptr, tlsAndSslContext.get());
    EXPECT_NE(tlsOnlyContext.get(), tlsAndSslContext.get());
}

TEST(CachedSSLContextTests,differentKeysCreateDistinctContexts)
{
    const SSLKeys keys1("", "", "", "", 5, 6);
    const SSLKeys keys2("", "", "", "", 7, 8);

    const auto context1 = CachedSSLContext::get(keys1, "ALL", true);
    const auto context2 = CachedSSLContext::get(keys2, "ALL", true);

    ASSERT_NE(nullptr, context1.get());
    ASSERT_NE(nullptr, context2.get());
    EXPECT_NE(context1.get(), context2.get());
}

} // namespace sptk_test
