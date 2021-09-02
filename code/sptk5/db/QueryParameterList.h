/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#include <sptk5/db/QueryParameter.h>

#include <map>
#include <vector>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * A vector of CParam*
 *
 * Doesn't mantain CParam memory.
 * Used to return a list of pointers on existing parameters.
 */
using CParamVector = std::vector<SQueryParameter>;

/**
 * Query parameters list.
 *
 * Has internal index to speed up the parameter search by name.
 * @see CQuery
 * @see CParam
 */
class SP_EXPORT QueryParameterList
{
    friend class Query;

public:
    /**
     * Query parameter iterator
     */
    using iterator = CParamVector::iterator;

    /**
     * Query parameter const iterator
     */
    using const_iterator = CParamVector::const_iterator;

    /**
     * Removes all the parameters from the list
     *
     * Releases any allocated resources
     */
    void clear();

    /**
     * Returns parameter by name
     *
     * If the parameter isn't found, returns 0
     * @param paramName         parameter name
     * @returns parameter pointer, or 0 if not found
     */
    SQueryParameter find(const String& paramName);

    /**
     * Removes a parameter from the list and from the memory.
     * @param ndx               parameter index in the list
     */
    void remove(size_t ndx);

    /**
     * Parameter access by index
     * @param index             parameter index
     */
    QueryParameter& operator[](size_t index) const;

    /**
     * Parameter access by name
     * @param paramName         parameter name
     */
    QueryParameter& operator[](const String& paramName) const;

    /**
     * Returns parameter count
     */
    size_t size() const;

    /**
     * Returns the parameter pointers
     *
     * A parameter is included for every parameter position in the query.
     * @param params            parameters vector
     */
    void enumerate(CParamVector& params) const;

    /**
     * First parameter iterator
     */
    iterator begin()
    {
        return m_items.begin();
    }

    /**
     * First parameter const iterator
     */
    const_iterator begin() const
    {
        return m_items.begin();
    }

    /**
     * End iterator
     */
    iterator end()
    {
        return m_items.end();
    }

    /**
     * End const iterator
     */
    const_iterator end() const
    {
        return m_items.end();
    }

protected:
    /**
     * Adds a parameter to the list
     */
    void add(const SQueryParameter& item);

private:
    CParamVector m_items;                      ///< The list of parameters
    std::map<String, SQueryParameter> m_index; ///< The parameters index
    bool m_bindingTypeChanged {true};          ///< Indicates that one of the parameters binding type has changed since prepare()
};

/**
 * @}
 */
} // namespace sptk
