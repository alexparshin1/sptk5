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

#include <sptk5/xdoc/Document.h>
#include <sptk5/wsdl/WSBasicTypes.h>
#include <iostream>
#include <list>
#include <set>
#include "WSRestriction.h"

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * Multiplicity flag
 */
enum class WSMultiplicity
    : uint8_t
{
    REQUIRED = 1,     ///< Element is required
    ZERO_OR_ONE = 2,  ///< Element is optional
    ZERO_OR_MORE = 4, ///< Element may occur 0 or more times
    ONE_OR_MORE = 8   ///< Element may occur one or more times
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
    String name() const
    {
        return m_name;
    }

    /**
     * Generates attribute presentation in C++ skeleton
     */
    String generate(bool initialize) const;

    /**
     * Returns attribute C++ type name
     */
    String cxxTypeName() const
    {
        return m_cxxTypeName;
    }

    /**
     * Returns attribute WSDL type name
     */
    String wsTypeName() const
    {
        return m_wsTypeName;
    }

private:

    String m_name;         ///< Attribute name
    String m_wsTypeName;   ///< Attribute type name
    String m_cxxTypeName;  ///< C++ type name
};

class WSParserComplexType;

using SWSParserComplexType = std::shared_ptr<WSParserComplexType>;
using WSParserComplexTypeList = std::list<SWSParserComplexType>;

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
    explicit WSParserComplexType(const xdoc::Node* complexTypeElement, const String& name = "",
                                 const String& typeName = "");

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
        return ((int) m_multiplicity & ((int) WSMultiplicity::ZERO_OR_MORE | (int) WSMultiplicity::ONE_OR_MORE)) != 0;
    }

    /**
     * Get child elements sequence
     * @return Child elements sequence
     */
    WSParserComplexTypeList sequence() const
    {
        return m_sequence;
    }

    /**
     * Get optional restriction
     * @return restriction
     */
    SWSRestriction restriction() const
    {
        return m_restriction;
    }

    /**
     * Parses WSDL complexType element
     */
    virtual void parse();

    /**
     * Parses WSDL child sequence
     */
    void parseSequence(const xdoc::Node* sequence);

    /**
     * Generates C++ class declaration and implementation
     */
    void generate(std::ostream& classDeclaration, std::ostream& classImplementation,
                  const String& externalHeader, const String& serviceNamespace) const;

    static std::map<String, const xdoc::Node*> SimpleTypeElements;

    static const xdoc::Node* findSimpleType(const String& typeName);

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

    class Initializer
    {
    public:
        Strings ctor {"WSComplexType(elementName, optional)"};
        Strings copyCtor {"WSComplexType(other)"};
        Strings moveCtor {"WSComplexType(std::move(other))"};
        Strings copyAssign;
        Strings moveAssign;
    };

    class ImplementationParts
    {
    public:
        std::stringstream declarations;
        std::stringstream body;
        std::stringstream checks;

        size_t restrictionNumber {0};

        void print(std::ostream& output) const;

        void printImplementationLoadArray(const SWSParserComplexType& complexType);

        String appendRestrictionIfDefined(const SWSParserComplexType& complexType);

        void printImplementationLoadField(Strings& requiredElements, const SWSParserComplexType& complexType);
    };

    /**
     * Map of attribute names to attribute objects
     */
    using AttributeMap = std::map<std::string, WSParserAttribute*, std::less<>>;

    String m_name;                             ///< Element name
    String m_typeName;                         ///< WSDL type name
    const xdoc::Node* m_element {nullptr};     ///< XML element for that WSDL element
    AttributeMap m_attributes;                 ///< Element attributes
    WSParserComplexTypeList m_sequence;        ///< Child element sequence
    WSMultiplicity m_multiplicity;             ///< Multiplicity flag
    SWSRestriction m_restriction;              ///< Element restriction (if any) or NULL
    String m_documentation;                    ///< Optional documentation

    /**
     * Generate includes for C++ class
     * @param classImplementation   Output stream
     * @param className             Class name
     */
    static void printImplementationIncludes(std::ostream& classImplementation, const String& className);

    void printImplementationRestrictions(std::ostream& classImplementation, std::ostream& checks) const;

    static void printDeclarationIncludes(std::ostream& classDeclaration, const std::set<String>& usedClasses);

    std::set<String> getUsedClasses() const;

    static void appendMemberDocumentation(std::ostream& classDeclaration, const SWSParserComplexType& complexType);

    void appendClassAttributes(std::ostream& classDeclaration, Strings& fieldNames,
                               Initializer& initializer) const;

    String addOptionalRestriction(std::ostream& implementation, const SWSParserComplexType& complexType,
                                  size_t& restrictionIndex) const;

    static String makeTagName(const String& className);

    Initializer makeInitializer() const;

    void printImplementationConstructors(std::ostream& classImplementation, const String& className,
                                         const Strings& elementNames, const Strings& attributeNames) const;

    void printImplementationAssignments(std::ostream& classImplementation, const String& className) const;

    void printImplementationCheckRestrictions(std::ostream& classImplementation, const String& className) const;

    static void generateSetFieldIndex(std::ostream& classDeclaration, const Strings& elementNames,
                                      const Strings& attributeNames);

    static String jsonAttributeOutputMethod(const String& wsTypeName);
};

/**
 * Alias for WSDL complex type
 */
using WSParserElement = WSParserComplexType;

/**
 * Map of complex type names to complex type objects
 */
using WSComplexTypeMap = std::map<String, SWSParserComplexType>;

/**
 * @}
 */
}
