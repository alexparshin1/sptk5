/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSParserComplexType.h - description                    ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __SPTK_WSPARSERCOMPLEXTYPE_H__
#define __SPTK_WSPARSERCOMPLEXTYPE_H__

#include <sptk5/cxml>
#include <sptk5/wsdl/WSBasicTypes.h>
#include <iostream>
#include <list>
#include <set>
#include "WSRestriction.h"

namespace sptk
{

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * Multiplicity flag
 */
enum WSMultiplicity {
    /**
     * Element is required
     */
    WSM_REQUIRED       = 1,

    /**
     * Element is optional
     */
    WSM_OPTIONAL       = 2,

    /**
     * Element may occur 0 or more times
     */
    WSM_ZERO_OR_MORE   = 4,

    /**
     * Element may occur one or more times
     */
    WSM_ONE_OR_MORE    = 8

};

/**
 * WSDL element attribute
 */
class WSParserAttribute
{
    /**
     * Attribute name
     */
    String          m_name;

    /**
     * Attribute type name
     */
    String          m_wsTypeName;

    /**
     * C++ type name
     */
    String          m_cxxTypeName;

public:
    /**
     * Constructor
     * @param name              Attribute name
     * @param typeName          Attribute WSDL type name
     */
    WSParserAttribute(const String& name = "", const String& typeName = "");

    /**
     * Copy constructor
     * @param attr              Attribute to copy from
     */
    WSParserAttribute(const WSParserAttribute& attr) = default;

    /**
     * Returns attribute name
     */
    String name() const { return m_name; }

    /**
     * Generates attribute presentation in C++ skeleton
     */
    String generate() const;

    /**
     * Returns attribute C++ type name
     */
    std::string cxxTypeName() const { return m_cxxTypeName; }

    /**
     * Returns attribute WSDL type name
     */
    std::string wsTypeName() const { return m_wsTypeName; }
};

/**
 * Parses WSDL complexType element
 */
class WSParserComplexType
{
    /**
     * Map of attribute names to attribute objects
     */
    typedef std::map<std::string,WSParserAttribute*>   AttributeMap;

    /**
     * List of complex type elements
     */
    typedef std::list<WSParserComplexType*>            ElementList;

protected:

    /**
     * Element name
     */
    String                  m_name;

    /**
     * WSDL type name
     */
    String                  m_typeName;

    /**
     * XML element for that WSDL element
     */
    const xml::Element*       m_element;

    /**
     * Element attributes
     */
    AttributeMap            m_attributes;

    /**
     * Child element sequence
     */
    ElementList             m_sequence;

    /**
     * Multiplicity flag
     */
    WSMultiplicity          m_multiplicity;

    /**
     * Object reference count
     */
    int                     m_refcount;

    /**
     * Element restriction (if any) or NULL
     */
    WSRestriction*          m_restriction;

    /**
     * Optional documentation
     */
    String                  m_documentation;


    /**
     * Generate C++ class declaration
     * @param classDeclaration std::ostream&, Output header file stream
     */
    void generateDefinition(std::ostream& classDeclaration);

    /**
     * Generate C++ class implementation
     * @param classImplementation std::ostream&, Output implementation file stream
     */
    void generateImplementation(std::ostream& classImplementation);

public:
    /**
     * WSDL class name
     */
    static String wsClassName(const String&);

public:
    /**
     * Constructor
     * @param complexTypeElement WSDL complexType element
     * @param name              Object name
     * @param typeName          Object types
     */
    WSParserComplexType(const xml::Element* complexTypeElement, const String& name = "", const String& typeName = "");

    /**
     * Destructor
     */
    virtual ~WSParserComplexType();

    /**
     * Returns element reference count
     */
    int refCount()
    {
        return m_refcount;
    }

    /**
     * Increases element reference count
     */
    void increaseRefCount()
    {
        m_refcount++;
    }

    /**
     * Decreases element reference count
     */
    void decreaseRefCount()
    {
        if (m_refcount > 0)
            m_refcount--;
        else
            throwException("Can't decrease complex type refcount: refcount is less than 1");
    }

    /**
     * WSDL element name
     */
    String name() const
    {
        return m_name;
    }

    /**
     * Generated C++ class name
     */
    String className() const;

    /**
     * Multiplicity flag
     */
    WSMultiplicity multiplicity() const
    {
        return m_multiplicity;
    }

    /**
     * Is the type an array?
     */
    bool isArray() const
    {
        return (m_multiplicity & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) != 0;
    }

    /**
     * Documentation
     */
    String documentation() const
    {
        return m_documentation;
    }

    /**
     * Parses WSDL complexType element
     */
    virtual void parse();

    /**
     * Parses WSDL child sequence
     */
    void parseSequence(xml::Element* sequence);

    /**
     * Generates C++ class declaration and implementation
     */
    void generate(std::ostream& classDeclaration, std::ostream& classImplementation, const String& externalHeader);
};

/**
 * Alias for WSDL complex type
 */
typedef WSParserComplexType WSParserElement;

/**
 * @}
 */

}
#endif
