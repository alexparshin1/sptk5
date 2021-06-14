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

#include <iomanip>
#include <sptk5/cutils>
#include <sptk5/wsdl/SourceModule.h>
#include <sptk5/wsdl/WSParser.h>
#include <sptk5/wsdl/WSMessageIndex.h>
#include <sptk5/wsdl/OpenApiGenerator.h>

using namespace std;
using namespace sptk;

static void replaceFile(const String& fileName, const stringstream& fileData)
{
    String str = fileData.str();
    Buffer newData(fileData.str());
    if (newData.empty())
        newData.set("", 1);

    Buffer oldData("", 1);
    try {
        oldData.loadFromFile(fileName);
    }
    catch (const Exception&) {
        // Doesn't exist?
        oldData.bytes(0);
    }

    if (String(oldData.c_str()) != String(newData.c_str()))
        newData.saveToFile(fileName);
}

WSParser::~WSParser()
{
    try {
        clear();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl)
    }
}

void WSParser::clear()
{
    m_complexTypeIndex.clear();
}

void WSParser::parseElement(const xml::Element* elementNode)
{
    String elementName = (String) elementNode->getAttribute("name");
    String elementType = (String) elementNode->getAttribute("type");

    size_t namespacePos = elementType.find(':');
    if (namespacePos != string::npos)
        elementType = elementType.substr(namespacePos + 1);

    SWSParserComplexType complexType;
    if (!elementType.empty()) {
        complexType = m_complexTypeIndex.complexType(elementType, "Element " + elementName);
    } else {
        // Element defines type inline
        complexType = m_complexTypeIndex.complexType(elementName, "Element " + elementName);
    }
    if (complexType) {
        m_complexTypeIndex.add(elementName, complexType);
    }
}

void WSParser::parseSimpleType(const xml::Element* simpleTypeElement) const
{
    String simpleTypeName = (String) simpleTypeElement->getAttribute("name");
    if (simpleTypeName.empty())
        return;

    simpleTypeName = "tns:" + simpleTypeName;

    if (WSParserComplexType::findSimpleType(simpleTypeName))
        throwException("Duplicate simpleType definition: " << simpleTypeName)

    WSParserComplexType::SimpleTypeElements[simpleTypeName] = simpleTypeElement;
}

void WSParser::parseComplexType(const xml::Element* complexTypeElement)
{
    String complexTypeName = (String) complexTypeElement->getAttribute("name");
    if (complexTypeName.empty())
        complexTypeName = (String) complexTypeElement->parent()->getAttribute("name");

    if (complexTypeName.empty()) {
        const xml::Node* parent = complexTypeElement->parent();
        complexTypeName = (String) parent->getAttribute("name");
    }

    auto& complexTypes = m_complexTypeIndex.complexTypes();
    if (complexTypes.find(complexTypeName) != complexTypes.end())
        throwException("Duplicate complexType definition: " << complexTypeName)

    auto complexType = make_shared<WSParserComplexType>(complexTypeElement, complexTypeName);
    m_complexTypeIndex.addType(complexTypeName, complexType);
    complexType->parse();
}

void WSParser::parseOperation(const xml::Element* operationNode)
{
    xml::NodeVector messageNodes;
    operationNode->document()->select(messageNodes, "//wsdl:message");

    map<String, String> messageToElementMap;
    for (auto* node: messageNodes) {
        if (node->type() != xml::Node::DOM_ELEMENT)
            throw Exception("The node " + node->name() + " is not an XML element");
        const auto* message = dynamic_cast<xml::Element*>(node);
        const auto* part = message->findFirst("wsdl:part");
        auto messageName = (String) message->getAttribute("name");
        auto elementName = strip_namespace((String) part->getAttribute("element"));
        messageToElementMap[messageName] = elementName;
        const auto* documentationNode = part->findFirst("wsdl:documentation");
        if (documentationNode != nullptr)
            m_documentation[elementName] = documentationNode->text().trim();
    }

    WSOperation operation = {};
    bool found = false;
    for (const auto* node: *operationNode) {
        if (node->type() != xml::Node::DOM_ELEMENT)
            throw Exception("The node " + node->name() + " is not an XML element");
        auto* element = dynamic_cast<const xml::Element*>(node);
        String message = (String) element->getAttribute("message");
        size_t pos = message.find(':');
        if (pos != string::npos)
            message = message.substr(pos+1);
        string elementName = messageToElementMap[message];
        if (element->name() == "wsdl:input") {
            operation.m_input = m_complexTypeIndex.complexType(elementName, "Message " + message);
            found = true;
            continue;
        }
        if (element->name() == "wsdl:output") {
            operation.m_output = m_complexTypeIndex.complexType(message, "Message " + message);
            found = true;
            continue;
        }
    }

    if (found) {
        String operationName = (String) operationNode->getAttribute("name");
        m_operations[operationName] = operation;
    }
}

void WSParser::parseSchema(xml::Element* schemaElement)
{
    xml::NodeVector simpleTypeNodes;
    schemaElement->select(simpleTypeNodes, "//xsd:simpleType");

    for (const auto* node: simpleTypeNodes) {
        auto* element = dynamic_cast<const xml::Element*>(node);
        if (element != nullptr && element->name() == "xsd:simpleType")
            parseSimpleType(element);
    }

    xml::NodeVector complexTypeNodes;
    schemaElement->select(complexTypeNodes, "//xsd:complexType");

    for (const auto* node: complexTypeNodes) {
        auto* element = dynamic_cast<const xml::Element*>(node);
        if (element != nullptr && element->name() == "xsd:complexType")
            parseComplexType(element);
    }

    for (const auto* node: *schemaElement) {
        auto* element = dynamic_cast<const xml::Element*>(node);
        if (element != nullptr && element->name() == "xsd:element")
            parseElement(element);
    }
}

void WSParser::parse(String wsdlFile)
{
    m_wsdlFile = wsdlFile;

    xml::Document wsdlXML;
    Buffer buffer;
    buffer.loadFromFile(wsdlFile);
    wsdlXML.load(buffer);

    const auto* service = (xml::Element*) wsdlXML.findFirst("wsdl:service");
    m_serviceName = (String) service->getAttribute("name");
    m_serviceNamespace = m_serviceName.toLowerCase() + "_service";

    const auto* address = service->findFirst("soap:address");
    if (address)
        m_location = (String) address->getAttribute("location");

    auto* schemaElement = dynamic_cast<xml::Element*>(wsdlXML.findFirst("xsd:schema"));
    if (schemaElement == nullptr)
        throwException("Can't find xsd:schema element")
    parseSchema(schemaElement);

    const auto* portElement = dynamic_cast<xml::Element*>(wsdlXML.findFirst("wsdl:portType"));
    if (portElement == nullptr)
        throwException("Can't find wsdl:portType element")

    const auto* descriptionElement = portElement->findFirst("wsdl:documentation");
    if (descriptionElement)
        m_description = descriptionElement->text();

    for (const auto* node: *portElement) {
        const auto* element = dynamic_cast<const xml::Element*>(node);
        if (element != nullptr && element->name() == "wsdl:operation")
            parseOperation(element);
    }
}

String capitalize(const String& name)
{
    Strings parts(lowerCase(name),"_");
    for (auto& part : parts)
        part[0] = (char) toupper(part[0]);
    return parts.join("");
}

string WSParser::strip_namespace(const string& name)
{
    size_t pos = name.find(':');
    if (pos == string::npos)
        return name;
    return name.substr(pos + 1);
}

string WSParser::get_namespace(const string& name)
{
    size_t pos = name.find(':');
    if (pos == string::npos)
        return name;
    return name.substr(0, pos);
}

void WSParser::generateDefinition(const Strings& usedClasses, ostream& serviceDefinition)
{
    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";
    string defineName = "__" + upperCase(serviceClassName) + "__";

    serviceDefinition << "// Web Service " << m_serviceName << " definition" << endl << endl;
    serviceDefinition << "#ifndef " << defineName << endl;
    serviceDefinition << "#define " << defineName << endl << endl;

    serviceDefinition << "#include \"" << "C" + capitalize(m_serviceName) + "WSDL.h\"" << endl;
    serviceDefinition << "#include <sptk5/wsdl/WSRequest.h>" << endl;
    serviceDefinition << "#include <sptk5/net/HttpAuthentication.h>" << endl << endl;

    serviceDefinition << "// This Web Service types" << endl;
    for (auto& usedClass: usedClasses)
        serviceDefinition << "#include \"" << usedClass << ".h\"" << endl;
    serviceDefinition << endl;

    serviceDefinition << "namespace " << m_serviceNamespace << " {" << endl << endl;

    serviceDefinition << "/**" << endl;
    serviceDefinition << " * Base class for service method." << endl;
    serviceDefinition << " *" << endl;
    serviceDefinition << " * Web Service application derives its service class from this class" << endl;
    serviceDefinition << " * by overriding abstract methods" << endl;
    serviceDefinition << " */" << endl;
    serviceDefinition << "class " << serviceClassName << " : public sptk::WSRequest" << endl;
    serviceDefinition << "{" << endl;
    serviceDefinition << "public:" << endl;
    serviceDefinition << "    /**" << endl;
    serviceDefinition << "     * Constructor" << endl;
    serviceDefinition << "     * @param logEngine        Optional log engine for error messages" << endl;
    serviceDefinition << "     */" << endl;
    serviceDefinition << "    explicit " << serviceClassName << "(sptk::LogEngine* logEngine=nullptr);" << endl << endl;

    serviceDefinition << "    /**" << endl;
    serviceDefinition << "     * Destructor" << endl;
    serviceDefinition << "     */" << endl;
    serviceDefinition << "    ~" << serviceClassName << "() override = default;" << endl << endl;

    serviceDefinition << "    // Abstract methods below correspond to WSDL-defined operations. " << endl;
    serviceDefinition << "    // Application must overwrite these methods with processing of corresponding" << endl;
    serviceDefinition << "    // requests, reading data from input and writing data to output structures." << endl;
    for (const auto& itor: m_operations) {
        const WSOperation& operation = itor.second;
        serviceDefinition << endl;
        serviceDefinition << "    /**" << endl;
        serviceDefinition << "     * Web Service " << itor.first << " operation" << endl;
        serviceDefinition << "     *" << endl;
        string documentation = m_documentation[operation.m_input->name()];
        if (!documentation.empty()) {
            Strings documentationRows(documentation, "\n");
            for (const auto& row: documentationRows)
                serviceDefinition << "     * " << trim(row) << endl;
        }
        serviceDefinition << "     * This method is abstract and must be overwritten by derived Web Service implementation class." << endl;
        serviceDefinition << "     * @param input            Operation input data" << endl;
        serviceDefinition << "     * @param output           Operation response data" << endl;
        serviceDefinition << "     */" << endl;
        serviceDefinition
            << "    virtual void " << itor.first
            << "(const " << operation.m_input->className() << "& input, "
            << operation.m_output->className() << "& output, sptk::HttpAuthentication* authentication) = 0;" << endl;
    }
    serviceDefinition << endl;
    serviceDefinition << "    /**" << endl;
    serviceDefinition << "     * @return original WSDL file content" << endl;
    serviceDefinition << "     */" << endl;
    serviceDefinition << "    sptk::String wsdl() const override;" << endl << endl;

    serviceDefinition << "private:" << endl << endl;
    for (const auto& itor: m_operations) {
        string requestName = strip_namespace(itor.second.m_input->name());
        serviceDefinition << "    /**" << endl;
        serviceDefinition << "     * Internal Web Service " << requestName << " processing" << endl;
        serviceDefinition << "     * @param requestNode      Operation input/output XML data" << endl;
        serviceDefinition << "     * @param authentication   Optional HTTP authentication" << endl;
        serviceDefinition << "     * @param requestNameSpace Request SOAP element namespace" << endl;
        serviceDefinition << "     */" << endl;
        serviceDefinition << "    void process_" << requestName << "(sptk::xml::Element* xmlContent, sptk::json::Element* jsonContent, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);" << endl << endl;
    }
    serviceDefinition << "};" << endl << endl;
    serviceDefinition << "typedef std::shared_ptr<" << serviceClassName << "> S" << capitalize(m_serviceName) + "ServiceBase;" << endl << endl;
    serviceDefinition << "}" << endl << endl;
    serviceDefinition << "#endif" << endl;
}

void WSParser::generateImplementation(ostream& serviceImplementation) const
{
    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

    Strings serviceOperations;
    for (const auto& itor: m_operations) {
        String requestName = strip_namespace(itor.second.m_input->name());
        serviceOperations.push_back(requestName);
    }
    String operationNames = serviceOperations.join("|");
    WSMessageIndex serviceOperationsIndex(serviceOperations);

    serviceImplementation << "#include \"" << serviceClassName << ".h\"" << endl;
    serviceImplementation << "#include <sptk5/wsdl/WSParser.h>" << endl;
    serviceImplementation << "#include <sptk5/wsdl/WSMessageIndex.h>" << endl;
    serviceImplementation << "#include <functional>" << endl;
    serviceImplementation << "#include <set>" << endl << endl;

    serviceImplementation << "using namespace std;" << endl;
    serviceImplementation << "using namespace placeholders;" << endl;
    serviceImplementation << "using namespace sptk;" << endl;
    serviceImplementation << "using namespace " << m_serviceNamespace << ";" << endl << endl;

    serviceImplementation << serviceClassName << "::" << serviceClassName << "(LogEngine* logEngine)" << endl;
    serviceImplementation << ": WSRequest(logEngine)" << endl;
    serviceImplementation << "{" << endl;
    serviceImplementation << "    map<String, RequestMethod> requestMethods {" << endl;
    for (const auto& itor: m_operations) {
        string requestName = strip_namespace(itor.second.m_input->name());
        serviceImplementation << "        {\"" << requestName << "\", "
            << "bind(&" << serviceClassName << "::process_" << requestName << ", this, _1, _2, _3, _4)}," << endl;
    }
    serviceImplementation << "    };" << endl;
    serviceImplementation << "    setRequestMethods(move(requestMethods));" << endl;
    serviceImplementation << "}" << endl << endl;

    serviceImplementation << endl <<
        "template <class InputData, class OutputData>\n"
        "void processAnyRequest(xml::Element* requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace, function<void(const InputData& input, OutputData& output, HttpAuthentication* authentication)>& method)\n"
        "{\n"
        "   const String requestName = wsTypeIdToName(typeid(InputData).name());\n"
        "   const String responseName = wsTypeIdToName(typeid(OutputData).name());\n"
        "   String ns(requestNameSpace.getAlias());\n"
        "   InputData inputData((ns + \":\" + requestName).c_str());\n"
        "   OutputData outputData((ns + \":\" + responseName).c_str());\n"
        "   try {\n"
        "      inputData.load(requestNode);\n"
        "   }"
        "   catch (const Exception& e) {\n"
        "      // Can't parse input data\n"
        "      throw HTTPException(400, e.what());\n"
        "   }\n"
        "   auto* soapBody = (xml::Element*) requestNode->parent();\n"
        "   soapBody->clearChildren();\n"
        "   method(inputData, outputData, authentication);\n"
        "   auto* response = new xml::Element(soapBody, (ns + \":\" + responseName).c_str());\n"
        "   response->setAttribute(\"xmlns:\" + ns, requestNameSpace.getLocation());\n"
        "   outputData.unload(response);\n"
        "}\n\n"

        "template <class InputData, class OutputData>\n"
        "void processAnyRequest(json::Element* request, HttpAuthentication* authentication,\n"
        "                       const function<void(const InputData&, OutputData&, HttpAuthentication*)>& method)\n"
        "{\n"
        "   InputData inputData;\n"
        "   OutputData outputData;\n"
        "   try {\n"
        "      inputData.load(request);\n"
        "   }\n"
        "   catch (const Exception& e) {\n"
        "      // Can't parse input data\n"
        "      throw HTTPException(400, e.what());\n"
        "   }\n"
        "   method(inputData, outputData, authentication);\n"
        "   request->clear();\n"
        "   outputData.unload(request);\n"
        "}\n\n";

    for (const auto& itor: m_operations) {
        String operationName = itor.first;
        Strings nameParts(itor.second.m_input->name(), ":");
        String requestName;
        if (nameParts.size() == 1)
            requestName = nameParts[0];
        else
            requestName = nameParts[1];
        const WSOperation& operation = itor.second;
        serviceImplementation << endl;
        serviceImplementation << "void " << serviceClassName << "::process_" << requestName << "(xml::Element* xmlNode, json::Element* jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)" << endl;

        auto inputType = "C" + operation.m_input->name();
        auto outputType = "C" + operation.m_output->name();
        serviceImplementation << "{\n"
                              << "  function<void(const "  << inputType << "&, " << outputType << "&, HttpAuthentication*)>"
                              << " method = bind(&" + serviceClassName + "::" << operationName << ", this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);\n"
                              << "  if (xmlNode)\n"
                              << "     processAnyRequest<" << inputType << "," << outputType << ">(xmlNode, authentication, requestNameSpace, method);\n"
                              << "  else\n"
                              << "     processAnyRequest<" << inputType << "," << outputType << ">(jsonNode, authentication, method);\n"
                              << "}\n";
    }
    serviceImplementation << endl;
    serviceImplementation << "String " << serviceClassName << "::wsdl() const" << endl;
    serviceImplementation << "{" << endl;
    serviceImplementation << "    stringstream output;" << endl;
    serviceImplementation << "    for (auto& row: " << m_serviceName << "_wsdl)" << endl;
    serviceImplementation << "        output << row << endl;" << endl;
    serviceImplementation << "    return output.str();" << endl;
    serviceImplementation << "}" << endl;
}

void WSParser::generate(const String& sourceDirectory, const String& headerFile,
                        const OpenApiGenerator::Options& options,
                        bool verbose, const String& serviceNamespace)
{
    if (!serviceNamespace.empty())
        m_serviceNamespace = serviceNamespace;

    Buffer externalHeader;
    if (!headerFile.empty())
        externalHeader.loadFromFile(headerFile);
    else
        externalHeader.set("");

    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";
    if (verbose)
        COUT("Creating service class " << serviceClassName)

    stringstream cmakeLists;
    cmakeLists << "# The following list of files is generated automatically." << endl;
    cmakeLists << "# Please don't edit it, or your changes may be overwritten." << endl << endl;
    cmakeLists << "SET (" << m_serviceName << "_files" << endl;
    cmakeLists << "  " << sourceDirectory << "/" << serviceClassName << ".cpp "
                       << sourceDirectory << "/" << serviceClassName << ".h" << endl;

    String wsdlFileName = "C" + capitalize(m_serviceName) + "WSDL";
    cmakeLists << "  " << sourceDirectory << "/" << wsdlFileName << ".cpp "
                       << sourceDirectory << "/" << wsdlFileName << ".h" << endl;

    Strings usedClasses;
    for (auto& itor: m_complexTypeIndex.complexTypes()) {
        SWSParserComplexType complexType = itor.second;
        SourceModule sourceModule("C" + complexType->name(), sourceDirectory);
        sourceModule.open();
        complexType->generate(sourceModule.header(), sourceModule.source(), externalHeader.c_str(), m_serviceNamespace);
        usedClasses.push_back("C" + complexType->name());
        cmakeLists << "  " << sourceDirectory << "/C" << complexType->name() << ".cpp "
                           << sourceDirectory << "/C" << complexType->name() << ".h" << endl;
    }

    // Generate Service class definition
    SourceModule serviceModule(serviceClassName, sourceDirectory);
    serviceModule.open();

    if (!externalHeader.empty()) {
        serviceModule.header() << externalHeader.c_str() << endl;
        serviceModule.source() << externalHeader.c_str() << endl;
    }

    generateDefinition(usedClasses, serviceModule.header());
    generateImplementation(serviceModule.source());

    cmakeLists << ")" << endl;

    replaceFile(m_serviceName + ".inc", cmakeLists);

    OpenApiGenerator openApiGenerator(m_serviceName, m_description, "1.0.0", {m_location}, options);
    auto openApiFileName = options.openApiFile;
    if (openApiFileName.empty())
        openApiFileName = m_wsdlFile.replace("\\.wsdl$", "") + ".json";

    if (verbose)
        COUT("Creating OpenAPI file " << openApiFileName)

    ofstream openApiFile(openApiFileName);
    openApiGenerator.generate(openApiFile, m_operations, m_complexTypeIndex.complexTypes(), m_documentation);
    openApiFile.close();
}

void WSParser::generateWsdlCxx(const String& sourceDirectory, const String& headerFile, const String& _wsdlFileName) const
{
    Strings wsdl;
    wsdl.loadFromFile(_wsdlFileName);

    Buffer externalHeader("// Auto-generated by wsdl2cxx\n");
    if (!headerFile.empty())
        externalHeader.loadFromFile(headerFile);

    String baseFileName = "C" + capitalize(m_serviceName) + "WSDL";
    String wsdlFileName = sourceDirectory + "/" + baseFileName;

    stringstream wsdlHeader;
    wsdlHeader << externalHeader.c_str() << endl;
    wsdlHeader << "#ifndef __" << m_serviceName.toUpperCase() << "_WSDL__" << endl;
    wsdlHeader << "#define __" << m_serviceName.toUpperCase() << "_WSDL__" << endl;
    wsdlHeader << endl << "#include <sptk5/Strings.h>" << endl;
    wsdlHeader << endl << "extern const sptk::Strings " << m_serviceName << "_wsdl;" << endl << endl;
    wsdlHeader << "#endif" << endl;

    stringstream wsdlCxx;
    wsdlCxx << externalHeader.c_str() << endl;
    wsdlCxx << "#include \"" << baseFileName << ".h\"" << endl << endl;
    wsdlCxx << "const sptk::Strings " << m_serviceName << "_wsdl {" << endl;

    for (auto row = wsdl.begin(); row != wsdl.end(); ++row) {
        wsdlCxx << "    R\"(" << *row << ")\"";
        if (row + 1 != wsdl.end())
            wsdlCxx << ",";
        wsdlCxx << endl;
    }

    wsdlCxx << "};" << endl;

    replaceFile(wsdlFileName + ".h", wsdlHeader);
    replaceFile(wsdlFileName + ".cpp", wsdlCxx);
}

const String& WSParser::description() const
{
    return m_description;
}

