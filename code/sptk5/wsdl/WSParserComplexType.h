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
enum WSMultiplicity
{
    WSM_REQUIRED       = 1, ///< Element is required
    WSM_OPTIONAL       = 2, ///< Element is optional
    WSM_ZERO_OR_MORE   = 4, ///< Element may occur 0 or more times
    WSM_ONE_OR_MORE    = 8  ///< Element may occur one or more times
};

/**
 * WSDL element attribute
 */
class SP_EXPORT WSParserAttribute
{
public:
    /**
     * Constructor
     * @param name              Attribute name
     * @param typeName          Attribute WSDL type name
     */
    explicit WSParserAttribute(const String& name = "", const String& typeName = "");

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
    String generate(bool initialize) const;

    /**
     * Returns attribute C++ type name
     */
    String cxxTypeName() const { return m_cxxTypeName; }

    /**
     * Returns attribute WSDL type name
     */
    String wsTypeName() const { return m_wsTypeName; }

private:

    String          m_name;         ///< Attribute name
    String          m_wsTypeName;   ///< Attribute type name
    String          m_cxxTypeName;  ///< C++ type name
};

class WSParserComplexType;
typedef std::shared_ptr<WSParserComplexType>    SWSParserComplexType;
typedef std::list<SWSParserComplexType>         WSParserComplexTypeList;

/**
 * Parses WSDL complexType element
 */
class SP_EXPORT WSParserComplexType
{
public:
    /**
     * WSDL class name
     */
    static String wsClassName(const String& className);

    /**
     * Constructor
     * @param complexTypeElement WSDL complexType element
     * @param name              Object name
     * @param typeName          Object types
     */
    explicit WSParserComplexType(const xml::Element* complexTypeElement, const String& name = "", const String& typeName = "");

    /**
     * Destructor
     */
    virtual ~WSParserComplexType() = default;

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
     * Get child elements sequence
     * @return Child elements sequence
     */
    const WSParserComplexTypeList sequence() const { return m_sequence; }

    /**
     * Get optional restriction
     * @return restriction
     */
    SWSRestriction restriction() const { return m_restriction; }

    /**
     * Parses WSDL complexType element
     */
    virtual void parse();

    /**
     * Parses WSDL child sequence
     */
    void parseSequence(const xml::Element* sequence);

    /**
     * Generates C++ class declaration and implementation
     */
    void generate(std::ostream& classDeclaration, std::ostream& classImplementation,
                  const String& externalHeader, const String& serviceNamespace) const;

    static std::map<String, const xml::Element*> SimpleTypeElements;
    static const xml::Element* findSimpleType(const String& typeName);

protected:

    /**
     * Generate C++ class declaration
     * @param classDeclaration std::ostream&, Output header file stream
     */
    void generateDefinition(std::ostream& classDeclaration, sptk::Strings& fieldNames,
                            sptk::Strings& elementNames, sptk::Strings& attributeNames,
                            const String& serviceNamespace) const;

    /**
     * Generate C++ class implementation
     * @param classImplementation std::ostream&, Output implementation file stream
     */
    void generateImplementation(std::ostream& classImplementation, const Strings& fieldNames,
                                const Strings& elementNames, const Strings& attributeNames,
                                const String& serviceNamespace) const;

private:

    class ImplementationParts
    {
    public:
        std::stringstream   declarations;
        std::stringstream   body;
        std::stringstream   checks;

        size_t              restrictionNumber {0};

        void print(std::ostream& output) const;
        void printImplementationLoadArray(const SWSParserComplexType& complexType);
        String appendRestrictionIfDefined(const SWSParserComplexType& complexType);
        void printImplementationLoadField(Strings& requiredElements, const SWSParserComplexType& complexType);
    };

    /**
     * Map of attribute names to attribute objects
     */
    typedef std::map<std::string, WSParserAttribute*>   AttributeMap;

    String                  m_name;                 ///< Element name
    String                  m_typeName;             ///< WSDL type name
    const xml::Element*     m_element {nullptr};    ///< XML element for that WSDL element
    AttributeMap            m_attributes;           ///< Element attributes
    WSParserComplexTypeList m_sequence;             ///< Child element sequence
    WSMultiplicity          m_multiplicity;         ///< Multiplicity flag
    SWSRestriction          m_restriction;          ///< Element restriction (if any) or NULL
    String                  m_documentation;        ///< Optional documentation

    /**
     * Generate includes for C++ class
     * @param classImplementation   Output stream
     * @param className             Class name
     */
    void printImplementationIncludes(std::ostream& classImplementation, const String& className) const;

    /**
     * Generate C++ class unload() to XML method
     * @param classImplementation   Output stream
     * @param className             Class name
     */
    void printImplementationUnloadXML(std::ostream& classImplementation, const String& className) const;

    /**
     * Generate C++ class unload() to JSON method
     * @param classImplementation   Output stream
     * @param className             Class name
     */
    void printImplementationUnloadJSON(std::ostream& classImplementation, const String& className) const;

    /**
     * Generate C++ class unload() to ParamList method
     * @param classImplementation   Output stream
     * @param className             Class name
     */
    void printImplementationUnloadParamList(std::ostream& classImplementation, const String& className) const;

    void printImplementationRestrictions(std::ostream& classImplementation, std::ostream& checks) const;

    void printDeclarationIncludes(std::ostream& classDeclaration, const std::set<String>& usedClasses) const;

    std::set<String> getUsedClasses() const;

    void makeImplementationLoadAttributes(std::stringstream& fieldLoads, int& fieldLoadCount) const;

    void appendMemberDocumentation(std::ostream& classDeclaration, const SWSParserComplexType& complexType) const;

    void appendClassAttributes(std::ostream& classDeclaration, Strings& fieldNames,
                               Strings& copyInitializer, Strings& moveInitializer) const;

    String addOptionalRestriction(std::ostream& implementation, const SWSParserComplexType& complexType,
                                  size_t& restrictionIndex) const;

    void printImplementationCheckRestrictions(std::ostream& classImplementation, const String& className) const;

    void generateSetFieldIndex(std::ostream& classDeclaration, const Strings& elementNames,
                               const Strings& attributeNames) const;
};

/**
 * Alias for WSDL complex type
 */
typedef WSParserComplexType WSParserElement;

/**
 * Map of complex type names to complex type objects
 */
typedef std::map<String, SWSParserComplexType>  WSComplexTypeMap;

/**
 * @}
 */
}
