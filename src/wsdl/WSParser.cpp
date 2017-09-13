/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSParser.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

using namespace std;
using namespace sptk;

WSParser::~WSParser()
{
    clear();
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

void WSParser::parseElement(const XMLElement* elementNode)
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

void WSParser::parseComplexType(const XMLElement* complexTypeElement)
{
    string complexTypeName = complexTypeElement->getAttribute("name");
    if (complexTypeName.empty())
        complexTypeName = complexTypeElement->parent()->getAttribute("name").str();

    if (complexTypeName.empty()) {
        const XMLNode* parent = complexTypeElement->parent();
        complexTypeName = parent->getAttribute("name").c_str();
    }

    if (m_complexTypes.find(complexTypeName) != m_complexTypes.end())
        throwException("Duplicate complexType definition: " + complexTypeName);
    WSParserComplexType* complexType = new WSParserComplexType(complexTypeElement, complexTypeName);
    m_complexTypes[complexTypeName] = complexType;
    if (complexTypeName == "HandlerType")
        cout << endl;
    complexType->parse();
}

void WSParser::parseOperation(XMLElement* operationNode)
{
    XMLNodeVector messageNodes;
    operationNode->document()->select(messageNodes, "//wsdl:message");

    map<string, string> messageToElementMap;
    for (auto node: messageNodes) {
        auto message = dynamic_cast<XMLElement*>(node);
        XMLNode* part = message->findFirst("wsdl:part");
        string messageName = message->getAttribute("name").c_str();
        string elementName = strip_namespace(part->getAttribute("element"));
        messageToElementMap[messageName] = elementName;
        XMLNode* documentationNode = part->findFirst("wsdl:documentation");
        if (documentationNode != nullptr)
            m_documentation[elementName] = documentationNode->text();
    }

    WSOperation operation = {};
    bool found = false;
    for (auto node: *operationNode) {
        auto element = dynamic_cast<const XMLElement*>(node);
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

void WSParser::parseSchema(XMLElement* schemaElement)
{
    XMLNodeVector complexTypeNodes;
    schemaElement->select(complexTypeNodes, "//xsd:complexType");

    for (auto node: complexTypeNodes) {
        auto element = dynamic_cast<const XMLElement*>(node);
        if (element != nullptr && element->name() == "xsd:complexType")
            parseComplexType(element);
    }

    for (auto node: *schemaElement) {
        auto element = dynamic_cast<const XMLElement*>(node);
        if (element != nullptr && element->name() == "xsd:element")
            parseElement(element);
    }
}

void WSParser::parse(std::string wsdlFile)
{
    XMLDocument wsdlXML;
    Buffer buffer;
    buffer.loadFromFile(wsdlFile);
    wsdlXML.load(buffer);

    XMLElement* service = (XMLElement*) wsdlXML.findFirst("wsdl:service");
    m_serviceName = service->getAttribute("name").str();

    XMLElement* schemaElement = dynamic_cast<XMLElement*>(wsdlXML.findFirst("xsd:schema"));
    if (schemaElement == nullptr)
        throwException("Can't find xsd:schema element");
    parseSchema(schemaElement);

    XMLElement* portElement = dynamic_cast<XMLElement*>(wsdlXML.findFirst("wsdl:portType"));
    if (portElement == nullptr)
        throwException("Can't find wsdl:portType element");
    for (auto node: *portElement) {
        auto element = dynamic_cast<XMLElement*>(node);
        if (element != nullptr && element->name() == "wsdl:operation")
            parseOperation(element);
    }
}

string capitalize(const string& name)
{
    Strings parts(lowerCase(name),"_");
    for (unsigned i = 0; i < parts.size(); i++) {
        parts[i][0] = (char) toupper(parts[i][0]);
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

    serviceDefinition << "#include <sptk5/wsdl/WSRequest.h>" << endl << endl;
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
        serviceDefinition << "   /// @brief Internal Web Service " << requestName << " processing" << endl;
        serviceDefinition << "   /// @param requestNode sptk::XMLElement*, Operation input/output XML data" << endl;
        serviceDefinition << "   void process_" << requestName << "(sptk::XMLElement* requestNode);" << endl << endl;
    }
    serviceDefinition << "protected:" << endl;
    serviceDefinition << "   /// @brief Internal SOAP body processor" << endl;
    serviceDefinition << "   ///" << endl;
    serviceDefinition << "   /// Receive incoming SOAP body of Web Service requests, and returns" << endl;
    serviceDefinition << "   /// application response." << endl;
    serviceDefinition << "   /// @param requestNode sptk::XMLElement*, Incoming and outgoing SOAP element" << endl;
    serviceDefinition << "   void requestBroker(sptk::XMLElement* requestNode) override;" << endl << endl;
    serviceDefinition << "public:" << endl;
    serviceDefinition << "   /// @brief Constructor" << endl;
    serviceDefinition << "   " << serviceClassName << "() = default;" << endl << endl;
    serviceDefinition << "   /// @brief Destructor" << endl;
    serviceDefinition << "   ~" << serviceClassName << "() override = default;" << endl << endl;
    serviceDefinition << "   // Abstract methods below correspond to WSDL-defined operations. " << endl;
    serviceDefinition << "   // Application must overwrite these methods with processing of corresponding" << endl;
    serviceDefinition << "   // requests, reading data from input and writing data to output structures." << endl;
    for (auto itor: m_operations) {
        WSOperation& operation = itor.second;
        serviceDefinition << endl;
        serviceDefinition << "   /// @brief Web Service " << itor.first << " operation" << endl;
        serviceDefinition << "   ///" << endl;
        string documentation = m_documentation[operation.m_input->name()];
        if (!documentation.empty()) {
            Strings documentationRows(documentation, "\n");
            for (unsigned i = 0; i < documentationRows.size(); i++)
                serviceDefinition << "   /// " << documentationRows[i] << endl;
        }
        serviceDefinition << "   /// This method is abstract and must be overwritten by derived Web Service implementation class." << endl;
        serviceDefinition << "   /// @param input " << operation.m_input->className() << "&, Operation input data" << endl;
        serviceDefinition << "   /// @param output " << operation.m_output->className() << "&, Operation response data" << endl;
        serviceDefinition
            << "   virtual void " << itor.first
            << "(const " << operation.m_input->className() << "& input, "
            << operation.m_output->className() << "& output) = 0;" << endl;
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
    string operationNames = serviceOperations.asString("|");

    serviceImplementation << "#include \"" << serviceClassName << ".h\"" << endl;
    serviceImplementation << "#include <sptk5/wsdl/WSParser.h>" << endl << endl;

    serviceImplementation << "using namespace std;" << endl;
    serviceImplementation << "using namespace sptk;" << endl << endl;

    serviceImplementation << "void " << serviceClassName << "::requestBroker(XMLElement* requestNode)" << endl;
    serviceImplementation << "{" << endl;
    serviceImplementation << "   static const Strings messageNames(\"" << operationNames << "\", \"|\");" << endl << endl;
    serviceImplementation << "   string requestName = WSParser::strip_namespace(requestNode->name());" << endl;
    serviceImplementation << "   int messageIndex = messageNames.indexOf(requestName);" << endl;
    serviceImplementation << "   try {" << endl;
    serviceImplementation << "      switch (messageIndex) {" << endl;
    for (auto itor: m_operations) {
        string requestName = strip_namespace(itor.second.m_input->name());
        int messageIndex = serviceOperations.indexOf(requestName);
        serviceImplementation << "      case " << messageIndex << ":" << endl;
        serviceImplementation << "         process_" << requestName << "(requestNode);" << endl;
        serviceImplementation << "         break;" << endl;
    }
    serviceImplementation << "      default:" << endl;
    serviceImplementation << "         throwSOAPException(\"Request node \'\" + requestNode->name() + \"' is not defined in this service\");" << endl;
    serviceImplementation << "      }" << endl;
    serviceImplementation << "   }" << endl;
    serviceImplementation << "   catch (const SOAPException& e) {" << endl;
    serviceImplementation << "      auto soapBody = (XMLElement*) requestNode->parent();" << endl;
    serviceImplementation << "      soapBody->clearChildren();" << endl;
    serviceImplementation << "      string soap_namespace = WSParser::get_namespace(soapBody->name());" << endl;
    serviceImplementation << "      if (!soap_namespace.empty()) soap_namespace += \":\";" << endl;
    serviceImplementation << "      auto faultNode = new XMLElement(soapBody, (soap_namespace + \"Fault\").c_str());" << endl;
    serviceImplementation << "      auto faultCodeNode = new XMLElement(faultNode, \"faultcode\");" << endl;
    serviceImplementation << "      faultCodeNode->text(soap_namespace + \"Client\");" << endl;
    serviceImplementation << "      auto faultStringNode = new XMLElement(faultNode, \"faultstring\");" << endl;
    serviceImplementation << "      faultStringNode->text(e.what());" << endl;
    serviceImplementation << "      new XMLElement(faultNode, \"detail\");" << endl;
    serviceImplementation << "   }" << endl;
    serviceImplementation << "}" << endl << endl;

    for (auto itor: m_operations) {
        string operationName = itor.first;
        Strings nameParts(itor.second.m_input->name(), ":");
        String requestName;
        if (nameParts.size() == 1)
            requestName = nameParts[0];
        else {
            // string requestNamespace = nameParts[0];
            requestName = nameParts[1];
        }
        WSOperation& operation = itor.second;
        serviceImplementation << "void " << serviceClassName << "::process_" << requestName << "(XMLElement* requestNode)" << endl;
        serviceImplementation << "{" << endl;
        serviceImplementation << "   String ns(requestNameSpace().getAlias());" << endl;
        serviceImplementation << "   C" << operation.m_input->name() << " inputData((ns + \":" << operation.m_input->name() << "\").c_str());" << endl;
        serviceImplementation << "   C" << operation.m_output->name() << " outputData((ns + \":" << operation.m_output->name() << "\").c_str());" << endl;
        serviceImplementation << "   inputData.load(requestNode);" << endl;
        serviceImplementation << "   auto soapBody = (XMLElement*) requestNode->parent();" << endl;
        serviceImplementation << "   soapBody->clearChildren();" << endl;
        serviceImplementation << "   " << operationName << "(inputData,outputData);" << endl;
        serviceImplementation << "   auto response = new XMLElement(soapBody, (ns + \":" << operation.m_output->name() << "\").c_str());" << endl;
        serviceImplementation << "   response->setAttribute(\"xmlns:\" + ns, requestNameSpace().getLocation());" << endl;
        serviceImplementation << "   outputData.unload(response);" << endl;
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
