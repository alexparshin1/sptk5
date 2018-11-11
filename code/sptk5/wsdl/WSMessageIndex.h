/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSMessageIndex.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Monday September 17 2018                               ║
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

#ifndef __WS_MESSAGE_INDEX_H__
#define __WS_MESSAGE_INDEX_H__

#include <sptk5/Strings.h>
#include <mutex>

namespace sptk {

/**
 * WSDL message index
 */
class WSMessageIndex
{
    /**
     * Mutex that protects internal data
     */
    mutable std::mutex      m_mutex;

    /**
     * Map of message name to index
     */
    std::map<String,int>    m_messageIndex;
public:
    /**
     * Constructor
     * @param messages          Message names
     */
    explicit WSMessageIndex(const Strings& messages);

    /**
     * Get message index by name
     * @param message           Message name
     * @return message index
     */
    int indexOf(const String& message) const;
};

} // namespace sptk

#endif
