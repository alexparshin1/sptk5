/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            (C) 1999-2026 Alexey Parshin. All rights reserved.     ║
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
┌──────────────────────────────────────────────────────────────────────────────┐
│   This code is based on JWT C Library, developed by Ben Collins.             │
│   Please see http://github.com/benmcollins/libjwt for more information.      │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <gtest/gtest.h>
#include <sptk5/Base64.h>
#include <sptk5/JWT.h>

using namespace std;
using namespace sptk;

namespace {

JWT makeTestJwt(JWT::Algorithm alg, const String& key)
{
    JWT jwt;
    jwt.set_alg(alg, key);

    constexpr auto secondsInDay = 86400;
    jwt.set("iat", static_cast<int>(time(nullptr)));
    jwt.set("iss", "https://test.com");
    jwt.set("exp", static_cast<int>(time(nullptr)) + secondsInDay);

    const auto& info = jwt.grants.root()->pushNode("info");
    info->set("company", "Linotex");
    info->set("city", "Melbourne");

    return jwt;
}

} // namespace
namespace sptk {

TEST(JWTTests,dup)
{
    time_t now = 0;
    int    valint = 0;

    const JWT jwt;

    jwt.set("iss", "test");
    auto val = static_cast<String>(jwt.get("iss"));
    EXPECT_FALSE(val.empty()) << "Can't get grant for first JWT";

    const JWT newJWT(jwt);
    val = static_cast<String>(newJWT.get("iss"));
    EXPECT_FALSE(val.empty()) << "Can't get grant for second JWT";

    EXPECT_STREQ("test", val.c_str()) << "Got incorrect grant";
    EXPECT_TRUE(JWT::Algorithm::NONE == jwt.get_alg()) << "Got incorrect alogorithm";

    now = time(nullptr);
    jwt.set("iat", static_cast<int>(now));

    valint = static_cast<int>(jwt.get("iat"));
    EXPECT_EQ(static_cast<long>(now), valint) << "Failed jwt_get_grant_int()";
}

TEST(JWTTests,dupSigned)
{
    const String key256("012345678901234567890123456789XY");

    JWT jwt;
    jwt.set("iss", "test");
    jwt.set_alg(JWT::Algorithm::HS256, key256);

    const JWT  newJWT(jwt);
    const auto val = static_cast<String>(newJWT.get("iss"));
    EXPECT_STREQ("test", val.c_str()) << "Failed jwt_get_grant_int()";
    EXPECT_TRUE(JWT::Algorithm::HS256 == jwt.get_alg()) << "Failed jwt_get_alg()";
}


TEST(JWTTests,decode)
{
    const char* token =
        "eyJhbGciOiJub25lIn0.eyJpc3MiOiJmaWxlcy5jeXBo"
        "cmUuY29tIiwic3ViIjoidXNlcjAifQ.";
    JWT::Algorithm alg = JWT::Algorithm::NONE;

    const auto jwt = make_shared<JWT>();

    EXPECT_NO_THROW(jwt->decode(token)) << "Failed jwt_decode()";
    alg = jwt->get_alg();
    EXPECT_TRUE(JWT::Algorithm::NONE == alg) << "Failed jwt_get_alg()";
}


TEST(JWTTests,decodeInvalidFinalDot)
{
    // Incomplete JWT token:
    const char* token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
                        "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                        "3ViIjoidXNlcjAifQ";

    const auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception);
}


TEST(JWTTests,decodeInvalidAlg)
{
    const char* token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIQUhBSCJ9."
                        "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                        "3ViIjoidXNlcjAifQ.";

    const auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";
}


TEST(JWTTests,decodeInvalidTyp)
{
    const char* token = "eyJ0eXAiOiJBTEwiLCJhbGciOiJIUzI1NiJ9."
                        "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                        "3ViIjoidXNlcjAifQ.";

    const auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";
}


TEST(JWTTests,decodeInvalidHead)
{
    const char* token =
        "yJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
        "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
        "3ViIjoidXNlcjAifQ.";

    const auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";
}


TEST(JWTTests,decodeAlgNoneWithKey)
{
    const char* token = "eyJhbGciOiJub25lIn0."
                        "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                        "3ViIjoidXNlcjAifQ.";

    const auto jwt = make_shared<JWT>();
    EXPECT_NO_THROW(jwt->decode(token)) << "Not failed jwt_decode()";
}


TEST(JWTTests,decodeInvalidBody)
{
    const char* token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
                        "eyJpc3MiOiJmaWxlcy5jeBocmUuY29tIiwic"
                        "3ViIjoidXNlcjAifQ.";

    const auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";
}

TEST(JWTTests,decodeHs256)
{
    const String key256("012345678901234567890123456789XY");

    JWT jwt = makeTestJwt(JWT::Algorithm::HS256, key256);

    stringstream tokenStream;
    jwt.encode(tokenStream);

    EXPECT_NO_THROW(jwt.decode(tokenStream.str().c_str(), key256)) << "Failed jwt_decode()";
}


TEST(JWTTests,decodeHs384)
{
    const String key384("aaaabbbbccccddddeeeeffffg"
                        "ggghhhhiiiijjjjkkkkllll");

    JWT jwt = makeTestJwt(JWT::Algorithm::HS384, key384);

    stringstream tokenStream;
    jwt.encode(tokenStream);

    EXPECT_NO_THROW(jwt.decode(tokenStream.str().c_str(), key384)) << "Failed jwt_decode()";
}


TEST(JWTTests,decodeHs512)
{
    const String key512("012345678901234567890123456789XY"
                        "012345678901234567890123456789XY");

    JWT jwt = makeTestJwt(JWT::Algorithm::HS512, key512);

    stringstream tokenStream;
    jwt.encode(tokenStream);

    try
    {
        jwt.decode(tokenStream.str().c_str(), key512);
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(JWTTests,encodeHs256Decode)
{
    const String key256("012345678901234567890123456789XY");

    JWT jwt = makeTestJwt(JWT::Algorithm::HS256, key256);

    stringstream originalToken;
    jwt.encode(originalToken);

    stringstream originalJSON;
    jwt.exportTo(originalJSON, false);

    JWT jwt2;
    jwt2.decode(originalToken.str().c_str(), key256);

    stringstream copiedJSON;
    jwt2.exportTo(copiedJSON, false);

    EXPECT_STREQ(originalJSON.str().c_str(), copiedJSON.str().c_str())
        << "Decoded JSON payload doesn't match the original";
}

} // namespace sptk_test
