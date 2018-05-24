/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSParser.h - description                               ║
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

#ifndef __SPTK_WSPARSER_H__
#define __SPTK_WSPARSER_H__

#include <sptk5/wsdl/WSParserComplexType.h>

namespace sptk
{

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * @brief WSDL operation
 */
struct WSOperation
{
    /**
     * WSDL operation input
     */
    WSParserComplexType*   m_input;

    /**
     * WSDL operation output
     */
    WSParserComplexType*   m_output;

};

/**
 * @brief Parser of WSDL files
 *
 * Loads a WSDL file and converts it to a set of abstract classes that can be saved
 * as C++ source files to a directory.
 * The actual web service is built by deriving concrete classes from these abstract classes.
 */
class SP_EXPORT WSParser
{
public:
    /**
     * @brief Map of element names to element objects
     */
    typedef std::map<String, WSParserElement*>     ElementMap;

    /**
     * @brief Map of complex type names to complex type objects
     */
    typedef std::map<String,WSParserComplexType*>  ComplexTypeMap;

    /**
     * @brief Map of element names to corresponding WSDL (XML) elements
     */
    typedef std::map<String,const XMLElement*>     XmlTypeMap;

    /**
     * @brief Map of operation names to operation objects
     */
    typedef std::map<String,WSOperation>          OperationMap;

    /**
     * @brief Map of operation names to operation objects
     */
    typedef std::map<String,String>           DocumentationMap;

private:
    /**
     * Service name, defining service class name and source file names
     */
    String              m_serviceName;

    /**
     * Map of all elements
     */
    ElementMap          m_elements;

    /**
     * Map of all parsed complex types
     */
    ComplexTypeMap      m_complexTypes;

    /**
     * Map of all operations
     */
    OperationMap        m_operations;

    /**
     * Map of all operation documentation if any
     */
    DocumentationMap    m_documentation;


protected:
    /**
     * @brief Parses xsd:element nodes directly under xsd:schema
     * @param element const XMLElement*, Schema element
     */
    void parseElement(const XMLElement* element);

    /**
     * @brief Parses xsd:complexType nodes directly under xsd:schema
     * @param complexTypeElement const XMLElement*, Schema complex type
     */
    void parseComplexType(const XMLElement* complexTypeElement);

    /**
     * @brief Parses wsdl:operation nodes directly under xsd:schema
     * @param operation XMLElement*, Schema complex type
     */
    void parseOperation(XMLElement* operation);

    /**
     * @brief Parses xsd:schema
     * @param schemaElement XMLElement*, Schema element
     */
    void parseSchema(XMLElement* schemaElement);

    /**
     * @brief Generates service definition to output stream
     * @param usedClasses const Strings&, List of this service complex types (classes)
     * @param output std::ostream, Output stream
     */
    void generateDefinition(const Strings& usedClasses, std::ostream& output);

    /**
     * @brief Generates service implementation to output stream
     * @param output std::ostream, Output stream
     */
    void generateImplementation(std::ostream& output);

public:
    /**
     * @brief Constructor
     */
    WSParser() = default;

    /**
     * @brief Destructor
     */
    virtual ~WSParser();

    /**
     * @brief Clears parsed data
     */
    void clear();

    /**
     * @brief Loads WSDL-file and parses it to output classes
     * @param wsdlFile std::string, WSDL file name
     */
    void parse(std::string wsdlFile);

    /**
     * @brief Stores parsed classes to files in source directory
     * @param sourceDirectory std::string, Directory to store output classes
     * @param headerFile std::string, Optional header file to insert at the start of each generated file
     */
    void generate(std::string sourceDirectory=".", std::string headerFile="");

    /**
     * @brief Utility function that removes namespace from the element name
     * @param name const std::string&, Element name
     */
    static std::string strip_namespace(const std::string& name);

    /**
     * @brief Utility function that returns namespace from the element name
     * @param name const std::string&, Element name
     */
    static std::string get_namespace(const std::string& name);
};

}
#endif
