/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSSLServerConnection.h  -  description
                             -------------------
    begin                : Feb 24 2014
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
**************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
***************************************************************************/

#ifndef __CSSLSERVERCONNECTION_H__
#define __CSSLSERVERCONNECTION_H__

#include <sptk5/net/CServerConnection.h>

namespace sptk
{
/// @addtogroup net Networking Classes
/// @{

/// @brief Abstract TCP server connection thread
///
/// Application derives concrete TCP server connections based on this class,
/// to use with CTCPServer as connection template
class CSSLServerConnection: public CServerConnection
{
public:
    /// @brief Constructor
    /// @param connectionSocket SOCKET, Already accepted by accept() function incoming connection socket
    /// @param sslContext CSSLContext&, Server SSL context (shared between connections)
    CSSLServerConnection(SOCKET connectionSocket, CSSLContext& sslContext)
    : CServerConnection(connectionSocket, "SSLServerConnection")
    {
        m_socket = new CSSLSocket(sslContext);
        m_socket->attach(connectionSocket);
    }

    /// @brief Destructor
    virtual ~CSSLServerConnection()
    {
        if (m_socket) {
            delete m_socket;
            m_socket = NULL;
        }
    }
};

/// @}
}
#endif
