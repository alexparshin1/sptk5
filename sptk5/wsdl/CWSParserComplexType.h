/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSParserComplexType.h  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CWSPARSERCOMPLEXTYPE_H__
#define __CWSPARSERCOMPLEXTYPE_H__

#include <sptk5/cxml>
#include <sptk5/wsdl/CWSBasicTypes.h>
#include <iostream>
#include <list>

namespace sptk
{

/// @addtogroup wsdl WSDL-related Classes
/// @{

enum CWSMultiplicity {
    CWSM_REQUIRED       = 1,
    CWSM_OPTIONAL       = 2,
    CWSM_ZERO_OR_MORE   = 4,
    CWSM_ONE_OR_MORE    = 8
};

class CWSParserAttribute
{
    std::string     m_name;
    std::string     m_wsTypeName;
    std::string     m_cxxTypeName;
    CWSMultiplicity m_multiplicity;

public:
    CWSParserAttribute(std::string name="", std::string typeName="");
    CWSParserAttribute(const CWSParserAttribute& attr);
    std::string name() const { return m_name; }
    std::string generate() const;
};

/// @brief Parses WSDL complexType element
class CWSParserComplexType
{
    typedef std::map<std::string,CWSParserAttribute*>   AttributeMap;
    typedef std::list<CWSParserComplexType*>            ElementList;
protected:
    std::string         m_name;
    std::string         m_typeName;
    const CXmlElement*  m_element;
    AttributeMap        m_attributes;
    ElementList         m_sequence;
    CWSMultiplicity     m_multiplicity;
    int                 m_refcount;

    /// @brief Generates C++ class declaration
    void generateDefinition(std::ostream& classDeclaration) throw (std::exception);

    /// @brief Generates C++ class implementation
    void generateImplementation(std::ostream& classImplementation) throw (std::exception);
    
public:
    static std::string wsClassName(std::string);

public:
    /// @brief Constructor
    /// @param complexTypeElement const CXmlElement*, WSDL complexType element
    /// @param name std::string, Object name
    /// @param typeName std::string, Object types
    CWSParserComplexType(const CXmlElement* complexTypeElement, std::string name="", std::string typeName="");

    /// @brief Destructor
    virtual ~CWSParserComplexType();
    
    int refCount()
    {
        return m_refcount;
    }

    void increaseRefCount()
    {
        m_refcount++;
    }

    void decreaseRefCount()
    {
        if (m_refcount > 0)
            m_refcount--;
        else
            throwException("Can't decrease complex type refcount: refcount is less than 1");
    }

    std::string name() const
    {
        return m_name;
    }
    
    std::string className() const;

    CWSMultiplicity multiplicity() const
    {
        return m_multiplicity;
    }
    
    /// @brief Parses WSDL complexType element
    virtual void parse() throw (std::exception);
    
    void parseSequence(CXmlElement* sequence) throw (std::exception);
    
    /// @brief Generates C++ class declaration and implementation
    void generate(std::ostream& classDeclaration, std::ostream& classImplementation) throw (std::exception);
};

class CWSParserElement
{
public:
    std::string                 m_name;
    const CWSParserComplexType* m_complexType;
public:
    /// @brief Constructor
    /// @param name std::string, Element name
    /// @param complexType const CWSParserComplexType*, Complex type used by element
    CWSParserElement(std::string name, const CWSParserComplexType* complexType)
    : m_name(name), m_complexType(complexType)
    {}
};

/// @}

}
#endif
