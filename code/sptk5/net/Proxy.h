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

#include <sptk5/net/Host.h>
#include <sptk5/net/Socket.h>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

class TCPSocket;

/**
 * @brief Base proxy class for network connection proxy.
 */
class SP_EXPORT Proxy
{
public:
    /**
     * @brief Constructor.
     * @param host              Proxy host.
     * @param username          Proxy username.
     * @param password          Proxy password.
     */
    explicit Proxy(Host host, String username = "", String password = "");

    /**
     * @brief Destructor.
     */
    virtual ~Proxy() = default;

    /**
     * @brief Connect to the destination host via the proxy.
     * Throws exceptions on connection problems.
     * @param destination       Destination host.
     * @param blockingMode      Blocking mode.
     * @param timeout           Connection timeout.
     * @return                  Socket handle.
     */
    virtual SocketType connect(const Host& destination, bool blockingMode, const std::chrono::milliseconds& timeout) = 0;

    [[nodiscard]] Host getHost() const
    {
        return m_host;
    }

    [[nodiscard]] String getUsername() const
    {
        return m_username;
    }

    [[nodiscard]] String getPassword() const
    {
        return m_password;
    }

protected:
    const Host   m_host;     ///< Proxy host.
    const String m_username; ///< Proxy username.
    const String m_password; ///< Proxy password.
};

/**
 * @}
 */

} // namespace sptk
