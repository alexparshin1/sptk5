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
 * Field index contains pointers to WSType objects
 */
class SP_EXPORT WSFieldIndex
{
public:
    /**
     * Type(s) of field that are processed with forEach
     */
    enum FieldGroup
    {
        ELEMENTS = 1,
        ATTRIBUTES = 2,
        ELEMENTS_AND_ATTRIBUTES = 3
    };

    /**
     * Initialize field index with field fieldList
     * @param fieldNames        Field names
     * @param fieldList         Field list
     */
    WSFieldIndex() {}

    /**
     * Destructor
     */
    ~WSFieldIndex() = default;

    /**
     * Do not change internal state    for (auto& field: m_attributes) {
        if (!method(field.first, field.second))
            return;
    }

     */
    WSFieldIndex(const WSFieldIndex&) {}

    /**
     * Do not change internal state
     */
    WSFieldIndex(WSFieldIndex&&) noexcept {}

    /**
     * Do not change internal state
     */
    WSFieldIndex& operator = (const WSFieldIndex&) { return *this; }

    /**
     * Do not change internal state
     */
    WSFieldIndex& operator = (WSFieldIndex&&) noexcept { return *this; }

    /**
     * Initialize field index with elements
     * @param fieldNames        Field names
     * @param fieldList         Field list
     */
    void setElements(const Strings& fieldNames, std::initializer_list<WSType*> fieldList);

    /**
     * Initialize field index with attributes
     * @param fieldNames        Field names
     * @param fieldList         Field list
     */
    void setAttributes(const Strings& fieldNames, std::initializer_list<WSType*> fieldList);

    /**
     * Get elements
     * @return const elements map
     */
    const std::map<String, WSType*>& elements() const { return m_elements; }

    /**
     * Get attributes
     * @return const attributes map
     */
    const std::map<String, WSType*>& attributes() const { return m_attributes; }

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
    void forEach(const std::function<bool(const String&,WSType*)>& method, FieldGroup fieldType=ELEMENTS_AND_ATTRIBUTES);

    /**
     * Execute the method for each field until it returns false
     * @param method            Method to execute
     */
    void forEach(const std::function<bool(const String&,const WSType*)>& method, FieldGroup fieldType=ELEMENTS_AND_ATTRIBUTES) const;

    bool hasElements() const;
    bool hasAttributes() const;

private:

    std::map<String, WSType*>   m_elements;
    std::map<String, WSType*>   m_attributes;
};

typedef std::shared_ptr<WSFieldIndex> SWSFieldIndex;

}
