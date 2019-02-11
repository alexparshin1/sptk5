/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpAuthentication.cpp - description                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday April 8 2018                                    ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

sptk::HttpAuthentication::HttpAuthentication(const sptk::String& authenticationHeader)
: m_authenticationHeader(authenticationHeader)
{
}

sptk::HttpAuthentication::~HttpAuthentication()
{
    delete m_userData;
    delete m_jwtData;
}

const sptk::json::Element& sptk::HttpAuthentication::getData()
{
    parse();

    switch (m_type) {
        case EMPTY:
        case BASIC:
            return m_userData->root();
        case BEARER:
            return m_jwtData->grants.root();
        default:
            throw Exception("Invalid or unsupported 'Authentication' header format");
    }
}

void HttpAuthentication::parse()
{
    if (m_type == UNDEFINED) {
        if (m_authenticationHeader.empty()) {
            m_userData = new json::Document;
            m_type = EMPTY;
        } else if (m_authenticationHeader.toLowerCase().startsWith("basic ")) {
            Buffer encoded(m_authenticationHeader.substr(6));
            Buffer decoded;
            Base64::decode(decoded, encoded);
            Strings usernameAndPassword(decoded.c_str(), ":");
            if (usernameAndPassword.size() != 2)
                throw Exception("Invalid or unsupported 'Authentication' header format");
            auto* xuserData = new json::Document;
            xuserData->root()["username"] = usernameAndPassword[0];
            xuserData->root()["password"] = usernameAndPassword[1];
            m_userData = xuserData;
            m_type = BASIC;
        } else if (m_authenticationHeader.toLowerCase().startsWith("bearer ")) {
            auto* xjwtData = new JWT;
            try {
                xjwtData->decode(m_authenticationHeader.substr(7).c_str());
            }
            catch (const Exception&) {
                delete xjwtData;
                throw;
            }
            m_jwtData = xjwtData;
            m_type = BEARER;
        }
    }
}

HttpAuthentication::Type HttpAuthentication::type()
{
    parse();
    return m_type;
}
