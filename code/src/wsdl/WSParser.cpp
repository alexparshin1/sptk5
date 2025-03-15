/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Printer.h>
#include <sptk5/wsdl/OpenApiGenerator.h>
#include <sptk5/wsdl/SourceModule.h>
#include <sptk5/wsdl/WSMessageIndex.h>
#include <sptk5/wsdl/WSParser.h>

#include <fstream>

using namespace std;
using namespace sptk;

namespace {
void replaceFile(const String& fileName, const stringstream& fileData)
{
    const uint8_t empty = 0;
    const String  str = fileData.str();
    Buffer        newData(fileData.str());
    if (newData.empty())
    {
        newData.set(&empty, 1);
    }

    Buffer oldData(&empty, 1);
    try
    {
        oldData.loadFromFile(fileName.c_str());
    }
    catch (const Exception&)
    {
        // Doesn't exist?
        oldData.bytes(0);
    }

    if (String(oldData.c_str()) != String(newData.c_str()))
    {
        newData.saveToFile(fileName.c_str());
    }
}
} // namespace

void WSParser::clear()
{
    m_complexTypeIndex.clear();
}

void WSParser::parseElement(const xdoc::SNode& elementNode)
{
    const auto elementName = elementNode->attributes().get("name");
    auto       elementType = elementNode->attributes().get("type");

    if (const size_t namespacePos = elementType.find(':');
        namespacePos != string::npos)
    {
        elementType = elementType.substr(namespacePos + 1);
    }

    SWSParserComplexType complexType;
    if (!elementType.empty())
    {
        complexType = m_complexTypeIndex.complexType(elementType, "Element " + elementName);
    }
    else
    {
        // Element defines type inline
        complexType = m_complexTypeIndex.complexType(elementName, "Element " + elementName);
    }
    if (complexType)
    {
        m_complexTypeIndex.add(elementName, complexType);
    }
}

void WSParser::parseSimpleType(const xdoc::SNode& simpleTypeElement)
{
    auto simpleTypeName = simpleTypeElement->attributes().get("name");
    if (simpleTypeName.empty())
    {
        return;
    }

    simpleTypeName = "tns:" + simpleTypeName;

    if (WSParserComplexType::findSimpleType(simpleTypeName))
    {
        throw Exception("Duplicate simpleType definition: " + simpleTypeName);
    }

    WSParserComplexType::SimpleTypeElements[simpleTypeName] = simpleTypeElement;
}

void WSParser::parseComplexType(xdoc::SNode& complexTypeElement)
{
    auto complexTypeName = complexTypeElement->attributes().get("name");
    if (complexTypeName.empty())
    {
        complexTypeName = complexTypeElement->parent()->attributes().get("name");
    }

    if (complexTypeName.empty())
    {
        const auto& parent = complexTypeElement->parent();
        complexTypeName = parent->attributes().get("name");
    }

    if (const auto& complexTypes = m_complexTypeIndex.complexTypes();
        complexTypes.contains(complexTypeName))
    {
        throw Exception("Duplicate complexType definition: " + complexTypeName);
    }

    const auto complexType = make_shared<WSParserComplexType>(complexTypeElement, complexTypeName);
    m_complexTypeIndex.addType(complexTypeName, complexType);
    complexType->parse();
}

void WSParser::parseOperation(const xdoc::SNode& operationNode)
{
    auto document = operationNode;
    while (document->parent())
    {
        document = document->parent();
    }

    const xdoc::Node::Vector messageNodes = document->select("//wsdl:message");

    map<String, String> messageToElementMap;
    for (const auto& message: messageNodes)
    {
        const auto part = message->findFirst("wsdl:part");
        const auto messageName = message->attributes().get("name");
        const auto elementName = stripNamespace(part->attributes().get("element"));
        messageToElementMap[messageName] = elementName;
        const auto documentationNode = part->findFirst("wsdl:documentation");
        if (documentationNode != nullptr)
        {
            m_documentation[elementName] = documentationNode->getText().trim();
        }
    }

    WSOperation operation = {};
    bool        found = false;
    for (const auto& element: operationNode->nodes())
    {
        auto message = element->attributes().get("message");
        if (const size_t pos = message.find(':');
            pos != string::npos)
        {
            message = message.substr(pos + 1);
        }
        const auto& elementName = messageToElementMap[message];
        if (element->getQualifiedName() == "wsdl:input")
        {
            operation.m_input = m_complexTypeIndex.complexType(elementName, "Message " + message);
            found = true;
            continue;
        }
        if (element->getQualifiedName() == "wsdl:output")
        {
            operation.m_output = m_complexTypeIndex.complexType(message, "Message " + message);
            found = true;
            continue;
        }
    }

    if (found)
    {
        const auto operationName = operationNode->attributes().get("name");
        m_operations[operationName] = operation;
    }
}

void WSParser::parseSchema(const xdoc::SNode& schemaElement)
{
    for (const auto  simpleTypeNodes = schemaElement->select("//xsd:simpleType");
         const auto& element: simpleTypeNodes)
    {
        if (element->getQualifiedName() == "xsd:simpleType")
        {
            parseSimpleType(element);
        }
    }

    for (auto  complexTypeNodes = schemaElement->select("//xsd:complexType");
         auto& element: complexTypeNodes)
    {
        if (element->getQualifiedName() == "xsd:complexType")
        {
            parseComplexType(element);
        }
    }

    for (const auto& element: schemaElement->nodes())
    {
        if (element->getQualifiedName() == "xsd:element")
        {
            parseElement(element);
        }
    }
}

void WSParser::parse(const filesystem::path& wsdlFile)
{
    m_wsdlFile = wsdlFile.string();

    xdoc::Document wsdlXML;
    Buffer         buffer;
    buffer.loadFromFile(wsdlFile);
    wsdlXML.load(buffer);

    const auto service = wsdlXML.root()->findFirst("wsdl:service");
    m_serviceName = service->attributes().get("name");
    m_serviceNamespace = m_serviceName.toLowerCase() + "_service";

    const auto schemaElement = wsdlXML.root()->findFirst("xsd:schema");
    if (schemaElement == nullptr)
    {
        throw Exception("Can't find xsd:schema element");
    }

    // Locate and read targetNamespace
    m_targetNamespace = wsdlXML.root()->attributes().get("targetNamespace");
    if (m_targetNamespace.empty())
    {
        auto targetNamespaceNode = wsdlXML.root()->findFirst("targetNamespace");
        if (targetNamespaceNode)
        {
            m_targetNamespace = targetNamespaceNode->getText();
        }
    }
    if (m_targetNamespace.empty())
    {
        m_targetNamespace = schemaElement->attributes().get("targetNamespace");
    }

    if (const auto address = service->findFirst("soap:address");
        address != nullptr)
    {
        m_location = address->attributes().get("location");
    }

    parseSchema(schemaElement);

    const auto portElement = wsdlXML.root()->findFirst("wsdl:portType");
    if (portElement == nullptr)
    {
        throw Exception("Can't find wsdl:portType element");
    }

    if (const auto descriptionElement = portElement->findFirst("wsdl:documentation");
        descriptionElement != nullptr)
    {
        m_description = descriptionElement->getText();
    }

    for (const auto& element: portElement->nodes())
    {
        if (element != nullptr && element->getQualifiedName() == "wsdl:operation")
        {
            parseOperation(element);
        }
    }
}

String capitalize(const String& name)
{
    Strings parts(lowerCase(name), "_");
    for (auto& part: parts)
    {
        part[0] = static_cast<char>(toupper(part[0]));
    }
    return parts.join("");
}

String WSParser::stripNamespace(const String& name)
{
    const size_t pos = name.find(':');
    if (pos == string::npos)
    {
        return name;
    }
    return name.substr(pos + 1);
}

String WSParser::getNamespace(const String& name)
{
    const size_t pos = name.find(':');
    if (pos == string::npos)
    {
        return name;
    }
    return name.substr(0, pos);
}

void WSParser::generateDefinition(const Strings& usedClasses, ostream& output)
{
    const string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

    output << "// Web Service " << m_serviceName << " definition\n\n";
    output << "#pragma once\n";

    output << "#include \""
           << "C" + capitalize(m_serviceName) + "WSDL.h\"\n";
    output << "#include <sptk5/wsdl/WSRequest.h>\n";
    output << "#include <sptk5/net/HttpAuthentication.h>\n\n";

    output << "// This Web Service types\n";
    for (const auto& usedClass: usedClasses)
    {
        output << "#include \"" << usedClass << ".h\"\n";
    }
    output << "\n";

    output << "namespace " << m_serviceNamespace << " {\n\n";

    output << "/**\n";
    output << " * Base class for service method.\n";
    output << " *\n";
    output << " * Web Service application derives its service class from this class\n";
    output << " * by overriding abstract methods\n";
    output << " */\n";
    output << "class WS_EXPORT " << serviceClassName << " : public sptk::WSRequest\n";
    output << "{\n";
    output << "public:\n";
    output << "    /**\n";
    output << "     * Constructor\n";
    output << "     * @param logEngine        Optional log engine for error messages\n";
    output << "     */\n";
    output << "    explicit " << serviceClassName << "(sptk::LogEngine* logEngine=nullptr);\n\n";

    output << "    /**\n";
    output << "     * Destructor\n";
    output << "     */\n";
    output << "    ~" << serviceClassName << "() override = default;\n\n";

    output << "    // Abstract methods below correspond to WSDL-defined operations.\n";
    output << "    // Application must overwrite these methods with processing of corresponding\n";
    output << "    // requests, reading data from input and writing data to output structures.\n";
    for (const auto& [name, operation]: m_operations)
    {
        output << "\n";
        output << "    /**\n";
        output << "     * Web Service " << name << " operation\n";
        output << "     *\n";
        if (const auto documentation = m_documentation[operation.m_input->name()];
            !documentation.empty())
        {
            const Strings documentationRows(documentation, "\n");
            for (const auto& row: documentationRows)
            {
                output << "     * " << trim(row) << "\n";
            }
        }
        output
            << "     * This method is abstract and must be overwritten by derived Web Service implementation class."
            << "\n";
        output << "     * @param input            Operation input data\n";
        output << "     * @param output           Operation response data\n";
        output << "     */\n";
        output
            << "    virtual void " << name
            << "(const " << operation.m_input->className() << "& input, "
            << operation.m_output->className() << "& output, sptk::HttpAuthentication* auth) = 0;\n";
    }
    output << "\n";

    output << "    /**\n";
    output << "     * @return original WSDL specifications\n";
    output << "     */\n";
    output << "    sptk::String wsdl() const override;\n\n";

    output << "    /**\n";
    output << "     * @return OpenAPI specifications\n";
    output << "     */\n";
    output << "    sptk::String openapi() const override;\n\n";

    output << "    /**\n";
    output << "     * @return SOAP WebService targetNamespace\n";
    output << "     */\n";
    output << "    sptk::String targetNamespace() const\n";
    output << "    {\n"
           << "        return \"" << m_targetNamespace << "\";\n"
           << "    }\n\n";

    output << "private:\n\n";

    for (const auto& [name, operation]: m_operations)
    {
        const auto requestName = stripNamespace(operation.m_input->name());
        output << "    /**\n";
        output << "     * Internal Web Service " << requestName << " processing\n";
        output << "     * @param requestNode      Operation input/output XML data\n";
        output << "     * @param authentication   Optional HTTP authentication\n";
        output << "     * @param requestNameSpace Request SOAP element namespace\n";
        output << "     */\n";
        output << "    void process_" << requestName
               << "(const sptk::xdoc::SNode& xmlContent, const sptk::xdoc::SNode& jsonContent, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);"
               << "\n\n";
    }
    output << "};\n\n";
    output << "using S"
           << capitalize(m_serviceName) + "ServiceBase = "
           << "std::shared_ptr<" << serviceClassName << ">;\n\n";
    output << "}\n";
}

void WSParser::generateImplementation(ostream& output) const
{
    const string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

    Strings serviceOperations;
    for (const auto& [name, operation]: m_operations)
    {
        const String requestName = stripNamespace(operation.m_input->name());
        serviceOperations.push_back(requestName);
    }
    const String         operationNames = serviceOperations.join("|");
    const WSMessageIndex serviceOperationsIndex(serviceOperations);

    output << "#include \"" << serviceClassName << ".h\"\n";
    output << "#include <sptk5/wsdl/WSParser.h>\n";
    output << "#include <sptk5/wsdl/WSMessageIndex.h>\n";
    output << "#include <functional>\n";
    output << "#include <set>\n\n";

    output << "using namespace std;\n";
    output << "using namespace placeholders;\n";
    output << "using namespace sptk;\n";
    output << "using namespace " << m_serviceNamespace << ";\n\n";

    output << serviceClassName << "::" << serviceClassName << "(LogEngine* logEngine)\n";
    output << ": WSRequest(targetNamespace(), logEngine)\n";
    output << "{\n";
    output << "    map<String, RequestMethod> requestMethods {\n\n";
    const auto lastOperationName = m_operations.rbegin()->first;
    for (const auto& [name, operation]: m_operations)
    {
        const auto requestName = stripNamespace(operation.m_input->name());
        output << "        {\"" << requestName << "\", \n"
               << "            [this](const xdoc::SNode& xmlNode, const xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)\n"
               << "            {\n"
               << "                process_" << requestName << "(xmlNode, jsonNode, authentication, requestNameSpace);\n"
               << "            }}";
        if (name != lastOperationName)
        {
            output << ",";
        }
        output << "\n\n";
    }
    output << "    };\n";
    output << "    setRequestMethods(std::move(requestMethods));\n";
    output << "}\n"
           << "\n";

    output << "\n"
           << "template <class InputData, class OutputData>\n"
              "void processAnyRequest(const xdoc::SNode& requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace,\n"
              "                       function<void(const InputData& input, OutputData& output, HttpAuthentication* authentication)>& method)\n"
              "{\n"
              "   const String requestName = InputData::classId();\n"
              "   const String responseName = OutputData::classId();\n"
              "   String ns(requestNameSpace.getAlias());\n"
              "   InputData inputData((ns + \":\" + requestName).c_str());\n"
              "   OutputData outputData((ns + \":\" + responseName).c_str());\n"
              "   try {\n"
              "      inputData.load(requestNode);\n"
              "   }"
              "   catch (const Exception& e) {\n"
              "      // Can't parse input data\n"
              "      constexpr int badRequest = 400;\n"
              "      throw HTTPException(badRequest, e.what());\n"
              "   }\n"
              "   const auto& soapBody = requestNode->parent();\n"
              "   soapBody->clearChildren();\n"
              "   method(inputData, outputData, authentication);\n"
              "   xdoc::SNode response;\n"
              "   if (requestNameSpace.getLocation().empty() || requestNameSpace.getLocation() == \"http://tempuri.org/\") {\n"
              "      response = soapBody->pushNode(xdoc::NodeName(responseName, \"resns\"));\n"
              "      response->attributes().set(\"xmlns:resns\", \"http://tempuri.org/\");\n"
              "   } else {\n"
              "      response = soapBody->pushNode(xdoc::NodeName(responseName, ns));\n"
              "      response->attributes().set(\"xmlns:\" + ns, requestNameSpace.getLocation());\n"
              "   }\n"
              "   outputData.unload(response);\n"
              "}\n\n"

              "template <class InputData, class OutputData>\n"
              "void processAnyRequest(const xdoc::SNode& request, HttpAuthentication* authentication,\n"
              "                       const function<void(const InputData&, OutputData&, HttpAuthentication*)>& method)\n"
              "{\n"
              "   InputData inputData;\n"
              "   OutputData outputData;\n"
              "   try {\n"
              "      inputData.load(request);\n"
              "   }\n"
              "   catch (const Exception& e) {\n"
              "      // Can't parse input data\n"
              "      constexpr int badRequest = 400;\n"
              "      throw HTTPException(badRequest, e.what());\n"
              "   }\n"
              "   method(inputData, outputData, authentication);\n"
              "   request->clear();\n"
              "   outputData.unload(request);\n"
              "}\n\n";

    for (const auto& [operationName, operation]: m_operations)
    {
        Strings nameParts(operation.m_input->name(), ":");
        String  requestName;
        if (nameParts.size() == 1)
        {
            requestName = nameParts[0];
        }
        else
        {
            requestName = nameParts[1];
        }
        output << "\n";
        output << "void " << serviceClassName << "::process_" << requestName
               << "(const xdoc::SNode& xmlNode, const xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)"
               << "\n";

        const auto inputType = "C" + operation.m_input->name();
        const auto outputType = "C" + operation.m_output->name();
        output << "{\n"
               << "    function<void(const " << inputType << "&, " << outputType << "&, HttpAuthentication*)> method =\n"
               << "        [this](const " << inputType << "& request, " << outputType << "& response, HttpAuthentication* auth)\n"
               << "        {\n"
               << "            " << operationName << "(request, response, auth);\n"
               << "        };\n\n"
               << "    if (xmlNode)\n"
               << "        processAnyRequest<" << inputType << "," << outputType
               << ">(xmlNode, authentication, requestNameSpace, method);\n"
               << "    else\n"
               << "        processAnyRequest<" << inputType << "," << outputType
               << ">(jsonNode, authentication, method);\n"
               << "}\n";
    }
    output << "\n";

    output << "String " << serviceClassName << "::wsdl() const\n";
    output << "{\n";
    output << "    return " << m_serviceName << "_wsdl;\n";
    output << "}\n";

    output << "String " << serviceClassName << "::openapi() const\n";
    output << "{\n";
    output << "    return " << m_serviceName << "_openapi;\n";
    output << "}\n";
}

void WSParser::generate(const String& sourceDirectory, const String& headerFile,
                        const OpenApiGenerator::Options& options,
                        bool verbose, const String& serviceNamespace)
{
    if (!serviceNamespace.empty())
    {
        m_serviceNamespace = serviceNamespace;
    }

    Buffer externalHeader;
    if (!headerFile.empty())
    {
        externalHeader.loadFromFile(headerFile.c_str());
    }
    else
    {
        externalHeader.set("");
    }

    const String serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";
    if (verbose)
    {
        COUT("Creating service class " << serviceClassName);
    }

    stringstream cmakeLists;
    cmakeLists << "# The following list of files is generated automatically.\n";
    cmakeLists << "# Please don't edit it, or your changes may be overwritten.\n\n";
    cmakeLists << "SET (" << m_serviceName << "_files\n";
    cmakeLists << "  " << sourceDirectory << "/" << serviceClassName << ".cpp "
               << sourceDirectory << "/" << serviceClassName << ".h\n";

    const String wsdlFileName = "C" + capitalize(m_serviceName) + "WSDL";
    cmakeLists << "  " << sourceDirectory << "/" << wsdlFileName << ".cpp "
               << sourceDirectory << "/" << wsdlFileName << ".h\n";

    Strings usedClasses;
    for (const auto& [name, complexType]: m_complexTypeIndex.complexTypes())
    {
        SourceModule sourceModule(String("C") + complexType->name(), sourceDirectory);
        sourceModule.open();
        complexType->generate(sourceModule.header(), sourceModule.source(), externalHeader.c_str(), m_serviceNamespace);
        usedClasses.push_back("C" + complexType->name());
        cmakeLists << "  " << sourceDirectory << "/C" << complexType->name() << ".cpp "
                   << sourceDirectory << "/C" << complexType->name() << ".h\n";
        sourceModule.writeOutputFiles();
    }

    // Generate Service class definition
    SourceModule serviceModule(serviceClassName, sourceDirectory);
    serviceModule.open();

    if (!externalHeader.empty())
    {
        serviceModule.header() << externalHeader.c_str() << "\n";
        serviceModule.source() << externalHeader.c_str() << "\n";
    }

    generateDefinition(usedClasses, serviceModule.header());
    generateImplementation(serviceModule.source());

    serviceModule.writeOutputFiles();

    cmakeLists << ")\n";

    replaceFile(m_serviceName + ".inc", cmakeLists);

    const OpenApiGenerator openApiGenerator(m_serviceName, m_description, "1.0.0", {m_location}, options);
    auto                   openApiFileName = options.openApiFile;
    if (openApiFileName.empty())
    {
        openApiFileName = m_wsdlFile.replace("\\.wsdl$", "") + ".json";
    }

    if (verbose)
    {
        COUT("Creating OpenAPI file " << openApiFileName);
    }

    ofstream openApiFile(openApiFileName);
    openApiGenerator.generate(openApiFile, m_operations, m_complexTypeIndex.complexTypes(), m_documentation);
    openApiFile.close();
}

namespace {
void fileToCxxStream(const filesystem::path& fileName, const String& variableName, stringstream& cxxStream)
{
    Strings content;
    content.loadFromFile(fileName);

    cxxStream << "const sptk::Strings " << variableName << "Rows({\n";

    bool first = true;
    for (const auto& row: content)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            cxxStream << ",\n";
        }
        cxxStream << "    R\"(" << row << ")\"";
    }

    cxxStream << "\n});\n\n";

    cxxStream << "const sptk::String " << variableName << "(" << variableName << "Rows.join(\"\\n\"));\n";
}
} // namespace

void WSParser::generateWsdlCxx(const String& sourceDirectory, const String& headerFile,
                               const filesystem::path& _wsdlFileName, const filesystem::path& openApiFileName) const
{
    Buffer externalHeader("// Auto-generated by wsdl2cxx\n");
    if (!headerFile.empty())
    {
        externalHeader.loadFromFile(headerFile.c_str());
    }

    const auto baseFileName = "C" + capitalize(m_serviceName) + "WSDL";
    const auto wsdlFileName = sourceDirectory + "/" + baseFileName;
    const auto wsdlVariableName = m_serviceName + "_wsdl";
    const auto openApiVariableName = m_serviceName + "_openapi";

    stringstream wsdlHeader;
    wsdlHeader << externalHeader.c_str() << "\n";
    wsdlHeader << "#pragma once\n\n";
    wsdlHeader << "#include <sptk5/Strings.h>\n\n";
    wsdlHeader << "extern const sptk::String " << wsdlVariableName << ";\n"
               << "extern const sptk::String " << openApiVariableName << ";\n\n";

    stringstream wsdlCxx;
    wsdlCxx << externalHeader.c_str() << "\n";
    wsdlCxx << "#include \"" << baseFileName << ".h\"\n\n";

    fileToCxxStream(_wsdlFileName, wsdlVariableName, wsdlCxx);
    wsdlCxx << "\n";
    fileToCxxStream(openApiFileName, openApiVariableName, wsdlCxx);

    replaceFile(wsdlFileName + ".h", wsdlHeader);
    replaceFile(wsdlFileName + ".cpp", wsdlCxx);
}

const String& WSParser::description() const
{
    return m_description;
}
