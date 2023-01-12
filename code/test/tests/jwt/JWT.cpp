/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            (C) 1999-2023 Alexey Parshin. All rights reserved.     ║
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
/*
TEST(SPTK_JWT, encode_es256_decode)
{
    String ecdsa256 = R"(MIIEpQIBAAKCAQEAykLgj7EOD2XVQf9TRJd3C1IKR01SbfD0ngXAAnOyShFZzYZv
pjw8mBfvongxQYgy9pW4TvXwf3GYFALWFD+GaR8ZhZZVds74/59yozA6xqLMOcZT
uwpRGRc/GQ0UvBMnHm/auxC3z0TJ1ZoTuL0gvd1jnuI195pAiL9pzQ167QZbBZM8
cpLAec44mOykTVhaq7XYJJu6aNZnjb8sUTHDDpvYsv7XEexFAtvI0AckWVHzhpgW
XNiodJsB7fTKMHMimlGHaEST2Toe+QCe2/MWvqdRVpbEmbqiE/Ffpjen9me3Bzmp
RJoecQVcd2AsARbG+GylhZplBr0Yw3japy+6AwIDAQABAoIBAQCy8q55kx2sc91V
Mc/7DMMvSdt8MCXwzVGvicjSoogoTeeVFg8sFXlK20qSBSMXJqhoUBisC55HMzpo
4gWEDpwd330WGubsYbwddXXYebBW0+w6ZYTpI/ySZWXXBSLGL2/Z1v6/qM/nWqX0
DQjL1tDkCr678MnskhXspuW1nOqYTwajnNGWBSqm5zKWqUtf2LpbBtQdVR+40EAu
nwtIz5+IHhofcOpUH+E9ZA2nVd15CBnRcvsXs9x7xfhEUfqM25X74KmmoTNOdJtT
UkwQw+vTrV5w48iEQlyIZKwq+KFt6tjbiFjXZEV44d46M2DYxRw/kGw0/Am49h+j
3HEJpdyZAoGBAPNhFU1JplzOmwSeh4qG1HYrrU+ploGFJlyKj4R4pUt1UxBgEdfi
/DdoUnFvs3yHvL8Lx57QmPGxe2CQmkRlYctqzaHyNxp+XZzhcs2hxjBzifloEKYk
xU7z+VHzw5vouoCuByhE+97cncZYQrz4bTs6VH3CVGh98UHjLHYc0T5tAoGBANS/
8WNLhojjO4g6OE4X9tssEP8IpGCGH44NhDa8haqbf3U4g7l4DPYmRvFPnQa8FC3d
D0qRfoSIiswtLfPxCXFTRkpQTeY2/3xHFGMvcmCP7EKsBLlmVgigMykDyfeiuWzo
jMKHREzmO3vOrIAAF8FUdL2fTpe/KzGaE3pxf9QvAoGBAKl7Y+Asd6ONZLo0w2Ke
PfoEtG4TRPHxDSPIgeTYNxNzImL247YZJVZYWYERLkZ8J95Kj7pyvO8ijy5RxHv4
tb94Irax+9mBQiNrhAzaqS84Zk6+P0nTtWsjzu1Y+VDrImVVyzopv9QUgfKLp/38
aeSi3A+vciRJ/+XIE0A1FSmJAoGAd0ibRgoNh2ioc0v5T8fd76r4aJXm2/u3a4Um
kS4IX8zJnOav7GhkFAsIEbqKl0ESq1hbf3quDg8kiy/1qOWHXtPLAFWgJ6jEfGC6
DJaIsZ1gYU1jZLP9Ht77cE6gicjh4C9O5K7E27zmsxcA3s+uggYhYkQU474asLfr
neZPPp8CgYEAn2Mh3Ai4+cip/D/zUZ9mLv3cvbJeid5LwQx4XwnY0gzs91dAupAB
hiT4xmcA7Uk77imAclhErGeHrMgecC8T7H/7lhYLLTm/SpllssFCBCZjyP1MSITl
GgVdND75fjS0xr6YLOBi6nSgNHI5/hP4HA7GBX2kUqONxb6gKaiQx4M=)";

    JWT jwt;
    jwt.set_alg(JWT::Algorithm::ES256, ecdsa256);

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
    jwt2.decode(originalToken.str().c_str(), ecdsa256);

    stringstream copiedJSON;
    jwt2.exportTo(copiedJSON, false);

    EXPECT_STREQ(originalJSON.str().c_str(), copiedJSON.str().c_str())
        << "Decoded JSON payload doesn't match the original";
}
*/
