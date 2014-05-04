/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSParser.h  -  description
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

#ifndef __CWSPARSER_H__
#define __CWSPARSER_H__

#include <sptk5/wsdl/CWSParserComplexType.h>

namespace sptk
{

/// @addtogroup wsdl WSDL-related Classes
/// @{

struct CWSOperation
{
    CWSParserComplexType*   m_input;
    CWSParserComplexType*   m_output;
};
    
/// @brief Parser of WSDL files
///
/// Loads a WSDL file and converts it to a set of abstract classes that can be saved 
/// as C++ source files to a directory.
/// The actual web service is built by deriving concrete classes from these abstract classes.
class SP_EXPORT CWSParser
{
public:
    typedef std::map<std::string,CWSParserElement*>     ElementMap;
    typedef std::map<std::string,CWSParserComplexType*> ComplexTypeMap;
    typedef std::map<std::string,const CXmlElement*>    XmlTypeMap;
    typedef std::map<std::string,CWSOperation>          OperationMap;

private:
    std::string     m_serviceName;      ///< Service name, defining service class name and source file names
    ElementMap      m_elements;         ///< Map of all elements
    ComplexTypeMap  m_complexTypes;     ///< Map of all parsed complex types
    OperationMap    m_operations;       ///< Map of all operations

protected:
    /// @brief Parses xsd:element nodes directly under xsd:schema
    /// @param element const CXmlElement*, Schema element
    void parseElement(const CXmlElement* element) THROWS_EXCEPTIONS;

    /// @brief Parses xsd:complexType nodes directly under xsd:schema
    /// @param complexTypeElement const CXmlElement*, Schema complex type
    void parseComplexType(const CXmlElement* complexTypeElement) THROWS_EXCEPTIONS;

    /// @brief Parses wsdl:operation nodes directly under xsd:schema
    /// @param operation const CXmlElement*, Schema complex type
    void parseOperation(const CXmlElement* operation) THROWS_EXCEPTIONS;

    /// @brief Parses xsd:schema
    /// @param schemaElement const CXmlElement*, Schema element
    void parseSchema(const CXmlElement* schemaElement) THROWS_EXCEPTIONS;

    /// @brief Generates service definition to output stream
    /// @param usedClasses const CStrings&, List of this service complex types (classes)
    /// @param output std::ostream, Output stream
    void generateDefinition(const CStrings& usedClasses, std::ostream& output) THROWS_EXCEPTIONS;

    /// @brief Generates service implementation to output stream
    /// @param output std::ostream, Output stream
    void generateImplementation(std::ostream& output) THROWS_EXCEPTIONS;

public:
    /// @brief Constructor
    CWSParser();

    /// @brief Destructor
    virtual ~CWSParser();
    
    /// @brief Clears parsed data
    void clear();

    /// @brief Loads WSDL-file and parses it to output classes
    /// @param wsdlFile std::string, WSDL file name
    void parse(std::string wsdlFile) THROWS_EXCEPTIONS;

    /// @brief Stores parsed classes to files in source directory
    /// @param sourceDirectory std::string, Directory to store output classes
    void generate(std::string sourceDirectory=".") THROWS_EXCEPTIONS;

    static std::string strip_namespace(const std::string& name);
};

}
#endif
