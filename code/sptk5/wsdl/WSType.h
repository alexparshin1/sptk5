/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/xdoc/Node.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * Class name support for WS-classes
 */
class SP_EXPORT WSType
{
public:
    /**
     * Get WS type name
     * @return WS type name
     */
    [[nodiscard]] virtual String className() const
    {
        return "";
    }

    [[nodiscard]] virtual String name() const
    {
        return "";
    }

    virtual void owaspCheck(const String& value);

    /**
     * Clears content (sets to NULL)
     */
    virtual void clear() = 0;

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    virtual void load(const xdoc::SNode& attr, bool nullLargeData = false) = 0;

    /**
     * Conversion to string
     */
    virtual String asString() const
    {
        return "";
    }

    virtual bool isNull() const = 0;

    /**
     * Unload data to new XML node
     * @param parent            Parent XML node where new node is created
     * @param name              Optional name for the child element
     */
    virtual void exportTo(const xdoc::SNode& parent, const char* name = nullptr) const = 0;
};

}
