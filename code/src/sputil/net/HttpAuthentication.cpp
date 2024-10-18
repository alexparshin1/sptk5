/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
            constexpr int basicLength = 6;
            const Buffer  encoded(m_authenticationHeader.substr(basicLength));
            Buffer        decoded;
            Base64::decode(decoded, encoded);
            Strings usernameAndPassword(decoded.c_str(), ":");
            if (usernameAndPassword.size() != 2)
            {
                throw Exception("Invalid or unsupported 'Authentication' header format");
            }
            const auto aUserData = make_shared<xdoc::Document>();
            aUserData->root()->set("username", usernameAndPassword[0]);
            aUserData->root()->set("password", usernameAndPassword[1]);
            m_userData = aUserData;
            m_type = Type::BASIC;
        }
        else
        {
            constexpr int bearerLength = 6;
            if (m_authenticationHeader.length() > 7)
            {
                if (const String authMethod = m_authenticationHeader.substr(0, bearerLength);
                    authMethod.toLowerCase() == "bearer")
                {
                    const auto aJwtData = make_shared<JWT>();
                    aJwtData->decode(m_authenticationHeader.substr(bearerLength + 1).c_str());
                    m_jwtData = aJwtData;
                    m_type = Type::BEARER;
                }
            }
        }
    }
}

HttpAuthentication::Type HttpAuthentication::type()
{
    parse();
    return m_type;
}
