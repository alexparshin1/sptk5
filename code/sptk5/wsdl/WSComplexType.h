/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSComplexType.h - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __SPTK_WSCOMPLEXTYPE_H__
#define __SPTK_WSCOMPLEXTYPE_H__

#include <sptk5/cxml>
#include <sptk5/Variant.h>
#include <sptk5/FieldList.h>
#include <sptk5/wsdl/WSBasicTypes.h>
#include <sptk5/db/QueryParameterList.h>
#include <sptk5/threads/Locks.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * Base type for all user WSDL types
 */
class WSComplexType : public WSTypeName
{
    /**
     * Mutex that protects internal data
     */
    mutable SharedMutex m_mutex;

    /**
    * WSDL element name
    */
    String       m_name;

    /**
     * Element optionality flag
     */
    bool         m_optional;

    /**
     * Is data loaded flag
     */
    bool         m_loaded;

protected:

    const bool loaded() const
    {
        return m_loaded;
    }

    void setLoaded(bool flag)
    {
        m_loaded = flag;
    }

    /**
     * Internal clear data
     */
    virtual void _clear()
    {
        // Implemented in derived classes
    }

public:
    /**
     * Default constructor
     * @param name              Element name
     * @param optional          Element optionality flag
     */
    WSComplexType(const char* name, bool optional=false)
    : m_name(name), m_optional(optional), m_loaded(false)
    {}

    /**
     * Copy constructor
     * @param other             Other object
     */
    WSComplexType(const WSComplexType& other)
    : m_name(other.m_name), m_optional(other.m_optional), m_loaded(other.m_loaded)
    {}

    /**
     * Move constructor
     * @param other             Other object
     */
    WSComplexType(WSComplexType&& other)
    : m_name(std::move(other.m_name)), m_optional(other.m_optional), m_loaded(other.m_loaded)
    {
        other.m_optional = false;
        other.m_loaded = false;
    }

    /**
     * Destructor
     */
    virtual ~WSComplexType()
    {}

    /**
     * Copy assignment
     * @param other             Other object
     */
    WSComplexType& operator = (const WSComplexType& other)
    {
        if (&other != this) {
            m_name = other.m_name;
            m_optional = other.m_optional;
            m_loaded = other.m_loaded;
        }
        return *this;
    }

    /**
     * Move assignment
     * @param other             Other object
     */
    WSComplexType& operator = (WSComplexType&& other)
    {
        if (&other != this) {
            m_name = std::move(other.m_name);
            m_optional = other.m_optional;
            m_loaded = other.m_loaded;
            other.m_optional = false;
            other.m_loaded = false;
        }
        return *this;
    }

    /**
     * Return class name
     */
    String className() const override
    {
        SharedLock(m_mutex);
        return "WSComplexType";
    }

    /**
     * Clear content and releases allocated memory
     */
    virtual void clear()
    {
        UniqueLock(m_mutex);
        _clear();
    }

    /**
     * Copy data from other object
     * @param other             Object to copy from
     */
    void copyFrom(const WSComplexType& other);

    /**
     * Load data from XML node
     * @param input             XML node containing CAddHandler data
     */
    virtual void load(const xml::Element* input) = 0;

    /**
     * Load data from FieldList
     *
     * Only simple WSDL type members are loaded.
     * @param input             Query field list containing CMqType data
     */

    virtual void load(const sptk::FieldList& input) = 0;

    /**
     * Unload data to existing XML node
     * @param output            Existing XML node
     */
    virtual void unload(xml::Element* output) const = 0;

    /**
     * Unload data to Query's parameters
     * @param output            Query parameters
     */
    virtual void unload(QueryParameterList& output) const = 0;

    /**
     * Unload single element or attribute to DB query parameter
     * @param output            Query parameters
     * @param paramName         Query parameter name
     * @param elementOrAttribute Complex type element (not an array!)
     */
    static void unload(QueryParameterList& output, const char* paramName, const WSBasicType* elementOrAttribute);

    /**
     * Unload data to new XML node
     * @param parent            Parent XML node where new node is created
     */
    virtual void addElement(xml::Element* parent) const;

    /**
     * True is data was loaded
     */
    virtual bool isNull() const
    {
        SharedLock(m_mutex);
        return !m_loaded;
    }

    /**
     * Returns element name
     */
    std::string complexTypeName() const
    {
        SharedLock(m_mutex);
        return m_name;
    }

    /**
     * True is element is optional
     */
    virtual bool isOptional() const
    {
        SharedLock(m_mutex);
        return m_optional;
    }

    /*
     * Print element as XML text
     */
    String toString(bool asJSON=true) const;
};

/**
 * @}
 */

}
#endif
