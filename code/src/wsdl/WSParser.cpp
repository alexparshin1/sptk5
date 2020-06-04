/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSParser.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <iomanip>
#include <sptk5/cutils>
#include <sptk5/wsdl/SourceModule.h>
#include <sptk5/wsdl/WSParser.h>
#include <sptk5/wsdl/WSMessageIndex.h>
#include <src/wsdl/openapi/OpenApiGenerator.h>

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

void WSParser::parseSimpleType(const xml::Element* simpleTypeElement)
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

void WSParser::parseOperation(xml::Element* operationNode)
{
    xml::NodeVector messageNodes;
    operationNode->document()->select(messageNodes, "//wsdl:message");

    map<String, String> messageToElementMap;
    for (auto* node: messageNodes) {
        if (node->type() != xml::Node::DOM_ELEMENT)
            throw Exception("The node " + node->name() + " is not an XML element");
        auto* message = dynamic_cast<xml::Element*>(node);
        xml::Node* part = message->findFirst("wsdl:part");
        String messageName = (String) message->getAttribute("name");
        String elementName = strip_namespace((String) part->getAttribute("element"));
        messageToElementMap[messageName] = elementName;
        xml::Node* documentationNode = part->findFirst("wsdl:documentation");
        if (documentationNode != nullptr)
            m_documentation[elementName] = documentationNode->text();
    }

    WSOperation operation = {};
    bool found = false;
    for (auto* node: *operationNode) {
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

    for (auto* node: simpleTypeNodes) {
        auto* element = dynamic_cast<const xml::Element*>(node);
        if (element != nullptr && element->name() == "xsd:simpleType")
            parseSimpleType(element);
    }

    xml::NodeVector complexTypeNodes;
    schemaElement->select(complexTypeNodes, "//xsd:complexType");

    for (auto* node: complexTypeNodes) {
        auto* element = dynamic_cast<const xml::Element*>(node);
        if (element != nullptr && element->name() == "xsd:complexType")
            parseComplexType(element);
    }

    for (auto* node: *schemaElement) {
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

    auto* service = (xml::Element*) wsdlXML.findFirst("wsdl:service");
    m_serviceName = (String) service->getAttribute("name");

    auto* address = service->findFirst("soap:address");
    if (address)
        m_location = (String) address->getAttribute("location");

    auto* schemaElement = dynamic_cast<xml::Element*>(wsdlXML.findFirst("xsd:schema"));
    if (schemaElement == nullptr)
        throwException("Can't find xsd:schema element")
    parseSchema(schemaElement);

    auto* portElement = dynamic_cast<xml::Element*>(wsdlXML.findFirst("wsdl:portType"));
    if (portElement == nullptr)
        throwException("Can't find wsdl:portType element")

    auto* descriptionElement = portElement->findFirst("wsdl:documentation");
    if (descriptionElement)
        m_description = descriptionElement->text();

    for (auto* node: *portElement) {
        auto* element = dynamic_cast<xml::Element*>(node);
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

    serviceDefinition << "/**" << endl;
    serviceDefinition << " * Base class for service method." << endl;
    serviceDefinition << " *" << endl;
    serviceDefinition << " * Web Service application derives its service class from this class" << endl;
    serviceDefinition << " * by overriding abstract methods" << endl;
    serviceDefinition << " */" << endl;
    serviceDefinition << "class " << serviceClassName << " : public sptk::WSRequest" << endl;
    serviceDefinition << "{" << endl;
    serviceDefinition << "    sptk::LogEngine*  m_logEngine;    ///< Optional logger, or nullptr" << endl;
    for (auto& itor: m_operations) {
        string requestName = strip_namespace(itor.second.m_input->name());
        serviceDefinition << "    /**" << endl;
        serviceDefinition << "     * Internal Web Service " << requestName << " processing" << endl;
        serviceDefinition << "     * @param requestNode      Operation input/output XML data" << endl;
        serviceDefinition << "     * @param authentication   Optional HTTP authentication" << endl;
        serviceDefinition << "     * @param requestNameSpace Request SOAP element namespace" << endl;
        serviceDefinition << "     */" << endl;
        serviceDefinition << "    void process_" << requestName << "(sptk::xml::Element* xmlContent, sptk::json::Element* jsonContent, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);" << endl << endl;
    }
    serviceDefinition << "protected:" << endl;
    serviceDefinition << "    /**" << endl;
    serviceDefinition << "     * Internal request processor" << endl;
    serviceDefinition << "     *" << endl;
    serviceDefinition << "     * Receive incoming requests, and return application response." << endl;
    serviceDefinition << "     * @param requestName      Incoming request name" << endl;
    serviceDefinition << "     * @param xmlContent       Incoming and outgoing XML data" << endl;
    serviceDefinition << "     * @param jsonContent      Incoming and outgoing JSON data" << endl;
    serviceDefinition << "     * @param requestNameSpace Request SOAP element namespace" << endl;
    serviceDefinition << "     */" << endl;
    serviceDefinition << "    void requestBroker(const sptk::String& requestName, sptk::xml::Element* xmlContent, sptk::json::Element* jsonContent, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace) override;" << endl << endl;
    serviceDefinition << "public:" << endl;
    serviceDefinition << "    /**" << endl;
    serviceDefinition << "     * Constructor" << endl;
    serviceDefinition << "     * @param logEngine        Optional log engine for error messages" << endl;
    serviceDefinition << "     */" << endl;
    serviceDefinition << "    explicit " << serviceClassName << "(sptk::LogEngine* logEngine=nullptr)" << endl;
    serviceDefinition << "     : m_logEngine(logEngine)" << endl;
    serviceDefinition << "     {}" << endl << endl;
    serviceDefinition << "    /**" << endl;
    serviceDefinition << "     * Destructor" << endl;
    serviceDefinition << "     */" << endl;
    serviceDefinition << "    ~" << serviceClassName << "() override = default;" << endl << endl;
    serviceDefinition << "    // Abstract methods below correspond to WSDL-defined operations. " << endl;
    serviceDefinition << "    // Application must overwrite these methods with processing of corresponding" << endl;
    serviceDefinition << "    // requests, reading data from input and writing data to output structures." << endl;
    for (auto& itor: m_operations) {
        WSOperation& operation = itor.second;
        serviceDefinition << endl;
        serviceDefinition << "    /**" << endl;
        serviceDefinition << "     * Web Service " << itor.first << " operation" << endl;
        serviceDefinition << "     *" << endl;
        string documentation = m_documentation[operation.m_input->name()];
        if (!documentation.empty()) {
            Strings documentationRows(documentation, "\n");
            for (auto& row: documentationRows)
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
    serviceDefinition << "    sptk::String wsdl() const override;" << endl;
    serviceDefinition << "};" << endl << endl;
    serviceDefinition << "#endif" << endl;
}

void WSParser::generateImplementation(ostream& serviceImplementation)
{
    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

    Strings serviceOperations;
    for (auto& itor: m_operations) {
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
    serviceImplementation << "using namespace sptk;" << endl << endl;

    serviceImplementation << "void " << serviceClassName << "::requestBroker(const String& requestName, xml::Element* xmlContent, json::Element* jsonContent, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)" << endl;
    serviceImplementation << "{" << endl;
    serviceImplementation << "    static const WSMessageIndex messageNames(Strings(\"" << operationNames << "\", \"|\"));" << endl << endl;
    serviceImplementation << "    int messageIndex = messageNames.indexOf(requestName);" << endl;
    serviceImplementation << "    try {" << endl;
    serviceImplementation << "        switch (messageIndex) {" << endl;
    for (auto& itor: m_operations) {
        string requestName = strip_namespace(itor.second.m_input->name());
        int messageIndex = serviceOperationsIndex.indexOf(requestName);
        serviceImplementation << "        case " << messageIndex << ":" << endl;
        serviceImplementation << "            process_" << requestName << "(xmlContent, jsonContent, authentication, requestNameSpace);" << endl;
        serviceImplementation << "            break;" << endl;
    }
    serviceImplementation << "        default:" << endl;
    serviceImplementation << "            throw SOAPException(\"Request '\" + requestName + \"' is not defined in this service\");" << endl;
    serviceImplementation << "        }" << endl;
    serviceImplementation << "    }" << endl;
    serviceImplementation << "    catch (const SOAPException& e) {" << endl;
    serviceImplementation << "        if (xmlContent) {" << endl;
    serviceImplementation << "            auto* soapBody = (xml::Element*) xmlContent->parent();" << endl;
    serviceImplementation << "            soapBody->clearChildren();" << endl;
    serviceImplementation << "            String soap_namespace = WSParser::get_namespace(soapBody->name());" << endl;
    serviceImplementation << "            if (!soap_namespace.empty())" << endl;
    serviceImplementation << "                soap_namespace += \":\";" << endl;
    serviceImplementation << "            auto* faultNode = new xml::Element(soapBody, (soap_namespace + \"Fault\").c_str());" << endl;
    serviceImplementation << "            auto* faultCodeNode = new xml::Element(faultNode, \"faultcode\");" << endl;
    serviceImplementation << "            faultCodeNode->text(soap_namespace + \"Client\");" << endl;
    serviceImplementation << "            auto* faultStringNode = new xml::Element(faultNode, \"faultstring\");" << endl;
    serviceImplementation << "            faultStringNode->text(e.what());" << endl;
    serviceImplementation << "            new xml::Element(faultNode, \"detail\");" << endl;
    serviceImplementation << "        }" << endl;
    serviceImplementation << "        else throw;" << endl;
    serviceImplementation << "    }" << endl;
    serviceImplementation << "    catch (const Exception& e) {" << endl;
    serviceImplementation << "        if (m_logEngine != nullptr) {" << endl;
    serviceImplementation << "            Logger logger(*m_logEngine);" << endl;
    serviceImplementation << "            logger.error(String(\"WS request error: \") + e.what());" << endl;
    serviceImplementation << "        }" << endl;
    serviceImplementation << "    }" << endl;
    serviceImplementation << "}" << endl;

    serviceImplementation << endl <<
        "template <class InputData, class OutputData>\n"
        "void processAnyRequest(xml::Element* requestNode, const String& requestName, const String& responseName, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace, function<void(const InputData& input, OutputData& output, HttpAuthentication* authentication)>& method)\n"
        "{\n"
        "   String ns(requestNameSpace.getAlias());\n"
        "   InputData inputData((ns + \":\" + requestName).c_str());\n"
        "   OutputData outputData((ns + \":\" + responseName).c_str());\n"
        "   inputData.load(requestNode);\n"
        "   auto* soapBody = (xml::Element*) requestNode->parent();\n"
        "   soapBody->clearChildren();\n"
        "   method(inputData, outputData, authentication);\n"
        "   auto* response = new xml::Element(soapBody, (ns + \":\" + responseName).c_str());\n"
        "   response->setAttribute(\"xmlns:\" + ns, requestNameSpace.getLocation());\n"
        "   outputData.unload(response);\n"
        "}\n\n"

        "template <class InputData, class OutputData>\n"
        "void processAnyRequest(json::Element* request, HttpAuthentication* authentication,\n"
        "                       function<void(const InputData&, OutputData&, HttpAuthentication*)>& method)\n"
        "{\n"
        "   InputData inputData;\n"
        "   OutputData outputData;\n"
        "   inputData.load(request);\n"
        "   method(inputData, outputData, authentication);\n"
        "   request->clear();\n"
        "   outputData.unload(request);\n"
        "}\n\n";

    for (auto& itor: m_operations) {
        String operationName = itor.first;
        Strings nameParts(itor.second.m_input->name(), ":");
        String requestName;
        if (nameParts.size() == 1)
            requestName = nameParts[0];
        else
            requestName = nameParts[1];
        WSOperation& operation = itor.second;
        serviceImplementation << endl;
        serviceImplementation << "void " << serviceClassName << "::process_" << requestName << "(xml::Element* xmlNode, json::Element* jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)" << endl;

        auto inputType = "C" + operation.m_input->name();
        auto outputType = "C" + operation.m_output->name();
        serviceImplementation << "{\n"
                              << "  function<void(const "  << inputType << "&, " << outputType << "&, HttpAuthentication*)>"
                              << " method = bind(&" + serviceClassName + "::" << operationName << ", this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);\n"
                              << "  if (xmlNode)\n"
                              << "     processAnyRequest<" << inputType << "," << outputType << ">(xmlNode, \"" + operation.m_input->name() + "\", \"" + operation.m_output->name() + "\", authentication, requestNameSpace, method);\n"
                              << "  else\n"
                              << "     processAnyRequest<" << inputType << "," << outputType << ">(jsonNode, authentication, method);\n"
                              << "}\n";
    }
    serviceImplementation << endl;
    serviceImplementation << "    String " << serviceClassName << "::wsdl() const" << endl;
    serviceImplementation << "    {" << endl;
    serviceImplementation << "        stringstream output;" << endl;
    serviceImplementation << "        for (int i = 0; " << m_serviceName << "_wsdl[i] != nullptr; i++)" << endl;
    serviceImplementation << "            output << " << m_serviceName << "_wsdl[i] << endl;" << endl;
    serviceImplementation << "        return output.str();" << endl;
    serviceImplementation << "    }" << endl;
}

void WSParser::generate(const String& sourceDirectory, const String& headerFile)
{
    Buffer externalHeader;
    if (!headerFile.empty())
        externalHeader.loadFromFile(headerFile);
    else
        externalHeader.set("");

    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

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
        SourceModule module("C" + complexType->name(), sourceDirectory);
        module.open();
        complexType->generate(module.header(), module.source(), externalHeader.c_str());
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

    OpenApiGenerator openApiGenerator(m_serviceName, m_description, "1.0.0", {m_location});
    auto openApiFileName = m_wsdlFile.replace(".wsdl", ".json");
    ofstream openApiFile(openApiFileName);
    openApiGenerator.generate(openApiFile, m_operations, m_complexTypeIndex.complexTypes(), m_documentation);
    openApiFile.close();
}

void WSParser::generateWsdlCxx(const String& sourceDirectory, const String& headerFile, const String& _wsdlFileName)
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
    wsdlHeader << endl << "extern const char* " << m_serviceName << "_wsdl[];" << endl << endl;
    wsdlHeader << "#endif" << endl;

    stringstream wsdlCxx;
    wsdlCxx << externalHeader.c_str() << endl;
    wsdlCxx << "#include \"" << baseFileName << ".h\"" << endl << endl;
    wsdlCxx << "const char* " << m_serviceName << "_wsdl[] = {" << endl;

    for (auto& row: wsdl)
        wsdlCxx << "    R\"(" << row << ")\"," << endl;
    wsdlCxx << "    nullptr" << endl;
    wsdlCxx << "};" << endl;

    replaceFile(wsdlFileName + ".h", wsdlHeader);
    replaceFile(wsdlFileName + ".cpp", wsdlCxx);
}

const String& WSParser::description() const
{
    return m_description;
}

