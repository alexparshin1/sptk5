/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpAuthentication.h - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday April 8 2018                                    ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __HTTP_AUTHENTICATION_H__
#define __HTTP_AUTHENTICATION_H__

#include <sptk5/Strings.h>
#include <sptk5/JWT.h>

namespace sptk {

/**
 * HTTP authentication data, that is passed through
 * Authentication header of HTTP request.
 * Only Basic and Bearer authentication types are currently supported.
 * The data is parsed upon first getData() call.
 */
class HttpAuthentication
{
public:
    /**
     * HTTP authentication type
     */
    enum Type {
        UNDEFINED,
        EMPTY,
        BASIC,
        BEARER,
        DIGEST,
        HOBA,
        MUTUAL,
        AWS4_HMAC_SHA256
    };

private:
    Type                    m_type { UNDEFINED };       ///< Authentication data type
    const String            m_authenticationHeader;     ///< Authentication data
    const JWT*              m_jwtData = { nullptr };    ///< JWT token, if type is BEARER
    const json::Document*   m_userData = { nullptr };   ///< Decoded user data

public:
    /**
     * Constructor
     * @param authenticationHeader  Authentication HTTP header content
     */
    explicit HttpAuthentication(const String& authenticationHeader);

    /**
     * Destructor
     */
    ~HttpAuthentication();

    /**
     * Get decoded authentication data (username and password, or JWT)
     * @return authentication data
     */
    const json::Element& getData();

    /**
     * Get authentication data type
     */
    Type type() const { return m_type; }
};

} // namespace sptk

#endif
