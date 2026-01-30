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
#include <sptk5/Base64.h>
#include <sptk5/net/HttpAuthentication.h>

using namespace std;
using namespace sptk;

namespace {
String makeJWT()
{
    const String key256("012345678901234567890123456789XY");

    JWT jwt;
    jwt.set_alg(JWT::Algorithm::HS256, key256);

    constexpr auto testTimestamp = 1594642696;
    jwt.set("iat", testTimestamp);
    jwt.set("iss", "https://test.com");
    jwt.set("exp", testTimestamp + 1);

    const auto& info = jwt.grants.root()->pushNode("info");
    info->set("company", "Linotex");
    info->set("city", "Melbourne");

    stringstream originalToken;
    jwt.encode(originalToken);

    return originalToken.str();
}

String makeBasicHeader(const String& username, const String& password)
{
    const String userPass = username + ":" + password;

    Buffer encoded;
    Base64::encode(encoded, Buffer(userPass));

    return "Basic " + String(encoded.c_str());
}
} // namespace

TEST(SPTK_HttpAuthentication, basic)
{
    HttpAuthentication test("Basic QWxhZGRpbjpPcGVuU2VzYW1l");
    const auto&        auth = test.getData();
    EXPECT_STREQ(auth->getString("username").c_str(), "Aladdin");
    EXPECT_STREQ(auth->getString("password").c_str(), "OpenSesame");
    EXPECT_TRUE(test.type() == HttpAuthentication::Type::BASIC);
}

TEST(SPTK_HttpAuthentication, bearer)
{
    const auto         token = makeJWT();
    HttpAuthentication test("Bearer " + token);
    const auto&        auth = test.getData();

    EXPECT_STREQ(auth->getString("iat").c_str(), "1594642696");
    EXPECT_STREQ(auth->getString("iss").c_str(), "https://test.com");
    EXPECT_STREQ(auth->getString("exp").c_str(), "1594642697");
    EXPECT_TRUE(test.type() == HttpAuthentication::Type::BEARER);
}

TEST(SPTK_HttpAuthentication, emptyHeaderIsTypeEmpty)
{
    HttpAuthentication test("");
    EXPECT_TRUE(test.type() == HttpAuthentication::Type::EMPTY);

    // getData() should be safe for EMPTY and return a valid node/document root
    const auto& auth = test.getData();
    EXPECT_TRUE(auth != nullptr);
}

TEST(SPTK_HttpAuthentication, bearerIsCaseInsensitive)
{
    const auto         token = makeJWT();
    HttpAuthentication test("bEaReR " + token);
    EXPECT_TRUE(test.type() == HttpAuthentication::Type::BEARER);
}

TEST(SPTK_HttpAuthentication, bearerMissingSpaceIsUnsupported)
{
    const auto         token = makeJWT();
    HttpAuthentication test("Bearer" + token); // missing scheme/value separator
    EXPECT_THROW((void) test.getData(), Exception);
}

TEST(SPTK_HttpAuthentication, unsupportedSchemeThrowsOnGetData)
{
    HttpAuthentication test("Digest something");
    EXPECT_THROW((void) test.getData(), Exception);
}

TEST(SPTK_HttpAuthentication, basicWithoutColonInDecodedPayloadThrows)
{
    Buffer encoded;
    Base64::encode(encoded, Buffer(String("usernameOnly")));

    HttpAuthentication test("Basic " + String(encoded.c_str()));
    EXPECT_THROW((void) test.getData(), Exception);
}

TEST(SPTK_HttpAuthentication, basicPasswordWithColonIsSupported)
{
    // NOTE: This test encodes a password containing ':' which is allowed in Basic auth
    // (only the first ':' separates username and password).
    HttpAuthentication test(makeBasicHeader("user", "pa:ss"));
    const auto&        auth = test.getData();

    EXPECT_STREQ(auth->getString("username").c_str(), "user");
    EXPECT_STREQ(auth->getString("password").c_str(), "pa:ss");
    EXPECT_TRUE(test.type() == HttpAuthentication::Type::BASIC);
}

TEST(SPTK_HttpAuthentication, basicInvalidBase64Throws)
{
    HttpAuthentication test("Basic !!!not_base64!!!");
    EXPECT_ANY_THROW((void) test.getData());
}
