/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/xml/Node.h>
#include <sptk5/json/JsonDocument.h>

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
    [[nodiscard]] virtual String className() const { return ""; }

    [[nodiscard]] virtual String name() const { return ""; }

    virtual void owaspCheck(const String& value);

    /**
     * Clears content (sets to NULL)
     */
    virtual void clear() = 0;

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    virtual void load(const xml::Node* attr) = 0;

    /**
     * Loads type data from request JSON element
     * @param attr              JSON element
     */
    virtual void load(const json::Element* attr) = 0;

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
    virtual void addElement(xml::Node* parent, const char* name=nullptr) const = 0;

    /**
     * Unload data to new JSON node
     * @param parent            Parent JSON node where new node is created
     */
    virtual void addElement(json::Element* parent) const = 0;
};

}