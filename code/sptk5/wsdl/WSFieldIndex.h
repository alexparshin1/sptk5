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

#include <sptk5/wsdl/WSType.h>

namespace sptk {

/**
 * Field index contains pointers to WSTypeName objects
 *
 * Field list is defined during construction, and
 * can't be altered later
 */
class WSFieldIndex
{
public:

    /**
     * Initialize field index with field fieldList
     * @param fieldNames        Field names
     * @param fieldList         Field list
     */
    WSFieldIndex() {}

    /**
     * Do not change internal state
     */
    WSFieldIndex(const WSFieldIndex&) {}

    /**
     * Do not change internal state
     */
    WSFieldIndex(WSFieldIndex&&) {}

    /**
     * Do not change internal state
     */
    WSFieldIndex& operator = (const WSFieldIndex&) { return *this; }

    /**
     * Do not change internal state
     */
    WSFieldIndex& operator = (WSFieldIndex&&) { return *this; }

    /**
     * Initialize field index with field fieldList
     * @param fieldNames        Field names
     * @param fieldList         Field list
     */
    void set(const Strings& fieldNames, std::initializer_list<WSType*> fieldList);

    /**
     * Get const fields map
     * @return fields map
     */
    const std::map<String, WSType*>& fields() const { return m_fields; }

    /**
     * Return a field for field name, or return nullptr if not found
     * @param name              Field name
     * @return field pointer, or nullptr if not found
     */
    WSType* find(const String& name) const;

    /**
     * Execute the method for each field until it returns false
     * @param method            Method to execute
     */
    void forEach(const std::function<bool(const String&,WSType*)>& method);

    /**
     * Execute the method for each field until it returns false
     * @param method            Method to execute
     */
    void forEach(const std::function<bool(const String&,WSType*)>& method) const;

private:

    std::map<String, WSType*>   m_fields;
};

typedef std::shared_ptr<WSFieldIndex> SWSFieldIndex;

}
