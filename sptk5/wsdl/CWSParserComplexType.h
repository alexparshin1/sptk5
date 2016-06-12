/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWSParserComplexType.h - description                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CWSPARSERCOMPLEXTYPE_H__
#define __CWSPARSERCOMPLEXTYPE_H__

#include <sptk5/cxml>
#include <sptk5/wsdl/CWSBasicTypes.h>
#include <iostream>
#include <list>
#include <set>
#include "CWSRestriction.h"

namespace sptk
{

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Multiplicity flag
enum CWSMultiplicity {
    CWSM_REQUIRED       = 1,    ///< Element is required
    CWSM_OPTIONAL       = 2,    ///< Element is optional
    CWSM_ZERO_OR_MORE   = 4,    ///< Element may occur 0 or more times
    CWSM_ONE_OR_MORE    = 8     ///< Element may occur one or more times
};

/// @brief WSDL element attribute
class CWSParserAttribute
{
    std::string     m_name;         ///< Attribute name
    std::string     m_wsTypeName;   ///< Attribute type name
    std::string     m_cxxTypeName;  ///< C++ type name
    CWSMultiplicity m_multiplicity; ///< Multiplicity flag

public:
    /// @brief Constructor
    /// @param name std::string, Attribute name
    /// @param typeName std::string, Attribute WSDL type name
    CWSParserAttribute(std::string name="", std::string typeName="");

    /// @brief Copy constructor
    /// @param attr const CWSParserAttribute&, Attribute to copy from
    CWSParserAttribute(const CWSParserAttribute& attr);

    /// @brief Returns attribute name
    std::string name() const { return m_name; }

    /// @brief Generates attribute presentation in C++ skeleton
    std::string generate() const;

    /// @brief Returns attribute C++ type name
    std::string cxxTypeName() const { return m_cxxTypeName; }

    /// @brief Returns attribute WSDL type name
    std::string wsTypeName() const { return m_wsTypeName; }
};

/// @brief Parses WSDL complexType element
class CWSParserComplexType
{
    /// @brief Map of attribute names to attribute objects
    typedef std::map<std::string,CWSParserAttribute*>   AttributeMap;

    /// @brief List of complex type elements
    typedef std::list<CWSParserComplexType*>            ElementList;
protected:
    std::string             m_name;             ///< Element name
    std::string             m_typeName;         ///< WSDL type name
    const CXmlElement*      m_element;          ///< XML element for that WSDL element
    AttributeMap            m_attributes;       ///< Element attributes
    ElementList             m_sequence;         ///< Child element sequence
    CWSMultiplicity         m_multiplicity;     ///< Multiplicity flag
    int                     m_refcount;         ///< Object reference count
    WSRestriction*          m_restriction;      ///< Element restriction (if any) or NULL

    /// @brief Generates C++ class declaration
    void generateDefinition(std::ostream& classDeclaration) THROWS_EXCEPTIONS;

    /// @brief Generates C++ class implementation
    void generateImplementation(std::ostream& classImplementation) THROWS_EXCEPTIONS;

public:
    /// @brief WSDL class name
    static std::string wsClassName(std::string);

public:
    /// @brief Constructor
    /// @param complexTypeElement const CXmlElement*, WSDL complexType element
    /// @param name std::string, Object name
    /// @param typeName std::string, Object types
    CWSParserComplexType(const CXmlElement* complexTypeElement, std::string name="", std::string typeName="");

    /// @brief Destructor
    virtual ~CWSParserComplexType();

    /// @brief Returns element reference count
    int refCount()
    {
        return m_refcount;
    }

    /// @brief Increases element reference count
    void increaseRefCount()
    {
        m_refcount++;
    }

    /// @brief Decreases element reference count
    void decreaseRefCount()
    {
        if (m_refcount > 0)
            m_refcount--;
        else
            throwException("Can't decrease complex type refcount: refcount is less than 1");
    }

    /// @brief WSDL element name
    std::string name() const
    {
        return m_name;
    }

    /// @brief Generated C++ class name
    std::string className() const;

    /// @brief Multiplicity flag
    CWSMultiplicity multiplicity() const
    {
        return m_multiplicity;
    }

    /// @brief Parses WSDL complexType element
    virtual void parse() THROWS_EXCEPTIONS;

    /// @brief Parses WSDL child sequence
    void parseSequence(CXmlElement* sequence) THROWS_EXCEPTIONS;

    /// @brief Generates C++ class declaration and implementation
    void generate(std::ostream& classDeclaration, std::ostream& classImplementation, std::string externalHeader) THROWS_EXCEPTIONS;
};

/// @brief Alias for WSDL complex type
typedef CWSParserComplexType CWSParserElement;

/// @}

}
#endif
