/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSProtocol.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
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

#ifndef __WSPROTOCOL_H__
#define __WSPROTOCOL_H__

#include <sptk5/cnet>
#include <sptk5/wsdl/WSRequest.h>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Abstract base class for different protocols used in Web Service servers
class WSProtocol
{
protected:

    TCPSocket&      m_socket;   ///< Connection socket
    HttpHeaders     m_headers;  ///< Connection HTTP headers

public:

    /// @brief Constructor
    /// @param socket           Connection socket
    /// @param headers          Connection HTTP headers
    WSProtocol(TCPSocket* socket, const HttpHeaders& headers)
    : m_socket(*socket), m_headers(headers)
    {
    }

    /// @brief Destructor
    ///
    /// Closes connection
    virtual ~WSProtocol()
    {
        m_socket.close();
    }

    /// @brief Process virtual method - to be implemented in derived classes
    virtual void process() = 0;
};

/// @}

}

#endif
