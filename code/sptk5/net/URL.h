/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
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
    explicit URL(const std::string& url = "");

    /**
     * @brief Constructor.
     * @param protocol          Protocol, such as "http" or "ssh", etc.
     * @param host              Host.
     * @param port              Port.
     * @param username          Optional username.
     * @param password          Optional password.
     * @param path              Optional path.
     */
    explicit URL(const std::string& protocol, const std::string& host, uint16_t port = 0,
                 const std::string& username = "", const std::string& password = "",
                 const std::string& path = "");

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
    [[nodiscard]] std::string protocol() const
    {
        return m_protocol;
    }

    /**
     * @return URL username.
     */
    [[nodiscard]] std::string username() const
    {
        return m_username;
    }

    /**
     * @return URL password.
     */
    [[nodiscard]] std::string password() const
    {
        return m_password;
    }

    /**
     * @return URL host and port.
     */
    [[nodiscard]] std::tuple<std::string, uint16_t> hostAndPort() const;

    /**
     * @return URL path.
     */
    [[nodiscard]] std::string path() const
    {
        return m_path;
    }

    /**
     * @return URL resource location.
     */
    [[nodiscard]] std::string location() const;

    /**
     * @return URL as a string.
     */
    [[nodiscard]] std::string toString() const;

    /**
     * @brief Set URL path.
     * @param path URL path.
     */
    void path(const std::string& path)
    {
        m_path = path;
    }

private:
    std::string m_protocol;    ///< URL protocol.
    std::string m_username;    ///< URL username.
    std::string m_password;    ///< URL password.
    std::string m_hostAndPort; ///< URL host and port.
    std::string m_path;        ///< URL path.
    HttpParams  m_params;      ///< URL params.
};

/**
 * @}
 */

} // namespace sptk
