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

#include <sptk5/cnet>
#include <sptk5/net/RequestInfo.h>

namespace sptk {
/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Abstract base class for different protocols used in Web Service servers.
class WSProtocol
{
public:
    /// @brief Constructor.
    /// Note: the socket is not owned by this class and not discarded by destructor.
    /// @param socket           Connection socket.
    /// @param headers          Connection HTTP headers.
    WSProtocol(std::shared_ptr<TCPSocket> socket, const HttpHeaders& headers)
        : m_socket(std::move(socket))
        , m_headers(headers)
    {
    }

    WSProtocol(const WSProtocol&) = delete;

    /// @brief Destructor.
    ///
    /// Closes connection.
    virtual ~WSProtocol() = default;

    WSProtocol& operator=(const WSProtocol&) = delete;

    /// Process virtual method - to be implemented in derived classes.
    virtual RequestInfo process() = 0;

protected:
    /**
     * @brief Connection socket.
     * @return Connection socket.
     */
    TCPSocket& socket()
    {
        return *m_socket;
    }

    /**
     * @brief Connection HTTP headers.
     * @return Connection HTTP headers
     */
    HttpHeaders& headers()
    {
        return m_headers;
    }

    /**
     * @brief Connection HTTP headers.
     * @return Connection HTTP headers.
     */
    String header(const String& name);

private:
    std::shared_ptr<TCPSocket> m_socket;  ///< Connection socket.
    HttpHeaders                m_headers; ///< Connection HTTP headers.
};

/// @}

} // namespace sptk
