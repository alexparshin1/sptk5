/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSParser.cpp  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#include <sptk5/wsdl/CWSParser.h>
#include <sptk5/wsdl/CSourceModule.h>

#include <iomanip>

using namespace std;
using namespace sptk;

CWSParser::CWSParser()
{
}

CWSParser::~CWSParser()
{
    clear();
}

void CWSParser::clear()
{
    for (ComplexTypeMap::iterator itor = m_complexTypes.begin(); itor != m_complexTypes.end(); itor++) {
        CWSParserComplexType* complexType = itor->second;
        if (complexType->refCount())
            complexType->decreaseRefCount();
        else
            delete complexType;
    }
    for (ElementMap::iterator itor = m_elements.begin(); itor != m_elements.end(); itor++)
        delete itor->second;
    m_complexTypes.clear();
    m_elements.clear();
}

void CWSParser::parseElement(const CXmlElement* elementNode) THROWS_EXCEPTIONS
{
    string elementName = elementNode->getAttribute("name");
    string elementType = elementNode->getAttribute("type");

    size_t namespacePos = elementType.find(":");
    if (namespacePos != string::npos)
        elementType = elementType.substr(namespacePos + 1);

    CWSParserComplexType* complexType;
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

void CWSParser::parseComplexType(const CXmlElement* complexTypeElement) THROWS_EXCEPTIONS
{
    string complexTypeName = complexTypeElement->getAttribute("name");

    if (complexTypeName.empty()) {
        const CXmlNode* parent = complexTypeElement->parent();
        complexTypeName = parent->getAttribute("name").c_str();
    }

    if (m_complexTypes.find(complexTypeName) != m_complexTypes.end())
        throwException("Duplicate complexType definition: " + complexTypeName);
    CWSParserComplexType* complexType = new CWSParserComplexType(complexTypeElement, complexTypeName);
    m_complexTypes[complexTypeName] = complexType;
    if (complexTypeName == "HandlerType")
        cout << endl;
    complexType->parse();
}

void CWSParser::parseOperation(CXmlElement* operationNode) THROWS_EXCEPTIONS
{
    CXmlNodeVector messageNodes;
    operationNode->document()->select(messageNodes, "//wsdl:message");

    map<string, string> messageToElementMap;
    for (CXmlNode::const_iterator itor = messageNodes.begin(); itor != messageNodes.end(); itor++) {
        CXmlElement* message = dynamic_cast<CXmlElement*>(*itor);
        CXmlNode* part = message->findFirst("wsdl:part");
        string messageName = message->getAttribute("name").c_str();
        string elementName = strip_namespace(part->getAttribute("element"));
        messageToElementMap[messageName] = elementName;
        CXmlNode* documentationNode = part->findFirst("wsdl:documentation");
        if (documentationNode)
            m_documentation[elementName] = documentationNode->text();
    }

    CWSOperation operation;
    bool found = false;
    for (CXmlElement::const_iterator itor = operationNode->begin(); itor != operationNode->end(); itor++) {
        const CXmlElement* element = dynamic_cast<const CXmlElement*>(*itor);
        string message = element->getAttribute("message");
        size_t pos = message.find(":");
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

void CWSParser::parseSchema(CXmlElement* schemaElement) THROWS_EXCEPTIONS
{
    CXmlNodeVector complexTypeNodes;
    schemaElement->select(complexTypeNodes, "//xsd:complexType");

    for (CXmlNode::const_iterator itor = complexTypeNodes.begin(); itor != complexTypeNodes.end(); itor++) {
        const CXmlElement* element = dynamic_cast<const CXmlElement*>(*itor);
        if (element && element->name() == "xsd:complexType")
            parseComplexType(element);
    }

    //for (ComplexTypeMap::iterator itor =  m_complexTypes.begin(); itor !=  m_complexTypes.end(); itor++)
    //    cout << setw(20) << itor->first + ": " + itor->second->className() << endl;

    for (CXmlElement::const_iterator itor = schemaElement->begin(); itor != schemaElement->end(); itor++) {
        const CXmlElement* element = dynamic_cast<const CXmlElement*>(*itor);
        if (element && element->name() == "xsd:element")
            parseElement(element);
    }
}

void CWSParser::parse(std::string wsdlFile) THROWS_EXCEPTIONS
{
    CXmlDoc wsdlXML;
    CBuffer buffer;
    buffer.loadFromFile(wsdlFile);
    wsdlXML.load(buffer);

    CXmlElement* service = (CXmlElement*) wsdlXML.findFirst("wsdl:service");
    m_serviceName = service->getAttribute("name").str();

    CXmlElement* schemaElement = dynamic_cast<CXmlElement*>(wsdlXML.findFirst("xsd:schema"));
    if (!schemaElement)
        throwException("Can't find xsd:schema element");
    parseSchema(schemaElement);

    CXmlElement* portElement = dynamic_cast<CXmlElement*>(wsdlXML.findFirst("wsdl:portType"));
    if (!portElement)
        throwException("Can't find wsdl:portType element");
    for (CXmlElement::const_iterator itor = portElement->begin(); itor != portElement->end(); itor++) {
        CXmlElement* element = dynamic_cast<CXmlElement*>(*itor);
        if (element && element->name() == "wsdl:operation")
            parseOperation(element);
    }
}

string capitalize(string name)
{
    CStrings parts(lowerCase(name),"_");
    for (unsigned i = 0; i < parts.size(); i++) {
        parts[i][0] = (char) toupper(parts[i][0]);
    }
    return parts.asString("");
}

string CWSParser::strip_namespace(const string& name)
{
    size_t pos = name.find(":");
    if (pos == string::npos)
        return name;
    return name.substr(pos + 1);
}

void CWSParser::generateDefinition(const CStrings& usedClasses, ostream& serviceDefinition) THROWS_EXCEPTIONS
{
    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";
    string defname = "__" + upperCase(serviceClassName) + "__";

    serviceDefinition << "// Web Service " << m_serviceName << " definition" << endl << endl;
    serviceDefinition << "#ifndef " << defname << endl;
    serviceDefinition << "#define " << defname << endl << endl;

    serviceDefinition << "#include <sptk5/wsdl/CWSRequest.h>" << endl << endl;
    serviceDefinition << "// This Web Service types" << endl;
    for (CStrings::const_iterator itor = usedClasses.begin(); itor != usedClasses.end(); itor++)
        serviceDefinition << "#include \"" << *itor << ".h\"" << endl;
    serviceDefinition << endl;

    serviceDefinition << "/// @brief Base class for service method." << endl;
    serviceDefinition << "///" << endl;
    serviceDefinition << "/// Web Service application derives its service class from this class" << endl;
    serviceDefinition << "/// by overriding abstract methods" << endl;
    serviceDefinition << "class " << serviceClassName << " : public sptk::CWSRequest" << endl;
    serviceDefinition << "{" << endl;
    for (OperationMap::iterator itor = m_operations.begin(); itor != m_operations.end(); itor++) {
        string requestName = strip_namespace(itor->second.m_input->name());
        serviceDefinition << "   /// @brief Internal Web Service " << requestName << " processing" << endl;
        serviceDefinition << "   /// @param requestNode sptk::CXmlElement*, Operation input/output XML data" << endl;
        serviceDefinition << "   void process_" << requestName << "(sptk::CXmlElement* requestNode) THROWS_EXCEPTIONS;" << endl << endl;
    }
    serviceDefinition << "protected:" << endl;
    serviceDefinition << "   /// @brief Internal SOAP body processor" << endl;
    serviceDefinition << "   ///" << endl;
    serviceDefinition << "   /// Receive incoming SOAP body of Web Service requests, and returns" << endl;
    serviceDefinition << "   /// application response." << endl;
    serviceDefinition << "   /// @param requestNode sptk::CXmlElement*, Incoming and outgoing SOAP element" << endl;
    serviceDefinition << "   virtual void requestBroker(sptk::CXmlElement* requestNode) THROWS_EXCEPTIONS;" << endl << endl;
    serviceDefinition << "public:" << endl;
    serviceDefinition << "   /// @brief Constructor" << endl;
    serviceDefinition << "   " << serviceClassName << "() {}" << endl << endl;
    serviceDefinition << "   /// @brief Destructor" << endl;
    serviceDefinition << "   ~" << serviceClassName << "() {}" << endl << endl;
    serviceDefinition << "   // Abstract methods below correspond to WSDL-defined operations. " << endl;
    serviceDefinition << "   // Application must overwrite these methods with processing of corresponding" << endl;
    serviceDefinition << "   // requests, reading data from input and writing data to output structures." << endl;
    for (OperationMap::iterator itor = m_operations.begin(); itor != m_operations.end(); itor++) {
        CWSOperation& operation = itor->second;
        serviceDefinition << endl;
        serviceDefinition << "   /// @brief Web Service " << itor->first << " operation" << endl;
        serviceDefinition << "   ///" << endl;
        string documentation = m_documentation[operation.m_input->name()];
        if (!documentation.empty()) {
            CStrings documentationRows(documentation, "\n");
            for (unsigned i = 0; i < documentationRows.size(); i++)
                serviceDefinition << "   /// " << documentationRows[i] << endl;
        }
        serviceDefinition << "   /// This method is abstract and must be overwritten by derived Web Service implementation class." << endl;
        serviceDefinition << "   /// @param input " << operation.m_input->className() << "&, Operation input data" << endl;
        serviceDefinition << "   /// @param output " << operation.m_output->className() << "&, Operation response data" << endl;
        serviceDefinition
            << "   virtual void " << itor->first
            << "(const " << operation.m_input->className() << "& input, "
            << operation.m_output->className() << "& output) THROWS_EXCEPTIONS = 0;" << endl;
    }
    serviceDefinition << "};" << endl << endl;
    serviceDefinition << "#endif" << endl;
}

void CWSParser::generateImplementation(ostream& serviceImplementation) THROWS_EXCEPTIONS
{
    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

    CStrings serviceOperations;
    for (OperationMap::iterator itor = m_operations.begin(); itor != m_operations.end(); itor++) {
        string requestName = strip_namespace(itor->second.m_input->name());
        serviceOperations.push_back(requestName);
    }
    string operationNames = serviceOperations.asString("|");

    serviceImplementation << "#include \"" << serviceClassName << ".h\"" << endl;
    serviceImplementation << "#include <sptk5/wsdl/CWSParser.h>" << endl << endl;

    serviceImplementation << "using namespace std;" << endl;
    serviceImplementation << "using namespace sptk;" << endl << endl;

    serviceImplementation << "void " << serviceClassName << "::requestBroker(CXmlElement* requestNode) THROWS_EXCEPTIONS" << endl;
    serviceImplementation << "{" << endl;
    serviceImplementation << "   static const CStrings messageNames(\"" << operationNames << "\", \"|\");" << endl << endl;
    serviceImplementation << "   string requestName = CWSParser::strip_namespace(requestNode->name());" << endl;
    serviceImplementation << "   int messageIndex = messageNames.indexOf(requestName);" << endl;
    serviceImplementation << "   switch (messageIndex) {" << endl;
    for (OperationMap::iterator itor = m_operations.begin(); itor != m_operations.end(); itor++) {
        string requestName = strip_namespace(itor->second.m_input->name());
        int messageIndex = serviceOperations.indexOf(requestName);
        serviceImplementation << "   case " << messageIndex << ":" << endl;
        serviceImplementation << "      process_" << requestName << "(requestNode);" << endl;
        serviceImplementation << "      break;" << endl;
    }
    serviceImplementation << "   default:" << endl;
    serviceImplementation << "      throwException(\"Request node \'\" + requestNode->name() + \"' is not defined in this service\");" << endl;
    serviceImplementation << "   }" << endl;
    serviceImplementation << "}" << endl << endl;

    for (OperationMap::iterator itor = m_operations.begin(); itor != m_operations.end(); itor++) {
        string operationName = itor->first;
        CStrings nameParts(itor->second.m_input->name(), ":");
        string requestNamespace, requestName;
        if (nameParts.size() == 1)
            requestName = nameParts[0];
        else {
            requestNamespace = nameParts[0];
            requestName = nameParts[1];
        }
        CWSOperation& operation = itor->second;
        serviceImplementation << "void " << serviceClassName << "::process_" << requestName << "(CXmlElement* requestNode) THROWS_EXCEPTIONS" << endl;
        serviceImplementation << "{" << endl;
        serviceImplementation << "   C" << operation.m_input->name() << " inputData((m_namespace + \"" << operation.m_input->name() << "\").c_str());" << endl;
        serviceImplementation << "   C" << operation.m_output->name() << " outputData((m_namespace + \"" << operation.m_output->name() << "\").c_str());" << endl;
        serviceImplementation << "   inputData.load(requestNode);" << endl;
        serviceImplementation << "   CXmlElement* soapBody = (CXmlElement*) requestNode->parent();" << endl;
        serviceImplementation << "   soapBody->clearChildren();" << endl;
        serviceImplementation << "   " << operationName << "(inputData,outputData);" << endl;
        serviceImplementation << "   CXmlElement* response = new CXmlElement(soapBody, (m_namespace + \"" << operation.m_output->name() << "\").c_str());" << endl;
        serviceImplementation << "   outputData.unload(response);" << endl;
        serviceImplementation << "}" << endl << endl;
    }
}

/// @brief Stores parsed classes to files in source directory
/// @param sourceDirectory std::string, Directory to store output classes
void CWSParser::generate(std::string sourceDirectory) THROWS_EXCEPTIONS
{
    CStrings usedClasses;
    for (ComplexTypeMap::iterator itor = m_complexTypes.begin(); itor !=  m_complexTypes.end(); itor++) {
        CWSParserComplexType* complexType = itor->second;
        string name = itor->first;
        CSourceModule module("C" + complexType->name(), sourceDirectory);
        module.open();
        complexType->generate(module.header(), module.source());
        usedClasses.push_back("C" + complexType->name());
    }

    // Generate Service class definition
    string serviceClassName = "C" + capitalize(m_serviceName) + "ServiceBase";

    CSourceModule serviceModule(serviceClassName, sourceDirectory);
    serviceModule.open();

    generateDefinition(usedClasses, serviceModule.header());
    generateImplementation(serviceModule.source());
}
