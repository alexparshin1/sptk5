/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        ParameterList.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __SPTK_PARAMETERLIST_H__
#define __SPTK_PARAMETERLIST_H__

#include <sptk5/db/QueryParameter.h>

#include <vector>
#include <map>

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief A vector of CParam*
 *
 * Doesn't mantain CParam memory.
 * Used to return a list of pointers on existing parameters.
 */
typedef std::vector<QueryParameter*> CParamVector;

/**
 * @brief Query parameters list.
 *
 * Has internal index to speed up the parameter search by name.
 * @see CQuery
 * @see CParam
 */
class SP_EXPORT QueryParameterList
{
    friend class Query;

    /**
     * The list of parameters
     */
    CParamVector                            m_items;

    /**
     * The parameters index
     */
    std::map<std::string, QueryParameter*>  m_index;

    /**
     * Indicates that one of the parameters binding type has changed since prepare()
     */
    bool                            m_bindingTypeChanged;

protected:
    /**
     * @brief Adds a parameter to the list
     */
    void add(QueryParameter* item);

public:
    /**
     * Query parameter iterator
     */
    typedef CParamVector::iterator          iterator;

    /**
     * Query parameter const iterator
     */
    typedef CParamVector::const_iterator    const_iterator;

    /**
     * @brief Default constructor
     */
    QueryParameterList();

    /**
     * @brief Destructor
     */
    ~QueryParameterList();

    /**
     * @brief Removes all the parameters from the list
     *
     * Releases any allocated resources
     */
    void clear();

    /**
     * @brief Returns parameter by name
     *
     * If the parameter isn't found, returns 0
     * @param paramName const char *, parameter name
     * @returns parameter pointer, or 0 if not found
     */
    QueryParameter* find(const char* paramName);

    /**
     * @brief Removes a parameter from the list and from the memory.
     * @param ndx uint32_t, parameter index in the list
     */
    void remove(uint32_t ndx);

    /**
     * @brief Parameter access by index
     * @param index int32_t, parameter index
     */
    QueryParameter& operator[](int32_t index) const;

    /**
     * @brief Parameter access by name
     * @param paramName const char *, parameter name
     */
    QueryParameter& operator[](const char* paramName) const;

    /**
     * @brief Parameter access by name
     * @param paramName const std::string&, parameter name
     */
    QueryParameter& operator[](const std::string& paramName) const;

    /**
     * @brief Returns parameter count
     */
    uint32_t size() const;

    /**
     * @brief Returns the parameter pointers
     *
     * A parameter is included for every parameter position in the query.
     * @param params CParamVector&, parameters vector
     */
    void enumerate(CParamVector& params);

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
};

/**
 * @}
 */
}

#endif
