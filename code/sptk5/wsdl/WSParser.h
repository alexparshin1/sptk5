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

#include <sptk5/wsdl/WSParserComplexType.h>
#include <sptk5/wsdl/WSOperation.h>
#include <sptk5/wsdl/OpenApiGenerator.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

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
    using ElementMap = std::map<String, const WSParserElement*>;

    class ComplexTypeIndex
    {
    public:
        void addType(const sptk::String& elementName, const SWSParserComplexType& complexType)
        {
            m_complexTypes[elementName] = complexType;
        }

        void add(const sptk::String& elementName, const SWSParserComplexType& complexType)
        {
            m_complexTypes[elementName] = complexType;
            m_elements[elementName] = complexType.get();
        }

        void clear()
        {
            m_complexTypes.clear();
            m_elements.clear();
        }

        [[nodiscard]] const WSParserElement* element(const sptk::String& elementName, const sptk::String& context) const
        {
            auto itor = m_elements.find(elementName);
            if (itor == m_elements.end())
            {
                throw Exception(context + ": Element '" + elementName + "' not found");
            }
            return itor->second;
        }

        [[nodiscard]] const ElementMap& elements() const
        {
            return m_elements;
        }

        [[nodiscard]] SWSParserComplexType complexType(const sptk::String& elementName,
                                                       const sptk::String& context) const
        {
            auto itor = m_complexTypes.find(elementName);
            if (itor == m_complexTypes.end())
            {
                throw Exception(context + ": Complex type '" + elementName + "' not found");
            }
            return itor->second;
        }

        [[nodiscard]] const WSComplexTypeMap& complexTypes() const
        {
            return m_complexTypes;
        }

    private:

        ElementMap m_elements;         ///< Map of all elements
        WSComplexTypeMap m_complexTypes;     ///< Map of all parsed complex types
    };

    /**
     * Map of element names to corresponding WSDL (XML) elements
     */
    using XmlTypeMap = std::map<String, const xml::Element*>;

    /**
     * Map of operation names to operation objects
     */
    using DocumentationMap = std::map<String, String>;

    /**
     * Constructor
     */
    WSParser() = default;

    WSParser(const WSParser& other) = delete;

    WSParser(WSParser&& other) = delete;

    /**
     * Destructor
     */
    virtual ~WSParser() = default;

    WSParser& operator=(const WSParser& other) = delete;

    WSParser& operator=(WSParser&& other) = delete;

    /**
     * Clears parsed data
     */
    void clear();

    /**
     * Loads WSDL-file and parses it to output classes
     * @param wsdlFile          WSDL file name
     */
    void parse(const fs::path& wsdlFile);

    /**
     * Stores parsed classes to files in source directory
     * @param sourceDirectory   Directory to store output classes
     * @param headerFile        Optional header file to insert at the start of each generated file
     */
    void generate(const String& sourceDirectory = ".", const String& headerFile = "",
                  const OpenApiGenerator::Options& options = OpenApiGenerator::Options(), bool verbose = false,
                  const String& serviceNamespace = "");

    /**
     * Stores WSDL to C++ file
     * @param sourceDirectory   Directory to store output files
     * @param headerFile        Optional header file to insert at the start of each generated file
     * @param wsdlFileName              WSDL content
     */
    void generateWsdlCxx(const String& sourceDirectory, const String& headerFile, const fs::path& wsdlFileName) const;

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

    const String& description() const;

protected:
    /**
     * Parses xsd:element nodes directly under xsd:schema
     * @param element           Schema element
     */
    void parseElement(const xml::Element* element);

    /**
     * Parses xsd:simpleType nodes directly under xsd:schema
     * @param simpleTypeElement Schema simple type
     */
    static void parseSimpleType(const xml::Element* simpleTypeElement);

    /**
     * Parses xsd:complexType nodes directly under xsd:schema
     * @param complexTypeElement Schema complex type
     */
    void parseComplexType(const xml::Element* complexTypeElement);

    /**
     * Parses wsdl:operation nodes directly under xsd:schema
     * @param operationNode         Schema complex type
     */
    void parseOperation(const xml::Element* operationNode);

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
    void generateImplementation(std::ostream& output) const;

private:

    String m_serviceName;      ///< Service name, defining service class name and source file names
    String m_serviceNamespace; ///< Service classes namespace
    String m_description;      ///< Service description
    String m_location;         ///< Service location
    String m_wsdlFile;         ///< WSDL source file name
    ComplexTypeIndex m_complexTypeIndex; ///< Index of all parsed complex types and elements
    WSOperationMap m_operations;       ///< Map of all operations
    DocumentationMap m_documentation;    ///< Map of documentation
};

/**
 * @}
 */

}
