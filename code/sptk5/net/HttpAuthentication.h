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

#pragma once

#include <sptk5/Strings.h>
#include <sptk5/JWT.h>

namespace sptk {

/**
 * HTTP authentication data, that is passed through
 * Authentication header of HTTP request.
 * Only Basic and Bearer authentication types are currently supported.
 * The data is parsed upon first getData() call.
 */
class SP_EXPORT HttpAuthentication
{
public:
    /**
     * HTTP authentication type
     */
    enum class Type
        : uint8_t
    {
        UNDEFINED,
        EMPTY,
        BASIC,
        BEARER,
        DIGEST,
        HOBA,
        MUTUAL,
        AWS4_HMAC_SHA256
    };

    /**
     * Constructor
     * @param authenticationHeader  Authentication HTTP header content
     */
    explicit HttpAuthentication(String authenticationHeader);

    /**
     * Copy constructor
     * @param authenticationHeader  Authentication HTTP header content
     */
    explicit HttpAuthentication(const HttpAuthentication& other) = default;

    /**
     * Get decoded authentication data (username and password, or JWT)
     * @return authentication data
     */
    String getHeader() const;

    /**
     * Get decoded authentication data (username and password, or JWT)
     * @return authentication data
     */
    const xdoc::SNode& getData();

    /**
     * Get authentication data type
     */
    Type type();

private:

    Type m_type {Type::UNDEFINED};         ///< Authentication data type
    const String m_authenticationHeader;   ///< Authentication data
    std::shared_ptr<JWT> m_jwtData;        ///< JWT token, if type is BEARER
    xdoc::SDocument m_userData;            ///< Decoded user data

    /**
     * Decode authentication data (username and password, or JWT)
     */
    void parse();
};

using SHttpAuthentication = std::shared_ptr<HttpAuthentication>;

}
