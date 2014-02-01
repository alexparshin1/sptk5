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

#include <sptk5/net/CUDPSocket.h>

using namespace std;
using namespace sptk;

CUDPSocket::CUDPSocket(SOCKET_ADDRESS_FAMILY domain)
 : CBaseSocket(domain,SOCK_DGRAM)
{
    m_sockfd = socket (m_domain, m_type, m_protocol);
}

size_t CUDPSocket::read(char *buffer,size_t size,sockaddr_in* from) throw(CException)
{
    socklen_t addrLength = sizeof(sockaddr_in);
    int32_t bytes = recvfrom(m_sockfd, (char*) buffer, size, 0, (sockaddr*) from, &addrLength);
    if (bytes < 0)
        THROW_SOCKET_ERROR("Can't read to socket");
    return bytes;
}
