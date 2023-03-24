/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Buffer.h>
#include <sptk5/net/Socket.h>

namespace sptk {

/**
 * @addtogroup net Networking Classes
 * @{
 */

/**
 * UDP Socket
 *
 * Sends and receives data using UDP protocol.
 * Not buffered. Doesn't use CSocket timeout settings in read and write operations by default.
 */
class SP_EXPORT UDPSocket
    : public Socket
{
public:
    /**
     * Constructor
     * @param domain SOCKET_ADDRESS_FAMILY, socket domain type
     */
    explicit UDPSocket(SOCKET_ADDRESS_FAMILY domain = AF_INET);

    /**
     * Destructor
     */
    ~UDPSocket() override = default;

protected:
    /**
     * Reads data from the socket
     * @param buffer            The memory buffer
     * @param size              The number of bytes to read
     * @param from              An optional structure for source address
     * @returns the number of bytes read from the socket
     */
    [[nodiscard]] size_t readUnlocked(uint8_t* buffer, size_t size, sockaddr_in* from) override;
};

/**
 * @}
 */
} // namespace sptk
