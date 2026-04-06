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

#pragma once

#include <sptk5/net/HttpParams.h>

namespace sptk {

/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief URL.
 */
class SP_EXPORT URL
{
public:
    /**
     * @brief Constructor.
     * @param url           URL as a string.
     */
    explicit URL(const String& url);

    /**
     * @brief Copy constructor.
     * @param other         Another object.
     */
    URL(const URL& other) = default;

    /**
     * @return URL params.
     */
    HttpParams& params()
    {
        return m_params;
    }

    /**
     * @return URL params.
     */
    [[nodiscard]] const HttpParams& params() const
    {
        return m_params;
    }

    /**
     * @return URL protocol.
     */
    [[nodiscard]] String protocol() const
    {
        return m_protocol;
    }

    /**
     * @return URL username.
     */
    [[nodiscard]] String username() const
    {
        return m_username;
    }

    /**
     * @return URL password.
     */
    [[nodiscard]] String password() const
    {
        return m_password;
    }

    /**
     * @return URL host and port.
     */
    [[nodiscard]] std::tuple<String, uint16_t> hostAndPort() const;

    /**
     * @return URL path.
     */
    [[nodiscard]] String path() const
    {
        return m_path;
    }

    /**
     * @return URL resource location.
     */
    [[nodiscard]] String location() const;

    /**
     * @return URL as a string.
     */
    [[nodiscard]] String toString() const;

    /**
     * @brief Set URL path.
     * @param path URL path.
     */
    void path(const String& path)
    {
        m_path = path;
    }

private:
    String     m_protocol;    ///< URL protocol.
    String     m_username;    ///< URL username.
    String     m_password;    ///< URL password.
    String     m_hostAndPort; ///< URL host and port.
    String     m_path;        ///< URL path.
    HttpParams m_params;      ///< URL params.
};

/**
 * @}
 */

} // namespace sptk
