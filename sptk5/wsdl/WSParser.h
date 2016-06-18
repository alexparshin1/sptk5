/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSParser.h - description                               ║
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

#ifndef __SPTK_WSPARSER_H__
#define __SPTK_WSPARSER_H__

#include <sptk5/wsdl/WSParserComplexType.h>

namespace sptk
{

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief WSDL operation
struct WSOperation
{
    WSParserComplexType*   m_input;    ///< WSDL operation input
    WSParserComplexType*   m_output;   ///< WSDL operation output
};

/// @brief Parser of WSDL files
///
/// Loads a WSDL file and converts it to a set of abstract classes that can be saved
/// as C++ source files to a directory.
/// The actual web service is built by deriving concrete classes from these abstract classes.
class SP_EXPORT WSParser
{
public:
    /// @brief Map of element names to element objects
    typedef std::map<std::string, WSParserElement*>     ElementMap;

    /// @brief Map of complex type names to complex type objects
    typedef std::map<std::string,WSParserComplexType*>  ComplexTypeMap;

    /// @brief Map of element names to corresponding WSDL (XML) elements
    typedef std::map<std::string,const XMLElement*>     XmlTypeMap;

    /// @brief Map of operation names to operation objects
    typedef std::map<std::string,WSOperation>          OperationMap;

    /// @brief Map of operation names to operation objects
    typedef std::map<std::string,std::string>           DocumentationMap;

private:
    std::string         m_serviceName;      ///< Service name, defining service class name and source file names
    ElementMap          m_elements;         ///< Map of all elements
    ComplexTypeMap      m_complexTypes;     ///< Map of all parsed complex types
    OperationMap        m_operations;       ///< Map of all operations
    DocumentationMap    m_documentation;    ///< Map of all operation documentation if any

protected:
    /// @brief Parses xsd:element nodes directly under xsd:schema
    /// @param element const XMLElement*, Schema element
    void parseElement(const XMLElement* element) THROWS_EXCEPTIONS;

    /// @brief Parses xsd:complexType nodes directly under xsd:schema
    /// @param complexTypeElement const XMLElement*, Schema complex type
    void parseComplexType(const XMLElement* complexTypeElement) THROWS_EXCEPTIONS;

    /// @brief Parses wsdl:operation nodes directly under xsd:schema
    /// @param operation XMLElement*, Schema complex type
    void parseOperation(XMLElement* operation) THROWS_EXCEPTIONS;

    /// @brief Parses xsd:schema
    /// @param schemaElement XMLElement*, Schema element
    void parseSchema(XMLElement* schemaElement) THROWS_EXCEPTIONS;

    /// @brief Generates service definition to output stream
    /// @param usedClasses const Strings&, List of this service complex types (classes)
    /// @param output std::ostream, Output stream
    void generateDefinition(const Strings& usedClasses, std::ostream& output) THROWS_EXCEPTIONS;

    /// @brief Generates service implementation to output stream
    /// @param output std::ostream, Output stream
    void generateImplementation(std::ostream& output) THROWS_EXCEPTIONS;

public:
    /// @brief Constructor
    WSParser();

    /// @brief Destructor
    virtual ~WSParser();

    /// @brief Clears parsed data
    void clear();

    /// @brief Loads WSDL-file and parses it to output classes
    /// @param wsdlFile std::string, WSDL file name
    void parse(std::string wsdlFile) THROWS_EXCEPTIONS;

    /// @brief Stores parsed classes to files in source directory
    /// @param sourceDirectory std::string, Directory to store output classes
    /// @param headerFile std::string, Optional header file to insert at the start of each generated file
    void generate(std::string sourceDirectory=".", std::string headerFile="") THROWS_EXCEPTIONS;

    /// @brief Utility function that removes namespace from the element name
    /// @param name const std::string&, Element name
    static std::string strip_namespace(const std::string& name);

    /// @brief Utility function that returns namespace from the element name
    /// @param name const std::string&, Element name
    static std::string get_namespace(const std::string& name);
};

}
#endif
