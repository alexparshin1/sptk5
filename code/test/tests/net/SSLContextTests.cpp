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

#include <future>
#include <gtest/gtest.h>
#include <openssl/ssl.h>
#include <sptk5/cnet>
#include <sptk5/net/SSLContext.h>

using namespace std;
using namespace sptk;
using namespace chrono;
namespace sptk {

TEST(SSLContextTests,ctorCreatesContextAndHandleIsNotNull)
{
    SSLContext ctx("ALL", true);
    EXPECT_NE(nullptr, ctx.handle());
}

TEST(SSLContextTests,ctorWithInvalidCipherListThrows)
{
    EXPECT_THROW(
        {
            SSLContext ctx("THIS-CIPHER-LIST-SHOULD-NOT-EXIST", true);
            (void) ctx;
        },
        Exception);
}

TEST(SSLContextTests,tlsOnlySetsMinProtocolVersionToTls11)
{
    SSLContext ctx("ALL", true);
    SSL_CTX*   raw = ctx.handle();
    ASSERT_NE(nullptr, raw);

#if defined(SSL_CTX_get_min_proto_version) && defined(TLS1_1_VERSION)
    const auto minVer = SSL_CTX_get_min_proto_version(raw);
    EXPECT_EQ(TLS1_1_VERSION, minVer);
#else
    GTEST_SKIP() << "OpenSSL does not expose SSL_CTX_get_min_proto_version/TLS1_1_VERSION in this build.";
#endif
}

TEST(SSLContextTests,loadKeysValidFilesDoesNotThrow)
{
    const auto    keyFile = TestData::SslKeysDirectory() / "test.key";
    const auto    certFile = TestData::SslKeysDirectory() / "test.cert";
    const SSLKeys keys(keyFile, certFile);

    if (!filesystem::exists(keys.privateKeyFileName()) || !filesystem::exists(keys.certificateFileName()))
    {
        GTEST_SKIP() << "Test key/certificate files are missing in the test data directory.";
    }

    SSLContext ctx("ALL", true);
    EXPECT_NO_THROW(ctx.loadKeys(keys));
}

TEST(SSLContextTests,loadKeysMissingCertificateThrows)
{
    const auto    missingKey = TestData::SslKeysDirectory() / "missing.key";
    const auto    missingCert = TestData::SslKeysDirectory() / "missing.cert";
    const SSLKeys keys(missingKey, missingCert);

    SSLContext ctx("ALL", true);
    EXPECT_THROW(ctx.loadKeys(keys), Exception);
}

TEST(SSLContextTests,loadKeysMissingKeyThrows)
{
    const auto    missingKey = TestData::SslKeysDirectory() / "missing.key";
    const auto    certFile = TestData::SslKeysDirectory() / "test.cert";
    const SSLKeys keys(missingKey, certFile);

    SSLContext ctx("ALL", true);
    EXPECT_THROW(ctx.loadKeys(keys), Exception);
}

} // namespace sptk_test
