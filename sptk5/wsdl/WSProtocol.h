/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSProtocol.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

/// @brief Abstract base class for different protocols used in Web Service servers
class WSProtocol
{
protected:

    TCPSocket&                      m_socket;   ///< Connection socket
    const std::map<String,String>&  m_headers;  ///< Connection HTTP headers

public:

    /// @brief Constructor
    /// @param socket TCPSocket*, Connection socket
    /// @param headers const std::map<String,String>&, Connection HTTP headers
    WSProtocol(TCPSocket* socket, const std::map<String,String>& headers)
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

/// @brief Handler for static files (.html, .js, .png, etc)
///
/// Session disconnects as soon as file is served.
class WSStaticHttpProtocol : public WSProtocol
{
    String  m_url;                      ///< File URL
    String  m_staticFilesDirectory;     ///< Directory where static files reside on the server
public:

    /// @brief Constructor
    /// @param socket TCPSocket*, Connection socket
    /// @param url String, File URL
    /// @param headers const std::map<String,String>&, Connection HTTP headers
    /// @param staticFilesDirectory String, Directory where static files reside on the server
    WSStaticHttpProtocol(TCPSocket *socket, String url, const std::map<String,String>& headers, String staticFilesDirectory);

    /// @brief Process method
    ///
    /// Writes HTTP response and file content to the connection
    virtual void process();
};

/// @brief WebSockets connection handler
///
/// Treats connection as WebSockets, implementing WebSockets
/// handshake and client session. Session stays connected until
/// client disconnects.
class WSWebSocketsProtocol : public WSProtocol
{
public:
    /// @brief Constructor
    /// @param socket TCPSocket*, Connection socket
    /// @param headers const std::map<String,String>&, Connection HTTP headers
    WSWebSocketsProtocol(TCPSocket *socket, const std::map<String,String>& headers);

    /// @brief Process method
    ///
    /// Implements WebSockets session
    virtual void process();
};

/// @brief WebService connection handler
///
/// Uses WSRequest service object to parse WS request and
/// reply, then closes connection.
class WSWebServiceProtocol : public WSProtocol
{
    WSRequest&     m_service;
public:

    /// @brief Constructor
    /// @param socket TCPSocket*, Connection socket
    /// @param headers const std::map<String,String>&, Connection HTTP headers
    WSWebServiceProtocol(TCPSocket *socket, const std::map<String,String>& headers, WSRequest& service);

    /// @brief Process method
    ///
    /// Calls WebService request through service object
    virtual void process();
};

}

#endif
