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

#include <filesystem>
#include <fstream>
#include <thread>

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

TEST(CachedSSLContextTests, replacedKeyFileCreatesDistinctContext)
{
    // Renewal writes new content to the path that was always there. A cache that recognised keys
    // by their file names alone would hand out the context built from the certificate that has
    // just been replaced, for the rest of the process's life.
    const auto keyFile = filesystem::temp_directory_path() /
                         ("sptk_sslkeys_" + to_string(::getpid()) + ".pem");

    ofstream(keyFile) << "first";
    // Named as the CA file: it counts towards the stamp like the other two, and is the one of the
    // three that does not send the context off to parse a certificate it does not need here.
    const SSLKeys keys("", "", "", keyFile, 9, 10);
    const auto    beforeReplacing = CachedSSLContext::get(keys, "ALL", true);
    ASSERT_NE(nullptr, beforeReplacing.get());

    ofstream(keyFile) << "second, and a different length";

    // Past the interval within which a cached context is handed out without looking at its files.
    // Inside it the old context is the right answer, and this test would be asserting that the
    // cache does not work.
    this_thread::sleep_for(chrono::milliseconds(1100));

    const auto afterReplacing = CachedSSLContext::get(keys, "ALL", true);
    ASSERT_NE(nullptr, afterReplacing.get());
    EXPECT_NE(beforeReplacing.get(), afterReplacing.get());

    error_code errorCode;
    filesystem::remove(keyFile, errorCode);
}

TEST(CachedSSLContextTests, unchangedKeyFileKeepsSharingContext)
{
    // The other half of it: a file that has not been touched must not cost a new context, which
    // is the whole reason the cache is there. Checked after the recheck interval has passed, so
    // that what holds is the comparison of the files and not merely the interval.
    const auto keyFile = filesystem::temp_directory_path() /
                         ("sptk_sslkeys_unchanged_" + to_string(::getpid()) + ".pem");

    ofstream(keyFile) << "unchanging";
    const SSLKeys keys("", "", "", keyFile, 11, 12);

    const auto first = CachedSSLContext::get(keys, "ALL", true);
    const auto within = CachedSSLContext::get(keys, "ALL", true);
    this_thread::sleep_for(chrono::milliseconds(1100));
    const auto second = CachedSSLContext::get(keys, "ALL", true);

    ASSERT_NE(nullptr, first.get());
    EXPECT_EQ(first.get(), within.get());
    EXPECT_EQ(first.get(), second.get());

    error_code errorCode;
    filesystem::remove(keyFile, errorCode);
}

} // namespace sptk_test
