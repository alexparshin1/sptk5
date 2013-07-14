/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CUDPSocket.h  -  description
                             -------------------
    begin                : Jul 10 2013
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#ifndef __CUDPSOCKET_H__
#define __CUDPSOCKET_H__

#include <sptk5/net/CBaseSocket.h>
#include <sptk5/CBuffer.h>

namespace sptk {

/// @addtogroup net Networking Classes
/// @{

/// @brief UDP Socket
///
/// Sends and receives data using UDP protocol.
/// Not buffered. Doesn't use CSocket timeout settings in read and write operations by default.
class CUDPSocket : public CBaseSocket
{
public:
    /// @brief Constructor
    /// @param domain SOCKET_ADDRESS_FAMILY, socket domain type
    CUDPSocket(SOCKET_ADDRESS_FAMILY domain=AF_INET);

    /// @brief Destructor
    virtual ~CUDPSocket() {}

    /// @brief Reads data from the socket
    /// @param buffer char *, the memory buffer
    /// @param size uint32_t, the number of bytes to read
    /// @param from sockaddr_in*, an optional structure for source address
    /// @returns the number of bytes read from the socket
    virtual uint32_t read(char *buffer,uint32_t size,sockaddr_in* from=NULL);
};

/// @}
}
#endif
