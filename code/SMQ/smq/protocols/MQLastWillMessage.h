/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MQLastWill.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Monday March 11 2019                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __MQ_LAST_WILL_H__
#define __MQ_LAST_WILL_H__

#include <sptk5/String.h>
#include <sptk5/Buffer.h>

namespace smq {

/**
 * MQTT Last Will message definition.
 *
 * Last will message is published to the defined topic,
 * when a client that defined it drops the connection to MQTT server.
 */
class MQLastWillMessage
{
    sptk::String      m_destination;  ///< Topic to publish the message
    sptk::String      m_message;      ///< Message to publish
public:
    /**
     * Constructor
     * @param destination       Topic to publish the message
     * @param message           Message to publish
     */
    MQLastWillMessage(const sptk::String& destination, const sptk::String& message)
    : m_destination(destination), m_message(message)
    {}

    /**
     * Get message destination
     * @return message destination
     */
    sptk::String destination() const { return m_destination; }

    /**
     * Get message content
     * @return message content
     */
    sptk::String message() const { return m_message; }
};

} // namespace sptk

#endif
