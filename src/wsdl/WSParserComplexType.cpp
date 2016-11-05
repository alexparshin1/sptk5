/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSParserComplexType.cpp - description                  ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/wsdl/WSParserComplexType.h>
#include <sptk5/wsdl/WSTypeTranslator.h>
#include <sptk5/RegularExpression.h>
#include <sstream>

using namespace std;
using namespace sptk;

WSParserAttribute::WSParserAttribute(std::string name, std::string typeName)
: m_name(name),
m_wsTypeName(typeName),
m_multiplicity(WSM_OPTIONAL)
{
    m_cxxTypeName = wsTypeTranslator.toCxxType(typeName);
}

WSParserAttribute::WSParserAttribute(const WSParserAttribute& attr)
: m_name(attr.m_name),
m_wsTypeName(attr.m_wsTypeName),
m_cxxTypeName(attr.m_cxxTypeName),
m_multiplicity(attr.m_multiplicity) { }

string WSParserAttribute::generate() const
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%-20s m_%s", m_cxxTypeName.c_str(), m_name.c_str());
    return string(buffer);
}

WSParserComplexType::WSParserComplexType(const XMLElement* complexTypeElement, string name, string typeName)
: m_element(complexTypeElement), m_refcount(0), m_restriction(NULL)
{
    m_name = name.empty() ? complexTypeElement->getAttribute("name").str() : name;
    m_typeName = typeName.empty() ? complexTypeElement->getAttribute("type").str() : typeName;

    if (m_typeName == "" && complexTypeElement->name() == "xsd:element") {
        XMLNode* restrictionElement = complexTypeElement->findFirst("xsd:restriction");
        if (restrictionElement) {
            m_typeName = restrictionElement->getAttribute("base").c_str();
            m_restriction = new WSRestriction(m_typeName, (XMLElement*) restrictionElement->parent());
        }
    }

    if (m_typeName.empty())
        m_typeName = m_name;

    string maxOccurs, minOccurs;
    if (complexTypeElement->hasAttribute("maxOccurs"))
        maxOccurs = (string) complexTypeElement->getAttribute("maxOccurs");
    if (complexTypeElement->hasAttribute("minOccurs"))
        minOccurs = (string) complexTypeElement->getAttribute("minOccurs");

    m_multiplicity = WSM_REQUIRED;

    // Relaxed defaults, in case of incomplete or missing multiplicity
    if (minOccurs.empty())
        minOccurs = "1";
    if (maxOccurs.empty())
        maxOccurs = "1";

    if (minOccurs == "0")
        m_multiplicity = maxOccurs == "1" ? WSM_OPTIONAL : WSM_ZERO_OR_MORE;
    else if (minOccurs == "1")
        m_multiplicity = maxOccurs == "1" ? WSM_REQUIRED : WSM_ONE_OR_MORE;
}

WSParserComplexType::~WSParserComplexType()
{
    if (m_refcount == 0) {
        for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); ++itor)
            delete *itor;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); ++itor)
            delete itor->second;
    }
}

string WSParserComplexType::className() const
{
    string cxxType = wsTypeTranslator.toCxxType(m_typeName, "");
    if (!cxxType.empty())
        return cxxType;
    size_t pos = m_typeName.find(":");
    return "C" + m_typeName.substr(pos + 1);
}

void WSParserComplexType::parseSequence(XMLElement* sequence) THROWS_EXCEPTIONS
{
    for (XMLElement::const_iterator itor = sequence->begin(); itor != sequence->end(); ++itor) {
        XMLElement* element = (XMLElement*) * itor;
        string elementName = element->name();
        if (elementName == "xsd:element")
            m_sequence.push_back(new WSParserComplexType(element));
    }
}

void WSParserComplexType::parse() THROWS_EXCEPTIONS
{
    m_attributes.clear();
    if (!m_element)
        return;
    for (XMLElement::const_iterator itor = m_element->begin(); itor != m_element->end(); ++itor) {
        XMLElement* element = (XMLElement*) * itor;
        if (element->name() == "xsd:attribute") {
            string attrName = element->getAttribute("name");
            m_attributes[attrName] = new WSParserAttribute(attrName, element->getAttribute("type"));
            continue;
        }
        if (element->name() == "xsd:sequence") {
            parseSequence(element);
            continue;
        }
    }
}

std::string WSParserComplexType::wsClassName(std::string name)
{
    return name;
}

void WSParserComplexType::generateDefinition(std::ostream& classDeclaration) THROWS_EXCEPTIONS
{
    string className = "C" + wsClassName(m_name);
    set<string> usedClasses;

    string defineName = "__" + upperCase(className) + "__";
    classDeclaration << "#ifndef " << defineName << endl;
    classDeclaration << "#define " << defineName << endl;

    // determine the list of used classes
    for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); ++itor) {
        WSParserComplexType* complexType = *itor;
        string cxxType = complexType->className();
        if (cxxType[0] == 'C')
            usedClasses.insert(cxxType);
    }

    classDeclaration << endl;
    classDeclaration << "#include <sptk5/sptk.h>" << endl;
    classDeclaration << "#include <sptk5/FieldList.h>" << endl;
    classDeclaration << "#include <sptk5/wsdl/WSBasicTypes.h>" << endl;
    classDeclaration << "#include <sptk5/wsdl/WSComplexType.h>" << endl;
    classDeclaration << "#include <sptk5/wsdl/WSRestriction.h>" << endl;
    for (set<string>::iterator itor = usedClasses.begin(); itor != usedClasses.end(); ++itor)
        classDeclaration << "#include \"" << *itor << ".h\"" << endl;
    classDeclaration << endl;

    classDeclaration << "/// @brief WSDL complex type class " << className << endl;
    classDeclaration << "class " << className << " : public sptk::WSComplexType" << endl;
    classDeclaration << "{" << endl;
    classDeclaration << "public:" << endl;
    Strings ctorInitializer, copyInitializer;
    ctorInitializer.push_back(string("sptk::WSComplexType(elementName, optional)"));
    copyInitializer.push_back(string("sptk::WSComplexType(other.name().c_str(), other.isOptional())"));
    if (m_sequence.size()) {
        classDeclaration << "   // Elements" << endl;
        for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); ++itor) {
            WSParserComplexType* complexType = *itor;
            char buffer[256];
            string cxxType = complexType->className();
            if (complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE))
                cxxType = "std::vector<" + cxxType + "*>";
            else {
                string optional = (complexType->multiplicity() & WSM_OPTIONAL) ? ", true" : "";
                ctorInitializer.push_back("m_" + complexType->name() + "(\"" + complexType->name() + "\"" + optional + ")");
                copyInitializer.push_back("m_" + complexType->name() + "(\"" + complexType->name() + "\"" + optional + ")");
            }
            snprintf(buffer, sizeof(buffer), "%-20s m_%s", cxxType.c_str(), complexType->name().c_str());
            classDeclaration << "   " << buffer << ";" << endl;
        }
    }
    if (m_attributes.size()) {
        classDeclaration << "   // Attributes" << endl;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); ++itor) {
            WSParserAttribute& attr = *(itor->second);
            classDeclaration << "   " << attr.generate() << ";" << endl;
            ctorInitializer.push_back("m_" + attr.name() + "(\"" + attr.name() + "\")");
            copyInitializer.push_back("m_" + attr.name() + "(\"" + attr.name() + "\")");
        }
    }
    classDeclaration << "public:" << endl;
    classDeclaration << "   /// @brief Constructor" << endl;
    classDeclaration << "   /// @param elementName const char*, WSDL element name" << endl;
    classDeclaration << "   /// @param optional bool, Is element optional flag" << endl;
    classDeclaration << "   " << className << "(const char* elementName, bool optional=false)" << endl << "   : " << ctorInitializer.asString(", ") << endl << "   {}" << endl << endl;
    classDeclaration << "   /// @brief Copy constructor" << endl;
    classDeclaration << "   /// @param other const " << className << "&, other element to copy from" << endl;
    classDeclaration << "   " << className << "(const " << className << "& other)" << endl << "   : " << copyInitializer.asString(", ") << endl
        << "   {" << endl
        << "       copyFrom(other);" << endl
        << "   }" << endl << endl;
    classDeclaration << "   /// @brief Destructor" << endl;
    classDeclaration << "   virtual ~" << className << "();" << endl << endl;
    classDeclaration << "   /// @brief Clear content and releases allocated memory" << endl;
    classDeclaration << "   virtual void clear();" << endl << endl;
    classDeclaration << "   /// @brief Load " << className << " from XML node" << endl;
    classDeclaration << "   ///" << endl;
    classDeclaration << "   /// Complex WSDL type members are loaded recursively." << endl;
    classDeclaration << "   /// @param input const sptk::XMLElement*, XML node containing " << className << " data" << endl;
    classDeclaration << "   void load(const sptk::XMLElement* input) THROWS_EXCEPTIONS override;" << endl << endl;
    classDeclaration << "   /// @brief Load " << className << " from FieldList" << endl;
    classDeclaration << "   ///" << endl;
    classDeclaration << "   /// Only simple WSDL type members are loaded." << endl;
    classDeclaration << "   /// @param input const sptk::FieldList&, query field list containing " << className << " data" << endl;
    classDeclaration << "   virtual void load(const sptk::FieldList& input) THROWS_EXCEPTIONS;" << endl << endl;
    classDeclaration << "   /// @brief Unload " << className << " to existing XML node" << endl;
    classDeclaration << "   /// @param output sptk::XMLElement*, existing XML node" << endl;
    classDeclaration << "   void unload(sptk::XMLElement* output) const THROWS_EXCEPTIONS override;" << endl;
    classDeclaration << "};" << endl;
    classDeclaration << endl;
    classDeclaration << "#endif" << endl;
}

void WSParserComplexType::generateImplementation(std::ostream& classImplementation) THROWS_EXCEPTIONS
{
    string className = "C" + wsClassName(m_name);

    classImplementation << "#include \"" << className << ".h\"" << endl << endl;
    classImplementation << "using namespace std;" << endl;
    classImplementation << "using namespace sptk;" << endl << endl;

    // Destructor
    classImplementation << className << "::~" << className << "()" << endl;
    classImplementation << "{" << endl;
    classImplementation << "   clear();" << endl;
    classImplementation << "}" << endl << endl;

    // Clear content
    classImplementation << "void " << className << "::clear()" << endl;
    classImplementation << "{" << endl;
    classImplementation << "   // Clear elements" << endl;
    for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); ++itor) {
        WSParserComplexType* complexType = *itor;
        if (complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) {
            classImplementation << "   for (vector<" << complexType->className() << "*>::iterator itor = m_" << complexType->name() << ".begin(); itor != m_" << complexType->name() << ".end(); ++itor)" << endl;
            classImplementation << "      delete *itor;" << endl;
        }
        classImplementation << "   m_" << complexType->name() << ".clear();" << endl;
    }
    if (m_attributes.size()) {
        classImplementation << "   // Clear attributes" << endl;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); ++itor) {
            WSParserAttribute& attr = *(itor->second);
            classImplementation << "   m_" << attr.name() << ".setNull();" << endl;
        }
    }
    classImplementation << "}" << endl << endl;

    // Loader from XML element
    classImplementation << "void " << className << "::load(const XMLElement* input) THROWS_EXCEPTIONS" << endl;
    classImplementation << "{" << endl;
    classImplementation << "   clear();" << endl;
    classImplementation << "   m_loaded = true;" << endl;

    if (m_attributes.size()) {
        classImplementation << endl << "   // Load attributes" << endl;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); ++itor) {
            WSParserAttribute& attr = *(itor->second);
            classImplementation << "   m_" << attr.name() << ".load(input->getAttribute(\"" << attr.name() << "\"));" << endl;
        }
    }

    if (m_sequence.size()) {
        classImplementation << endl << "   // Load elements" << endl;
        classImplementation << "   for (XMLNode* node: *input) {" << endl;
        classImplementation << "      XMLElement* element = dynamic_cast<XMLElement*>(node);" << endl;
        classImplementation << "      if (!element) continue;" << endl;
        Strings requiredElements;
        for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); ++itor) {
            WSParserComplexType* complexType = *itor;
            classImplementation << "      if (element->name() == \"" << complexType->name() << "\") {" << endl;
            if (complexType->m_restriction)
                classImplementation << "         static const " << complexType->m_restriction->generateConstructor("restriction") << ";" << endl;
            //string optional = complexType->multiplicity() & WSM_OPTIONAL ? "true" : "false";
            if (complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) {
                classImplementation << "         " << complexType->className() << "* item = new " << complexType->className() << "(\"" << complexType->name() << "\");" << endl;
                classImplementation << "         item->load(element);" << endl;
                if (complexType->m_restriction)
                    classImplementation << "         restriction.check(m_" << complexType->name() << ".asString());" << endl;
                classImplementation << "         m_" << complexType->name() << ".push_back(item);" << endl;
            }
            else {
                classImplementation << "         m_" << complexType->name() << ".load(element);" << endl;
                if (complexType->m_restriction)
                    classImplementation << "         restriction.check(m_" << complexType->name() << ".asString());" << endl;
                classImplementation << "         continue;" << endl;
                if (complexType->multiplicity() & WSM_REQUIRED)
                    requiredElements.push_back(complexType->name());
            }
            classImplementation << "      }" << endl;
        }
        classImplementation << "   }" << endl;

        if (!requiredElements.empty()) {
            classImplementation << endl << "   // Check restrictions" << endl;
            for (string& requiredElement : requiredElements) {
                classImplementation << "   if (m_" << requiredElement << ".isNull())" << endl;
                classImplementation << "      throw SOAPException(\"Element '" << requiredElement << "' is required in '" << wsClassName(m_name) << "'.\");" << endl;
            }
        }
    }
    classImplementation << "}" << endl << endl;

    RegularExpression matchStandardType("^xsd:");
    
    // Loader from FieldList
    classImplementation << "void " << className << "::load(const FieldList& input) THROWS_EXCEPTIONS" << endl;
    classImplementation << "{" << endl;
    classImplementation << "   clear();" << endl;
    classImplementation << "   m_loaded = true;" << endl;
    
    stringstream fieldLoads;
    int fieldLoadCount = 0;
    
    if (m_attributes.size()) {
        fieldLoads << endl << "   // Load attributes" << endl;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); ++itor) {
            WSParserAttribute& attr = *(itor->second);
            fieldLoads << "   if ((field = input.fieldByName(\"" << attr.name() << "\"))) {" << endl;
            fieldLoads << "      m_" << attr.name() << ".load(*field);" << endl;
            fieldLoads << "   }" << endl;
            fieldLoadCount++;
        }
    }

    Strings requiredElements;
    
    if (m_sequence.size()) {
        fieldLoads << endl << "   // Load elements" << endl;
        for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); ++itor) {
            WSParserComplexType* complexType = *itor;
            if (complexType->multiplicity() & WSM_REQUIRED)
                requiredElements.push_back(complexType->name());
            if (complexType->m_typeName != matchStandardType)
                continue;
            fieldLoadCount++;
            fieldLoads << "   if ((field = input.fieldByName(\"" << complexType->name() << "\"))) {" << endl;
            if (complexType->m_restriction)
                fieldLoads << "      static const " << complexType->m_restriction->generateConstructor("restriction") << ";" << endl;
            if (complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) {
                fieldLoads << "      " << complexType->className() << "* item = new " << complexType->className() << "(\"" << complexType->name() << "\");" << endl;
                fieldLoads << "      item->load(*field);" << endl;
                if (complexType->m_restriction)
                    fieldLoads << "      restriction.check(m_" << complexType->name() << ".asString());" << endl;
                fieldLoads << "      m_" << complexType->name() << ".push_back(item);" << endl;
            }
            else {
                fieldLoads << "      m_" << complexType->name() << ".load(*field);" << endl;
                if (complexType->m_restriction)
                    fieldLoads << "      restriction.check(m_" << complexType->name() << ".asString());" << endl;
            }
            fieldLoads << "   }" << endl;
        }
    }

    if (fieldLoadCount) {
        classImplementation << "   Field* field;" << endl;
        classImplementation << fieldLoads.str();
    }

    if (!requiredElements.empty()) {
        classImplementation << endl << "   // Check restrictions" << endl;
        for (string& requiredElement : requiredElements) {
            classImplementation << "   if (m_" << requiredElement << ".isNull())" << endl;
            classImplementation << "      throw SOAPException(\"Element '" << requiredElement << "' is required in '" << wsClassName(m_name) << "'.\");" << endl;
        }
    }
    
    classImplementation << "}" << endl << endl;

    // Unloader
    classImplementation << "void " << className << "::unload(XMLElement* output) const THROWS_EXCEPTIONS" << endl;
    classImplementation << "{" << endl;
    if (m_attributes.size()) {
        classImplementation << "   // Unload attributes" << endl;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); ++itor) {
            WSParserAttribute& attr = *(itor->second);
            classImplementation << "   output->setAttribute(\"" << attr.name() << "\", m_" << attr.name() << ".asString());" << endl;
        }
    }
    if (m_sequence.size()) {
        classImplementation << "   // Unload elements" << endl;
        for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); ++itor) {
            WSParserComplexType* complexType = *itor;
            if (complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) {
                classImplementation << "   for (vector<" << complexType->className() << "*>::const_iterator itor = m_" << complexType->name() << ".begin(); "
                    << " itor != m_" << complexType->name() << ".end(); ++itor) {" << endl;
                classImplementation << "      " << complexType->className() << "* item = *itor;" << endl;
                classImplementation << "      item->addElement(output);" << endl;
                classImplementation << "   }" << endl;
            }
            else {
                classImplementation << "   m_" << complexType->name() << ".addElement(output);" << endl;
            }
        }
    }
    classImplementation << "}" << endl;
}

void WSParserComplexType::generate(ostream& classDeclaration, ostream& classImplementation, string externalHeader) THROWS_EXCEPTIONS
{
    if (externalHeader.length()) {
        classDeclaration << externalHeader << endl;
        classImplementation << externalHeader << endl;
    }

    generateDefinition(classDeclaration);
    generateImplementation(classImplementation);
}

