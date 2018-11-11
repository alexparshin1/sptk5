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
 * WSDL operation
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
 * Parser of WSDL files
 *
 * Loads a WSDL file and converts it to a set of abstract classes that can be saved
 * as C++ source files to a directory.
 * The actual web service is built by deriving concrete classes from these abstract classes.
 */
class SP_EXPORT WSParser
{
public:
    /**
     * Map of element names to element objects
     */
    typedef std::map<String, WSParserElement*>      ElementMap;

    /**
     * Map of complex type names to complex type objects
     */
    typedef std::map<String,WSParserComplexType*>   ComplexTypeMap;

    /**
     * Map of element names to corresponding WSDL (XML) elements
     */
    typedef std::map<String,const xml::Element*>    XmlTypeMap;

    /**
     * Map of operation names to operation objects
     */
    typedef std::map<String,WSOperation>            OperationMap;

    /**
     * Map of operation names to operation objects
     */
    typedef std::map<String,String>                 DocumentationMap;

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
     * Parses xsd:element nodes directly under xsd:schema
     * @param element           Schema element
     */
    void parseElement(const xml::Element* element);

    /**
     * Parses xsd:complexType nodes directly under xsd:schema
     * @param complexTypeElement Schema complex type
     */
    void parseComplexType(const xml::Element* complexTypeElement);

    /**
     * Parses wsdl:operation nodes directly under xsd:schema
     * @param operation         Schema complex type
     */
    void parseOperation(xml::Element* operation);

    /**
     * Parses xsd:schema
     * @param schemaElement     Schema element
     */
    void parseSchema(xml::Element* schemaElement);

    /**
     * Generates service definition to output stream
     * @param usedClasses       List of this service complex types (classes)
     * @param output            Output stream
     */
    void generateDefinition(const Strings& usedClasses, std::ostream& output);

    /**
     * Generates service implementation to output stream
     * @param output            Output stream
     */
    void generateImplementation(std::ostream& output);

public:
    /**
     * Constructor
     */
    WSParser() = default;

    /**
     * Destructor
     */
    virtual ~WSParser();

    /**
     * Clears parsed data
     */
    void clear();

    /**
     * Loads WSDL-file and parses it to output classes
     * @param wsdlFile          WSDL file name
     */
    void parse(std::string wsdlFile);

    /**
     * Stores parsed classes to files in source directory
     * @param sourceDirectory   Directory to store output classes
     * @param headerFile        Optional header file to insert at the start of each generated file
     */
    void generate(std::string sourceDirectory=".", std::string headerFile="");

    /**
     * Utility function that removes namespace from the element name
     * @param name              Element name
     */
    static std::string strip_namespace(const std::string& name);

    /**
     * Utility function that returns namespace from the element name
     * @param name              Element name
     */
    static std::string get_namespace(const std::string& name);
};

}
#endif
