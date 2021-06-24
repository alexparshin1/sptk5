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

#include <sptk5/cxml>
#include <sptk5/json/JsonElement.h>
#include <sptk5/Variant.h>
#include <sptk5/FieldList.h>
#include <sptk5/wsdl/WSBasicTypes.h>
#include <sptk5/wsdl/WSArray.h>
#include <sptk5/wsdl/WSFieldIndex.h>
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
class SP_EXPORT WSComplexType
    : public WSType
{
public:

    /**
     * Default constructor
     * @param name              Element name
     * @param optional          Element optionality flag
     */
    WSComplexType(const char* name, bool optional = false)
        : m_name(name), m_optional(optional)
    {
    }

    /**
     * Copy constructor
     * @param other             Other object
     */
    WSComplexType(const WSComplexType& other)
        : m_name(other.m_name),
          m_optional(other.m_optional),
          m_loaded(other.m_loaded)
    {
    }

    /**
     * Move constructor
     * @param other             Other object
     */
    WSComplexType(WSComplexType&& other) noexcept
        : m_name(other.m_name),
          m_optional(std::exchange(other.m_optional, false)),
          m_loaded(std::exchange(other.m_loaded, false))
    {
    }

    /**
     * Destructor
     */
    virtual ~WSComplexType() = default;

    /**
     * Copy assignment
     * @param other             Other object
     */
    WSComplexType& operator=(const WSComplexType& other)
    {
        if (&other != this)
        {
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
    WSComplexType& operator=(WSComplexType&& other) noexcept
    {
        if (&other != this)
        {
            _clear();
            m_name = other.m_name;
            m_optional = std::exchange(other.m_optional, false);
            m_loaded = std::exchange(other.m_loaded, false);
        }
        return *this;
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSComplexType";
    }

    /**
     * Clear content and releases allocated memory
     */
    void clear() override
    {
        _clear();
    }

    /**
     * Copy data from other object
     * @param other             Object to copy from
     */
    void copyFrom(const WSComplexType& other);

    /**
     * Loads type data from request XML node
     * @param input             XML node
     */
    void load(const xml::Node* input) override;

    /**
     * Loads type data from request JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override;

    /**
     * Load data from FieldList
     *
     * Only simple WSDL type members are loaded.
     * @param input             Query field list containing CMqType data
     */
    virtual void load(const sptk::FieldList& input);

    /**
     * Unload data to existing XML node
     * @param output            Existing XML node
     */
    virtual void unload(xml::Node* output) const;

    /**
     * Unload data to existing JSON node
     * @param output            Existing JSON node
     */
    virtual void unload(json::Element* output) const;

    /**
     * Unload data to Query's parameters
     * @param output            Query parameters
     */
    virtual void unload(QueryParameterList& output) const;

    /**
     * Unload single element or attribute to DB query parameter
     * @param output            Query parameters
     * @param paramName         Quermy parameter name
     * @param elementOrAttribute Complex type element (not an array!)
     */
    static void unload(QueryParameterList& output, const char* paramName, const WSBasicType* elementOrAttribute);

    /**
     * Unload data to new XML node
     * @param parent            Parent XML node where new node is created
     * @param name              Optional name for the child element
     */
    void addElement(xml::Node* parent, const char* name = nullptr) const override;

    /**
     * Unload data to new JSON node
     * @param parent            Parent JSON node where new node is created
     */
    void addElement(json::Element* parent) const override;

    /**
     * True if data was not loaded, or if all the fields are null.
     */
    bool isNull() const override;

    /**
     * Returns element name
     */
    [[nodiscard]] String name() const override
    {
        return m_name;
    }

    /**
     * True is element is optional
     */
    virtual bool isOptional() const
    {
        return m_optional;
    }

    /**
     * Print element as XML text
     * @param asJSON            Output is JSON (true) or XML (false)
     * @return object presentation as JSON or XML string
     */
    virtual String toString(bool asJSON = true, bool formatted = false) const;

    /**
     * Throw SOAPException is the object is null
     * @param parentTypeName    Parent object type name
     */
    void throwIfNull(const String& parentTypeName) const;

    /**
     * If object is exportable, it's included during export to JSON or XML
     * @param flag              Exportable flag
     */
    void exportable(bool flag)
    {
        m_exportable = flag;
    }

protected:

    /**
     * @return true if object is loaded
     */
    bool loaded() const
    {
        return m_loaded;
    }

    /**
     * Set loaded flag
     * @param flag              If true then object is loaded
     */
    void setLoaded(bool flag)
    {
        m_loaded = flag;
    }

    /**
     * Internal clear data
     */
    virtual void _clear()
    {
        m_fields.forEach([](WSType* field) {
            field->clear();
            return true;
        });
    }

    const WSFieldIndex& getFields() const
    {
        return m_fields;
    }

    void setElements(const Strings& fieldNames, std::initializer_list<WSType*> fields)
    {
        m_fields.setElements(fieldNames, fields);
    }

    void setAttributes(const Strings& fieldNames, std::initializer_list<WSType*> fields)
    {
        m_fields.setAttributes(fieldNames, fields);
    }

    /**
     * Optional checking for restrictions
     */
    virtual void checkRestrictions() const
    {
        // Implement in derived class
    }

private:
    String m_name;                ///< WSDL element name
    bool m_optional {false};    ///< Element optionality flag
    bool m_loaded {false};      ///< Is data loaded flag
    bool m_exportable {true};   ///< Is this object exportable?
    WSFieldIndex m_fields;              ///< All fields
    void setAttributes(const std::map<String, String>& values, json::Element* attributes) const;
};

/**
 * @}
 */

}
