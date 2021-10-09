/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Base64.h>
#include <sptk5/net/HttpAuthentication.h>

using namespace std;
using namespace sptk;

HttpAuthentication::HttpAuthentication(String authenticationHeader)
    : m_authenticationHeader(std::move(authenticationHeader))
{
}

const xdoc::SNode& HttpAuthentication::getData()
{
    parse();

    switch (m_type)
    {
        case Type::EMPTY:
        case Type::BASIC:
            return m_userData->root();
        case Type::BEARER:
            return m_jwtData->grants.root();
        default:
            throw Exception("Invalid or unsupported 'Authentication' header format");
    }
}

String HttpAuthentication::getHeader() const
{
    return m_authenticationHeader;
}

void HttpAuthentication::parse()
{
    if (m_type == Type::UNDEFINED)
    {
        if (m_authenticationHeader.empty())
        {
            m_userData = make_shared<xdoc::Document>();
            m_type = Type::EMPTY;
        }
        else if (m_authenticationHeader.toLowerCase().startsWith("basic "))
        {
            Buffer encoded(m_authenticationHeader.substr(6));
            Buffer decoded;
            Base64::decode(decoded, encoded);
            Strings usernameAndPassword(decoded.c_str(), ":");
            if (usernameAndPassword.size() != 2)
            {
                throw Exception("Invalid or unsupported 'Authentication' header format");
            }
            auto xuserData = make_shared<xdoc::Document>();
            xuserData->root()->set("username", usernameAndPassword[0]);
            xuserData->root()->set("password", usernameAndPassword[1]);
            m_userData = xuserData;
            m_type = Type::BASIC;
        }
        else
        {
            String authMethod = m_authenticationHeader.substr(0, 6);
            if (authMethod.toLowerCase() == "bearer")
            {
                auto xjwtData = make_shared<JWT>();
                xjwtData->decode(m_authenticationHeader.substr(7).c_str());
                m_jwtData = xjwtData;
                m_type = Type::BEARER;
            }
        }
    }
}

HttpAuthentication::Type HttpAuthentication::type()
{
    parse();
    return m_type;
}

#ifdef USE_GTEST

static String makeJWT()
{
    String key256("012345678901234567890123456789XY");

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

TEST(SPTK_HttpAuthentication, basic)
{
    HttpAuthentication test("Basic QWxhZGRpbjpPcGVuU2VzYW1l");
    const auto& auth = test.getData();
    EXPECT_STREQ(auth->getString("username").c_str(), "Aladdin");
    EXPECT_STREQ(auth->getString("password").c_str(), "OpenSesame");
    EXPECT_EQ(test.type(), HttpAuthentication::Type::BASIC);
}

TEST(SPTK_HttpAuthentication, bearer)
{
    auto token = makeJWT();
    HttpAuthentication test("Bearer " + token);
    const auto& auth = test.getData();

    EXPECT_STREQ(auth->getString("iat").c_str(), "1594642696");
    EXPECT_STREQ(auth->getString("iss").c_str(), "https://test.com");
    EXPECT_STREQ(auth->getString("exp").c_str(), "1594642697");
    EXPECT_EQ(test.type(), HttpAuthentication::Type::BEARER);
}

#endif
