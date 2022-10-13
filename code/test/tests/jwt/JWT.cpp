/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            (C) 1999-2022 Alexey Parshin. All rights reserved.     ║
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

TEST(SPTK_JWT, dup)
{
    time_t now = 0;
    int valint = 0;

    JWT jwt;

    jwt.set("iss", "test");
    auto val = (String) jwt.get("iss");
    EXPECT_FALSE(val.empty()) << "Can't get grant for first JWT";

    JWT newJWT(jwt);
    val = (String) newJWT.get("iss");
    EXPECT_FALSE(val.empty()) << "Can't get grant for second JWT";

    EXPECT_STREQ("test", val.c_str()) << "Got incorrect grant";
    EXPECT_EQ(JWT::Algorithm::NONE, jwt.get_alg()) << "Got incorrect alogorithm";

    now = time(nullptr);
    jwt.set("iat", (int) now);

    valint = (int) jwt.get("iat");
    EXPECT_EQ((long) now, valint) << "Failed jwt_get_grant_int()";
}

TEST(SPTK_JWT, dup_signed)
{
    String key256("012345678901234567890123456789XY");

    JWT jwt;
    jwt.set("iss", "test");
    jwt.set_alg(JWT::Algorithm::HS256, key256);

    JWT newJWT(jwt);
    auto val = (String) newJWT.get("iss");
    EXPECT_STREQ("test", val.c_str()) << "Failed jwt_get_grant_int()";
    EXPECT_EQ(JWT::Algorithm::HS256, jwt.get_alg()) << "Failed jwt_get_alg()";
}


TEST(SPTK_JWT, decode)
{
    const char token[] =
        "eyJhbGciOiJub25lIn0.eyJpc3MiOiJmaWxlcy5jeXBo"
        "cmUuY29tIiwic3ViIjoidXNlcjAifQ.";
    JWT::Algorithm alg = JWT::Algorithm::NONE;

    auto jwt = make_shared<JWT>();

    EXPECT_NO_THROW(jwt->decode(token)) << "Failed jwt_decode()";
    alg = jwt->get_alg();
    EXPECT_EQ(JWT::Algorithm::NONE, alg) << "Failed jwt_get_alg()";
}


TEST(SPTK_JWT, decode_invalid_final_dot)
{
    const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
                         "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                         "3ViIjoidXNlcjAifQ";

    auto jwt = make_shared<JWT>();
    EXPECT_NO_THROW(jwt->decode(token)) << "Not failed jwt_decode()";
}


TEST(SPTK_JWT, decode_invalid_alg)
{
    const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIQUhBSCJ9."
                         "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                         "3ViIjoidXNlcjAifQ.";

    auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";
}


TEST(SPTK_JWT, decode_invalid_typ)
{
    const char token[] = "eyJ0eXAiOiJBTEwiLCJhbGciOiJIUzI1NiJ9."
                         "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                         "3ViIjoidXNlcjAifQ.";

    auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";
}


TEST(SPTK_JWT, decode_invalid_head)
{
    const char token[] =
        "yJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
        "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
        "3ViIjoidXNlcjAifQ.";

    auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";
}


TEST(SPTK_JWT, decode_alg_none_with_key)
{
    const char token[] =
        "eyJhbGciOiJub25lIn0."
        "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
        "3ViIjoidXNlcjAifQ.";

    auto jwt = make_shared<JWT>();
    EXPECT_NO_THROW(jwt->decode(token)) << "Not failed jwt_decode()";
}


TEST(SPTK_JWT, decode_invalid_body)
{
    const char token[] =
        "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
        "eyJpc3MiOiJmaWxlcy5jeBocmUuY29tIiwic"
        "3ViIjoidXNlcjAifQ.";

    auto jwt = make_shared<JWT>();
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";
}


TEST(SPTK_JWT, decode_hs256)
{
    const char token[] =
        "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3Mi"
        "OiJmaWxlcy5jeXBocmUuY29tIiwic3ViIjoidXNlcjAif"
        "Q.dLFbrHVViu1e3VD1yeCd9aaLNed-bfXhSsF0Gh56fBg";
    String key256("012345678901234567890123456789XY");

    auto jwt = make_shared<JWT>();
    EXPECT_NO_THROW(jwt->decode(token, key256)) << "Failed jwt_decode()";
}


TEST(SPTK_JWT, decode_hs384)
{
    const char token[] =
        "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
        "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
        "3ViIjoidXNlcjAifQ.xqea3OVgPEMxsCgyikr"
        "R3gGv4H2yqMyXMm7xhOlQWpA-NpT6n2a1d7TD"
        "GgU6LOe4";
    String key384(
        "aaaabbbbccccddddeeeeffffg"
        "ggghhhhiiiijjjjkkkkllll");

    auto jwt = make_shared<JWT>();
    EXPECT_NO_THROW(jwt->decode(token, key384)) << "Failed jwt_decode()";
}


TEST(SPTK_JWT, decode_hs512)
{
    const char token[] =
        "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzUxMiJ9.eyJpc3Mi"
        "OiJmaWxlcy5jeXBocmUuY29tIiwic3ViIjoidXNlcjAif"
        "Q.u-4XQB1xlYV8SgAnKBof8fOWOtfyNtc1ytTlc_vHo0U"
        "lh5uGT238te6kSacnVzBbC6qwzVMT1806oa1Y8_8EOg";
    String key512(
        "012345678901234567890123456789XY"
        "012345678901234567890123456789XY");

    auto jwt = make_shared<JWT>();

    try
    {
        jwt->decode(token, key512);
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_JWT, encode_hs256_decode)
{
    String key256("012345678901234567890123456789XY");

    JWT jwt;
    jwt.set_alg(JWT::Algorithm::HS256, key256);

    constexpr int secondsInDay = 86400;
    jwt.set("iat", (int) time(nullptr));
    jwt.set("iss", "https://test.com");
    jwt.set("exp", (int) time(nullptr) + secondsInDay);

    const auto& info = jwt.grants.root()->pushNode("info");
    info->set("company", "Linotex");
    info->set("city", "Melbourne");

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
