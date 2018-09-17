/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSParser.cpp - description                             ║
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

#include <iomanip>
#include <sptk5/wsdl/SourceModule.h>
#include <sptk5/wsdl/WSParser.h>
#include <sptk5/wsdl/WSMessageIndex.h>

using namespace std;
using namespace sptk;

WSParser::~WSParser()
{
    try {
        clear();
    }
    catch (...) {}
}

void WSParser::clear()
{
    for (auto itor: m_complexTypes) {
        WSParserComplexType* complexType = itor.second;
        if (complexType->refCount() != 0)
            complexType->decreaseRefCount();
        else
            delete complexType;
    }
    for (auto itor: m_elements)
        delete itor.second;
    m_complexTypes.clear();
    m_elements.clear();
}

void WSParser::parseElement(const xml::Element* elementNode)
{
    string elementName = elementNode->getAttribute("name");
    string elementType = elementNode->getAttribute("type");

    size_t namespacePos = elementType.find(':');
    if (namespacePos != string::npos)
        elementType = elementType.substr(namespacePos + 1);

    WSParserComplexType* complexType;
    if (!elementType.empty()) {
        complexType = m_complexTypes[elementType];
        complexType->increaseRefCount();
    } else {
        // Element defines type inline
        complexType = m_complexTypes[elementName];
        complexType->increaseRefCount();
    }
    m_complexTypes[elementName] = complexType;
    m_elements[elementName] = complexType;
}

void WSParser::parseComplexType(const xml::Element* complexTypeElement)
{
    String complexTypeName = complexTypeElement->getAttribute("name");
    if (complexTypeName.empty())
        complexTypeName = complexTypeElement->parent()->getAttribute("name").str();

    if (complexTypeName.empty()) {
        const xml::Node* parent = complexTypeElement->parent();
        complexTypeName = parent->getAttribute("name").c_str();
    }

    if (m_complexTypes.find(complexTypeName) != m_complexTypes.end())
        throwException("Duplicate complexType definition: " << complexTypeName);

    WSParserComplexType* complexType = new WSParserComplexType(complexTypeElement, complexTypeName);
    m_complexTypes[complexTypeName] = complexType;
    if (complexTypeName == "HandlerType")
        cout << endl;
    complexType->parse();
}

void WSParser::parseOperation(xml::Element* operationNode)
{
    xml::NodeVector messageNodes;
    operationNode->document()->select(messageNodes, "//wsdl:message");

    map<string, string> messageToElementMap;
    for (auto node: messageNodes) {
        auto message = dynamic_cast<xml::Element*>(node);
        if (message == nullptr)
            throw Exception("The node " + node->name() + " is not an XML element");
        xml::Node* part = message->findFirst("wsdl:part");
        string messageName = message->getAttribute("name").c_str();
        string elementName = strip_namespace(part->getAttribute("element"));
        messageToElementMap[messageName] = elementName;
        xml::Node* documentationNode = part->findFirst("wsdl:documentation");
        if (documentationNode != nullptr)
            m_documentation[elementName] = documentationNode->text();
    }

    WSOperation operation = {};
    bool found = false;
    for (auto node: *operationNode) {
        auto element = dynamic_cast<const xml::Element*>(node);
        if (element == nullptr)
            throw Exception("The node " + node->name() + " is not an XML element");
        string message = element->getAttribute("message");
        size_t pos = message.find(':');
        if (pos != string::npos)
            message = message.substr(pos+1);
        string elementName = messageToElementMap[message];
        if (element->name() == "wsdl:input") {
            operation.m_input = m_complexTypes[elementName];
            found = true;
            continue;
        }
        if (element->name() == "wsdl:output") {
            operation.m_output = m_complexTypes[message];
            found = true;
            continue;
        }
    }

    if (found) {
        string operationName = operationNode->getAttribute("name");
        m_operations[operationName] = operation;
    }
}

void WSParser::parseSchema(xml::Element* schemaElement)
{
    xml::NodeVector complexTypeNodes;
    schemaElement->select(complexTypeNodes, "//xsd:complexType");

    for (auto node: complexTypeNodes) {
        auto element = dynamic_cast<const xml::Element*>(node);
        if (element != nullptr && element->name() == "xsd:complexType")
            parseComplexType(element);
    }

    for (auto node: *schemaElement) {
        auto element = dynamic_cast<const xml::Element*>(node);
        if (element != nullptr && element->name() == "xsd:element")
            parseElement(element);
    }
}

void WSParser::parse(std::string wsdlFile)
{
    xml::Document wsdlXML;
    Buffer buffer;
    buffer.loadFromFile(wsdlFile);
    wsdlXML.load(buffer);

    xml::Element* service = (xml::Element*) wsdlXML.findFirst("wsdl:service");
    m_serviceName = service->getAttribute("name").str();

    xml::Element* schemaElement = dynamic_cast<xml::Element*>(wsdlXML.findFirst("xsd:schema"));
    if (schemaElement == nullptr)
        throwException("Can't find xsd:schema element");
    parseSchema(schemaElement);

    xml::Element* portElement = dynamic_cast<xml::Element*>(wsdlXML.findFirst("wsdl:portType"));
    if (portElement == nullptr)
        throwException("Can't find wsdl:portType element");
    for (auto node: *portElement) {
        auto element = dynamic_cast<xml::Element*>(node);
        if (element != nullptr && element->name() == "wsdl:operation")
            parseOperation(element);
    }
}

String capitalize(const String& name)
{
    Strings parts(lowerCase(name),"_");
    for (auto& part : parts) {
        part[0] = (char) toupper(part[0]);
    }
    return parts.asString("");
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

    serviceDefinition << "#include <sptk5/wsdl/WSRequest.h>" << endl;
    serviceDefinition << "#include <sptk5/net/HttpAuthentication.h>" << endl << endl;
    serviceDefinition << "// This Web Service types" << endl;
    for (auto& usedClass: usedClasses)
        serviceDefinition << "#include \"" << usedClass << ".h\"" << endl;
    serviceDefinition << endl;

    serviceDefinition << "/// @brief Base class for service method." << endl;
    serviceDefinition << "///" << endl;
    serviceDefinition << "/// Web Service application derives its service class from this class" << endl;
    serviceDefinition << "/// by overriding abstract methods" << endl;
    serviceDefinition << "class " << serviceClassName << " : public sptk::WSRequest" << endl;
    serviceDefinition << "{" << endl;
    for (auto itor: m_operations) {
        string requestName = strip_namespace(itor.second.m_input->name());
        serviceDefinition << "    /**" << endl;
        serviceDefinition << "     * Internal Web Service " << requestName << " processing" << endl;
        serviceDefinition << "     * @param requestNode      Operation input/output XML data" << endl;
        serviceDefinition << "     * @param authentication   Optional HTTP authentication" << endl;
        serviceDefinition << "     * @param requestNameSpace Request SOAP element namespace" << endl;
        serviceDefinition << "     */" << endl;
        serviceDefinition << "    void process_" << requestName << "(sptk::xml::Element* requestNode, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);" << endl << endl;
    }
    serviceDefinition << "protected:" << endl;
    serviceDefinition << "    /**" << endl;
    serviceDefinition << "     * Internal SOAP body processor" << endl;
    serviceDefinition << "     *" << endl;
    serviceDefinition << "     * Receive incoming SOAP body of Web Service requests, and returns" << endl;
    serviceDefinition << "     * application response." << endl;
    serviceDefinition << "     * @param requestNode      Incoming and outgoing SOAP element" << endl;
    serviceDefinition << "     * @param requestNameSpace Request SOAP element namespace" << endl;
    serviceDefinition << "     */" << endl;
    serviceDefinition << "    void requestBroker(sptk::xml::Element* requestNode, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace) override;" << endl << endl;
    serviceDefinition << "public:" << endl;
    serviceDefinition << "    /// @brief Constructor" << endl;
    serviceDefinition << "    " << serviceClassName << "() = default;" << endl << endl;
    serviceDefinition << "    /// @brief Destructor" << endl;
    serviceDefinition << "    ~" << serviceClassName << "() override = default;" << endl << endl;
    serviceDefinition << "    // Abstract methods below correspond to WSDL-defined operations. " << endl;
    serviceDefinition << "    // Application must overwrite these methods with processing of corresponding" << endl;
    serviceDefinition << "    // requests, reading data from input and writing data to output structures." << endl;
    for (auto itor: m_operations) {
        WSOperation& operation = itor.second;
        serviceDefinition << endl;
        serviceDefinition << "    /// @brief Web Service " << itor.first << " operation" << endl;
        serviceDefinition << "    ///" << endl;
        string documentation = m_documentation[operation.m_input->name()];
        if (!documentation.empty()) {
            Strings documentationRows(documentation, "\n");
            for (unsigned i = 0; i < documentationRows.size(); i++)
                serviceDefinition << "    /// " << trim(documentationRows[i]) << endl;
        }
        serviceDefinition << "    /// This method is abstract and must be overwritten by derived Web Service implementation class." << endl;
        serviceDefinition << "    /// @param input            Operation input data" << endl;
        serviceDefinition << "    /// @param output           Operation response data" << endl;
        serviceDefinition
            << "    virtual void " << itor.first
            << "(const " << operation.m_input->className() << "& input, "
            << operation.m_output->className() << "& output, sptk::HttpAuthentication* authentication) = 0;" << endl;
    }
    serviceDefinition << "};" << endl << endl;
    serviceDefinition << "#endif" << endl;
}

void WSParser::generateImplementation(ostream& serviceImplementation)
{
    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

    Strings serviceOperations;
    for (auto itor: m_operations) {
        String requestName = strip_namespace(itor.second.m_input->name());
        serviceOperations.push_back(requestName);
    }
    String operationNames = serviceOperations.asString("|");
    WSMessageIndex serviceOperationsIndex(serviceOperations);

    serviceImplementation << "#include \"" << serviceClassName << ".h\"" << endl;
    serviceImplementation << "#include <sptk5/wsdl/WSParser.h>" << endl;
    serviceImplementation << "#include <sptk5/wsdl/WSMessageIndex.h>" << endl;
    serviceImplementation << "#include <set>" << endl << endl;

    serviceImplementation << "using namespace std;" << endl;
    serviceImplementation << "using namespace sptk;" << endl << endl;

    serviceImplementation << "void " << serviceClassName << "::requestBroker(xml::Element* requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)" << endl;
    serviceImplementation << "{" << endl;
    serviceImplementation << "    static const WSMessageIndex messageNames(Strings(\"" << operationNames << "\", \"|\"));" << endl << endl;
    serviceImplementation << "    string requestName = WSParser::strip_namespace(requestNode->name());" << endl;
    serviceImplementation << "    int messageIndex = messageNames.indexOf(requestName);" << endl;
    serviceImplementation << "    try {" << endl;
    serviceImplementation << "        switch (messageIndex) {" << endl;
    for (auto itor: m_operations) {
        string requestName = strip_namespace(itor.second.m_input->name());
        int messageIndex = serviceOperationsIndex.indexOf(requestName);
        serviceImplementation << "        case " << messageIndex << ":" << endl;
        serviceImplementation << "            process_" << requestName << "(requestNode, authentication, requestNameSpace);" << endl;
        serviceImplementation << "            break;" << endl;
    }
    serviceImplementation << "        default:" << endl;
    serviceImplementation << "            throwSOAPException(\"Request node \'\" + requestNode->name() + \"' is not defined in this service\");" << endl;
    serviceImplementation << "        }" << endl;
    serviceImplementation << "    }" << endl;
    serviceImplementation << "    catch (const SOAPException& e) {" << endl;
    serviceImplementation << "        auto soapBody = (xml::Element*) requestNode->parent();" << endl;
    serviceImplementation << "        soapBody->clearChildren();" << endl;
    serviceImplementation << "        string soap_namespace = WSParser::get_namespace(soapBody->name());" << endl;
    serviceImplementation << "        if (!soap_namespace.empty()) soap_namespace += \":\";" << endl;
    serviceImplementation << "        auto faultNode = new xml::Element(soapBody, (soap_namespace + \"Fault\").c_str());" << endl;
    serviceImplementation << "        auto faultCodeNode = new xml::Element(faultNode, \"faultcode\");" << endl;
    serviceImplementation << "        faultCodeNode->text(soap_namespace + \"Client\");" << endl;
    serviceImplementation << "        auto faultStringNode = new xml::Element(faultNode, \"faultstring\");" << endl;
    serviceImplementation << "        faultStringNode->text(e.what());" << endl;
    serviceImplementation << "        new xml::Element(faultNode, \"detail\");" << endl;
    serviceImplementation << "    }" << endl;
    serviceImplementation << "}" << endl << endl;

    for (auto itor: m_operations) {
        String operationName = itor.first;
        Strings nameParts(itor.second.m_input->name(), ":");
        String requestName;
        if (nameParts.size() == 1)
            requestName = nameParts[0];
        else {
            // string requestNamespace = nameParts[0];
            requestName = nameParts[1];
        }
        WSOperation& operation = itor.second;
        serviceImplementation << "void " << serviceClassName << "::process_" << requestName << "(xml::Element* requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)" << endl;
        serviceImplementation << "{" << endl;
        serviceImplementation << "    String ns(requestNameSpace.getAlias());" << endl;
        serviceImplementation << "    C" << operation.m_input->name() << " inputData((ns + \":" << operation.m_input->name() << "\").c_str());" << endl;
        serviceImplementation << "    C" << operation.m_output->name() << " outputData((ns + \":" << operation.m_output->name() << "\").c_str());" << endl;
        serviceImplementation << "    inputData.load(requestNode);" << endl;
        serviceImplementation << "    auto soapBody = (xml::Element*) requestNode->parent();" << endl;
        serviceImplementation << "    soapBody->clearChildren();" << endl;
        serviceImplementation << "    " << operationName << "(inputData, outputData, authentication);" << endl;
        serviceImplementation << "    auto response = new xml::Element(soapBody, (ns + \":" << operation.m_output->name() << "\").c_str());" << endl;
        serviceImplementation << "    response->setAttribute(\"xmlns:\" + ns, requestNameSpace.getLocation());" << endl;
        serviceImplementation << "    outputData.unload(response);" << endl;
        serviceImplementation << "}" << endl << endl;
    }
}

/// @brief Stores parsed classes to files in source directory
/// @param sourceDirectory std::string, Directory to store output classes
void WSParser::generate(std::string sourceDirectory, std::string headerFile)
{
    Buffer externalHeader;
    if (!headerFile.empty())
        externalHeader.loadFromFile(headerFile);

    Strings usedClasses;
    for (auto itor: m_complexTypes) {
        WSParserComplexType* complexType = itor.second;
        SourceModule module("C" + complexType->name(), sourceDirectory);
        module.open();
        complexType->generate(module.header(), module.source(), externalHeader.c_str());
        usedClasses.push_back("C" + complexType->name());
    }

    // Generate Service class definition
    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

    SourceModule serviceModule(serviceClassName, sourceDirectory);
    serviceModule.open();

    if (!externalHeader.empty()) {
        serviceModule.header() << externalHeader.c_str() << endl;
        serviceModule.source() << externalHeader.c_str() << endl;
    }

    generateDefinition(usedClasses, serviceModule.header());
    generateImplementation(serviceModule.source());
}
