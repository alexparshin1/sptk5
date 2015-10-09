/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CTCPServerConnection.h  -  description
                             -------------------
    begin                : Jul 13 2013
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/


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

#ifndef __CTCPSERVERCONNECTION_H__
#define __CTCPSERVERCONNECTION_H__

#include <sptk5/net/CServerConnection.h>

namespace sptk
{

/// @addtogroup net Networking Classes
/// @{

/// @brief Abstract TCP server connection thread
///
/// Application derives concrete TCP server connections based on this class,
/// to use with CTCPServer as connection template
class CTCPServerConnection: public CServerConnection
{
public:
    /// @brief Constructor
    /// @param connectionSocket SOCKET, Already accepted by accept() function incoming connection socket
    CTCPServerConnection(SOCKET connectionSocket)
    : CServerConnection(connectionSocket, "TCPServerConnection")
    {
        m_socket = new CTCPSocket;
        m_socket->attach(connectionSocket);
    }
};

/// @}
}
#endif
