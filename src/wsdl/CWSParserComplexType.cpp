/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSParserComplexType.cpp  -  description
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

#include <sptk5/wsdl/CWSParserComplexType.h>
#include <sptk5/wsdl/CWSTypeTranslator.h>
#include <sstream>
#include <stdio.h>
#include <set>

using namespace std;
using namespace sptk;

CWSParserAttribute::CWSParserAttribute(std::string name, std::string typeName)
:   m_name(name),
    m_wsTypeName(typeName),
    m_multiplicity(CWSM_OPTIONAL)
{
    m_cxxTypeName = wsTypeTranslator.toCxxType(typeName);
}

CWSParserAttribute::CWSParserAttribute(const CWSParserAttribute& attr)
:   m_name(attr.m_name),
    m_wsTypeName(attr.m_wsTypeName),
    m_cxxTypeName(attr.m_cxxTypeName),
    m_multiplicity(attr.m_multiplicity)
{
}

string CWSParserAttribute::generate() const
{
    char buffer[256];
    sprintf(buffer, "%-20s m_%s", m_cxxTypeName.c_str(), m_name.c_str());
    return buffer;
}

CWSParserComplexType::CWSParserComplexType(const CXmlElement* complexTypeElement, string name, string typeName)
: m_element(complexTypeElement), m_refcount(0), m_restriction(NULL)
{
    m_name = name.empty() ? complexTypeElement->getAttribute("name").str() : name;
    m_typeName = typeName.empty() ? complexTypeElement->getAttribute("type").str() : typeName;

    if (m_typeName == "" && complexTypeElement->name() == "xsd:element") {
        CXmlNode* restrictionElement = complexTypeElement->findFirst("xsd:restriction");
        if (restrictionElement) {
            m_typeName = restrictionElement->getAttribute("base").c_str();
            m_restriction = new WSRestriction(m_typeName, (CXmlElement*) restrictionElement->parent());
        }
    }

    if (m_typeName.empty())
        m_typeName = m_name;

    string maxOccurs, minOccurs;
    if (complexTypeElement->hasAttribute("maxOccurs"))
        maxOccurs = (string) complexTypeElement->getAttribute("maxOccurs");
    if (complexTypeElement->hasAttribute("minOccurs"))
        minOccurs = (string) complexTypeElement->getAttribute("minOccurs");

    m_multiplicity = CWSM_REQUIRED;

    // Relaxed defaults, in case of incomplete or missing multiplicity
    if (minOccurs.empty())
        minOccurs = "1";
    if (maxOccurs.empty())
        maxOccurs = "1";

    if (minOccurs == "0")
        m_multiplicity = maxOccurs == "1" ? CWSM_OPTIONAL : CWSM_ZERO_OR_MORE;
    else if (minOccurs == "1")
        m_multiplicity = maxOccurs == "1" ? CWSM_REQUIRED : CWSM_ONE_OR_MORE;
}

CWSParserComplexType::~CWSParserComplexType()
{
    if (m_refcount > 0)
        throwException("Can't delete complex type: refcount is greater than 0");
    for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); itor++)
        delete *itor;
    for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); itor++)
        delete itor->second;
}

string CWSParserComplexType::className() const
{
    string cxxType = wsTypeTranslator.toCxxType(m_typeName,"");
    if (!cxxType.empty())
        return cxxType;
    size_t pos = m_typeName.find(":");
    return "C" + m_typeName.substr(pos + 1);
}

void CWSParserComplexType::parseSequence(CXmlElement* sequence) THROWS_EXCEPTIONS
{
    for (CXmlElement::const_iterator itor = sequence->begin(); itor != sequence->end(); itor++) {
        CXmlElement* element = (CXmlElement*) *itor;
        string elementName = element->name();
        if (elementName == "xsd:element")
            m_sequence.push_back(new CWSParserComplexType(element));
    }
}

void CWSParserComplexType::parse() THROWS_EXCEPTIONS
{
    m_attributes.clear();
    if (!m_element)
        return;
    for (CXmlElement::const_iterator itor = m_element->begin(); itor != m_element->end(); itor++) {
        CXmlElement* element = (CXmlElement*) *itor;
        if (element->name() == "xsd:attribute") {
            string attrName = element->getAttribute("name");
            m_attributes[attrName] = new CWSParserAttribute(attrName, element->getAttribute("type"));
            continue;
        }
        if (element->name() == "xsd:sequence") {
            parseSequence(element);
            continue;
        }
    }
}

std::string CWSParserComplexType::wsClassName(std::string name)
{
    return name;
}

void CWSParserComplexType::generateDefinition(std::ostream& classDeclaration) THROWS_EXCEPTIONS
{
    string className = "C" + wsClassName(m_name);
    set<string> usedClasses;

    classDeclaration << "// Complex type " << className << " declaration" << endl << endl;

    string defname = "__" + upperCase(className) + "__";
    classDeclaration << "#ifndef " << defname << endl;
    classDeclaration << "#define " << defname << endl;

    // determine the list of used classes
    for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); itor++) {
        CWSParserComplexType* complexType = *itor;
        string cxxType = complexType->className();
        if (cxxType[0] == 'C')
            usedClasses.insert(cxxType);
    }

    classDeclaration << endl;
    classDeclaration << "#include <sptk5/sptk.h>" << endl;
    classDeclaration << "#include <sptk5/wsdl/CWSBasicTypes.h>" << endl;
    classDeclaration << "#include <sptk5/wsdl/CWSComplexType.h>" << endl;
    classDeclaration << "#include <sptk5/wsdl/CWSRestriction.h>" << endl;
    for (set<string>::iterator itor = usedClasses.begin(); itor != usedClasses.end(); itor++)
        classDeclaration << "#include \"" << *itor << ".h\"" << endl;
    classDeclaration << endl;

    classDeclaration << "/// @brief WSDL complex type class " << className << endl;
    classDeclaration << "class " << className << " : public sptk::WSComplexType" << endl;
    classDeclaration << "{" << endl;
    classDeclaration << "public:" << endl;
    CStrings ctorInitializer;
    ctorInitializer.push_back("sptk::WSComplexType(elementName, optional)");
    if (m_sequence.size()) {
        classDeclaration << "   // Elements" << endl;
        for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); itor++) {
            CWSParserComplexType* complexType = *itor;
            char buffer[256];
            string cxxType = complexType->className();
            if (complexType->multiplicity() & (CWSM_ZERO_OR_MORE|CWSM_ONE_OR_MORE))
                cxxType = "std::vector<" + cxxType + "*>";
            else {
                string optional = complexType->multiplicity() & CWSM_OPTIONAL ? ", true" : "";
                ctorInitializer.push_back("m_" + complexType->name() + "(\"" + complexType->name() + "\"" + optional + ")");
            }
            sprintf(buffer, "%-20s m_%s", cxxType.c_str(), complexType->name().c_str());
            classDeclaration << "   " << buffer << ";" << endl;
        }
    }
    if (m_attributes.size()) {
        classDeclaration << "   // Attributes" << endl;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); itor++) {
            CWSParserAttribute& attr = *(itor->second);
            classDeclaration << "   " << attr.generate() << ";" << endl;
            ctorInitializer.push_back("m_" + attr.name() + "(\"" + attr.name() + "\")");
        }
    }
    classDeclaration << "public:" << endl;
    classDeclaration << "   /// @brief Constructor" << endl;
    classDeclaration << "   /// @param elementName const char*, WSDL element name" << endl;
    classDeclaration << "   /// @param optional bool, Is element optional flag" << endl;
    classDeclaration << "   " << className << "(const char* elementName, bool optional=false)" << endl << "   : " << ctorInitializer.asString(", ") << endl << "   {}" << endl << endl;
    classDeclaration << "   /// @brief Destructor" << endl;
    classDeclaration << "   virtual ~" << className << "();" << endl << endl;
    classDeclaration << "   /// @brief Clear content and releases allocated memory" << endl;
    classDeclaration << "   virtual void clear();" << endl << endl;
    classDeclaration << "   /// @brief Load " << className << " from XML node" << endl;
    classDeclaration << "   /// @param input const sptk::CXmlElement*, XML node containing " << className << " data" << endl;
    classDeclaration << "   virtual void load(const sptk::CXmlElement* input) THROWS_EXCEPTIONS;" << endl << endl;
    classDeclaration << "   /// @brief Unload " << className << " to existing XML node" << endl;
    classDeclaration << "   /// @param output sptk::CXmlElement*, existing XML node" << endl;
    classDeclaration << "   virtual void unload(sptk::CXmlElement* output) const THROWS_EXCEPTIONS;" << endl;
    classDeclaration << "};" << endl;
    classDeclaration << endl;
    classDeclaration << "#endif" << endl;
}

void CWSParserComplexType::generateImplementation(std::ostream& classImplementation) THROWS_EXCEPTIONS
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
    for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); itor++) {
        CWSParserComplexType* complexType = *itor;
        if (complexType->multiplicity() & (CWSM_ZERO_OR_MORE | CWSM_ONE_OR_MORE)) {
            classImplementation << "   for (vector<" << complexType->className() << "*>::iterator itor = m_" << complexType->name() << ".begin(); itor != m_" << complexType->name() << ".end(); itor++)" << endl;
            classImplementation << "      delete *itor;" << endl;
            classImplementation << "   m_" << complexType->name() << ".clear();" << endl;
        }
    }
    classImplementation << "}" << endl << endl;

    // Loader
    classImplementation << "void " << className << "::load(const CXmlElement* input) THROWS_EXCEPTIONS" << endl;
    classImplementation << "{" << endl;
    if (m_attributes.size()) {
        classImplementation << "   // Load attributes" << endl;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); itor++) {
            CWSParserAttribute& attr = *(itor->second);
            classImplementation << "   m_" << attr.name() << ".load(input->getAttribute(\"" << attr.name() << "\"));" << endl;
        }
    }
    if (m_sequence.size()) {
        classImplementation << "   clear();" << endl;
        classImplementation << "   // Load elements" << endl;
        classImplementation << "   for (CXmlElement::const_iterator itor = input->begin(); itor != input->end(); itor++) {" << endl;
        classImplementation << "      CXmlElement* element = (CXmlElement*) *itor;" << endl;
        for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); itor++) {
            CWSParserComplexType* complexType = *itor;
            classImplementation << "      if (element->name() == \"" << complexType->name() << "\") {" << endl;
            if (complexType->m_restriction)
                classImplementation << "         static const " << complexType->m_restriction->generateConstructor("restriction") << ";" << endl;
            //string optional = complexType->multiplicity() & CWSM_OPTIONAL ? "true" : "false";
            if (complexType->multiplicity() & (CWSM_ZERO_OR_MORE|CWSM_ONE_OR_MORE)) {
                classImplementation << "         " << complexType->className() << "* item = new " << complexType->className() << "(\"" << complexType->name() << "\");" << endl;
                classImplementation << "         item->load(element);" << endl;
                if (complexType->m_restriction)
                    classImplementation << "         restriction.check(m_" << complexType->name() << ".asString());" << endl;
                classImplementation << "         m_" << complexType->name() << ".push_back(item);" << endl;
            } else {
                classImplementation << "         m_" << complexType->name() << ".load(element);" << endl;
                if (complexType->m_restriction)
                    classImplementation << "         restriction.check(m_" << complexType->name() << ".asString());" << endl;
                classImplementation << "         continue;" << endl;
            }
            classImplementation << "      }" << endl;
        }
        classImplementation << "   }" << endl;
    }
    classImplementation << "}" << endl << endl;

    // Unloader
    classImplementation << "void " << className << "::unload(CXmlElement* output) const THROWS_EXCEPTIONS" << endl;
    classImplementation << "{" << endl;
    if (m_attributes.size()) {
        classImplementation << "   // Unload attributes" << endl;
        for (AttributeMap::iterator itor = m_attributes.begin(); itor != m_attributes.end(); itor++) {
            CWSParserAttribute& attr = *(itor->second);
            classImplementation << "   output->setAttribute(\"" << attr.name() << "\", m_" << attr.name() << ".asString());" << endl;
        }
    }
    if (m_sequence.size()) {
        classImplementation << "   // Unload elements" << endl;
        for (ElementList::iterator itor = m_sequence.begin(); itor != m_sequence.end(); itor++) {
            CWSParserComplexType* complexType = *itor;
            if (complexType->multiplicity() & (CWSM_ZERO_OR_MORE|CWSM_ONE_OR_MORE)) {
                classImplementation << "   for (vector<" << complexType->className() << "*>::const_iterator itor = m_" << complexType->name() << ".begin(); "
                                    << " itor != m_" << complexType->name() << ".end(); itor++) {" << endl;
                classImplementation << "      " << complexType->className() << "* item = *itor;" << endl;
                classImplementation << "      item->addElement(output);" << endl;
                classImplementation << "   }" << endl;
            } else {
                classImplementation << "   m_" << complexType->name() << ".addElement(output);" << endl;
            }
        }
    }
    classImplementation << "}" << endl;
}

void CWSParserComplexType::generate(ostream& classDeclaration, ostream& classImplementation) THROWS_EXCEPTIONS
{
    generateDefinition(classDeclaration);
    generateImplementation(classImplementation);
}

