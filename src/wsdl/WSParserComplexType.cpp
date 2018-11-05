/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSParserComplexType.cpp - description                  ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/wsdl/WSParserComplexType.h>
#include <sptk5/wsdl/WSTypeTranslator.h>
#include <iomanip>

using namespace std;
using namespace sptk;

WSParserAttribute::WSParserAttribute(const String& name, const String& typeName)
: m_name(name),
  m_wsTypeName(typeName)
{
    m_cxxTypeName = wsTypeTranslator.toCxxType(typeName);
}

String WSParserAttribute::generate() const
{
    stringstream str;
    str << left << setw(20) << m_cxxTypeName << " m_" << m_name;
    return str.str();
}

WSParserComplexType::WSParserComplexType(const xml::Element* complexTypeElement, const String& name,
                                         const String& typeName)
: m_element(complexTypeElement), m_refcount(0), m_restriction(nullptr)
{
    m_name = name.empty() ? complexTypeElement->getAttribute("name").str() : name;
    m_typeName = typeName.empty() ? complexTypeElement->getAttribute("type").str() : typeName;

    if (m_typeName.empty() && complexTypeElement->name() == "xsd:element") {
        xml::Node* restrictionElement = complexTypeElement->findFirst("xsd:restriction");
        if (restrictionElement != nullptr) {
            m_typeName = restrictionElement->getAttribute("base").c_str();
            m_restriction = new WSRestriction(m_typeName, (xml::Element*) restrictionElement->parent());
        }
    }

    xml::Node* documentationElement = complexTypeElement->findFirst("xsd:documentation");
    if (documentationElement != nullptr)
        m_documentation = documentationElement->text();

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
        for (auto element: m_sequence)
            delete element;
        for (auto itor: m_attributes)
            delete itor.second;
        delete m_restriction;
    }
}

String WSParserComplexType::className() const
{
    String cxxType = wsTypeTranslator.toCxxType(m_typeName, "");
    if (!cxxType.empty())
        return cxxType;
    size_t pos = m_typeName.find(':');
    return "C" + m_typeName.substr(pos + 1);
}

void WSParserComplexType::parseSequence(xml::Element* sequence)
{
    for (auto node: *sequence) {
        auto element = dynamic_cast<xml::Element*>(node);
        if (element == nullptr)
            throw Exception("The node " + node->name() + " is not an XML element");
        string elementName = element->name();
        if (elementName == "xsd:element")
            m_sequence.push_back(new WSParserComplexType(element));
    }
}

void WSParserComplexType::parse()
{
    m_attributes.clear();
    if (m_element == nullptr)
        return;
    for (auto node: *m_element) {
        auto element = dynamic_cast<xml::Element*>(node);
        if (element == nullptr)
            throw Exception("The node " + node->name() + " is not an XML element");
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

String WSParserComplexType::wsClassName(const String& name)
{
    return name;
}

void WSParserComplexType::generateDefinition(std::ostream& classDeclaration)
{
    String className = "C" + wsClassName(m_name);
    set<String> usedClasses;

    String defineName = "__" + className.toUpperCase() + "__";
    classDeclaration << "#ifndef " << defineName << endl;
    classDeclaration << "#define " << defineName << endl;

    // determine the list of used classes
    for (auto complexType: m_sequence) {
        String cxxType = complexType->className();
        if (cxxType[0] == 'C')
            usedClasses.insert(cxxType);
    }
    classDeclaration << endl;

    Strings includeFiles;
    includeFiles.push_back("#include <sptk5/sptk.h>");
    includeFiles.push_back("#include <sptk5/FieldList.h>");
    includeFiles.push_back("#include <sptk5/threads/Locks.h>");
    includeFiles.push_back("#include <sptk5/db/QueryParameterList.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSBasicTypes.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSComplexType.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSRestriction.h>");
    for (auto& usedClass: usedClasses)
        includeFiles.push_back("#include \"" + usedClass + ".h\"");
    includeFiles.sort();
    classDeclaration << includeFiles.join("\n") << endl << endl;

    Strings words;
    String  tagName = lowerCase(className.substr(1));
    RegularExpression matchWords("([A-Z]+[a-z]+)", "g");
    if (matchWords.m(className.substr(1), words)) {
        tagName = words.join("_").toLowerCase();
    }

    classDeclaration << "/**" << endl;
    classDeclaration << " * WSDL complex type class " << className << "." << endl;
    classDeclaration << " * Generated by wsdl2cxx SPTK utility." << endl;
    classDeclaration << " */" << endl;
    classDeclaration << "class " << className << " : public sptk::WSComplexType" << endl;
    classDeclaration << "{" << endl;
    classDeclaration << "public:" << endl;
    Strings ctorInitializer, copyInitializer;
    ctorInitializer.push_back(string("sptk::WSComplexType(elementName, optional)"));
    copyInitializer.push_back(string("sptk::WSComplexType(other.complexTypeName().c_str(), other.isOptional())"));
    if (!m_sequence.empty()) {
        classDeclaration << "   // Elements" << endl;
        for (auto complexType: m_sequence) {
            String cxxType = complexType->className();
            if (!complexType->documentation().empty()) {
                classDeclaration << endl;
                classDeclaration << "   /**" << endl;
                Strings rows(complexType->documentation(), "[\n\r]+", Strings::SM_REGEXP);
                for (const String& row: rows) {
                    classDeclaration << "    * " << trim(row) << endl;
                }
                classDeclaration << "    */" << endl;
            }
            if (complexType->isArray())
                cxxType = "sptk::WSArray<" + cxxType + "*>";
            else {
                string optional = (complexType->multiplicity() & WSM_OPTIONAL) != 0 ? ", true" : "";
                ctorInitializer.push_back("m_" + complexType->name() + "(\"" + complexType->name() + "\"" + optional + ")");
                copyInitializer.push_back("m_" + complexType->name() + "(\"" + complexType->name() + "\"" + optional + ")");
            }

            classDeclaration << "   " << left << setw(20) << cxxType << " m_" << complexType->name() << ";" << endl;
        }
    }
    if (!m_attributes.empty()) {
        classDeclaration << "   // Attributes" << endl;
        for (auto itor: m_attributes) {
            WSParserAttribute& attr = *itor.second;
            classDeclaration << "   " << attr.generate() << ";" << endl;
            ctorInitializer.push_back("m_" + attr.name() + "(\"" + attr.name() + "\")");
            copyInitializer.push_back("m_" + attr.name() + "(\"" + attr.name() + "\")");
        }
    }
    classDeclaration << "public:" << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Disabled move constructor" << endl;
    classDeclaration << "    * @param other              Other element to move from" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   " << className << "(" << className << "&& other) = delete;" << endl << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Disabled move assignment" << endl;
    classDeclaration << "    * @param other              Other element to move from" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   " << className << "& operator = (" << className << "&& other) = delete;" << endl << endl
                     << endl
                     << "protected:" << endl
                     << "   /**" << endl
                     << "    * Clear content and releases allocated memory (internal)" << endl
                     << "    */" << endl
                     << "   void _clear() override;" << endl << endl;
    classDeclaration << "public:" << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Constructor" << endl;
    classDeclaration << "    * @param elementName        WSDL element name" << endl;
    classDeclaration << "    * @param optional bool, Is element optional flag" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   explicit " << className << "(const char* elementName=\"" << tagName << "\", bool optional=false) noexcept" << endl << "   : " << ctorInitializer.asString(", ") << endl << "   {}" << endl << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Copy constructor" << endl;
    classDeclaration << "    * @param other              Other element to copy from" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   " << className << "(const " << className << "& other) noexcept" << endl << "   : " << copyInitializer.asString(", ") << endl
                     << "   {" << endl
                     << "       copyFrom(other);" << endl
                     << "   }" << endl << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Destructor" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   ~" << className << "() override;" << endl << endl
                     << "   /**" << endl
                     << "    * Copy assignment" << endl
                     << "    * @param other              Other element to copy from" << endl
                     << "    */" << endl
                     << "   " << className << "& operator = (const " << className << "& other)" << endl
                     << "   {" << endl
                     << "       copyFrom(other);" << endl
                     << "       return *this;" << endl
                     << "   }" << endl << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Load " << className << " from XML node" << endl;
    classDeclaration << "    *" << endl;
    classDeclaration << "    * Complex WSDL type members are loaded recursively." << endl;
    classDeclaration << "    * @param input              XML node containing " << className << " data" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void load(const sptk::xml::Element* input) override;" << endl << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Load " << className << " from FieldList" << endl;
    classDeclaration << "    *" << endl;
    classDeclaration << "    * Only simple WSDL type members are loaded." << endl;
    classDeclaration << "    * @param input              Query field list containing " << className << " data" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void load(const sptk::FieldList& input) override;" << endl << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Unload " << className << " to existing XML node" << endl;
    classDeclaration << "    * @param output             Existing XML node" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void unload(sptk::xml::Element* output) const override;" << endl << endl;
    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Unload " << className << " to Query's parameters" << endl;
    classDeclaration << "    * @param output             Query parameters" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void unload(sptk::QueryParameterList& output) const override;" << endl;
    classDeclaration << "};" << endl;
    classDeclaration << endl;
    classDeclaration << "#endif" << endl;
}

void WSParserComplexType::generateImplementation(std::ostream& classImplementation)
{
    String className = "C" + wsClassName(m_name);

    classImplementation << "#include \"" << className << ".h\"" << endl << endl;
    classImplementation << "using namespace std;" << endl;
    classImplementation << "using namespace sptk;" << endl << endl;

    // Destructor
    classImplementation << className << "::~" << className << "()" << endl;
    classImplementation << "{" << endl;
    classImplementation << "    clear();" << endl;
    classImplementation << "}" << endl << endl;

    // Clear content
    classImplementation << "void " << className << "::_clear()" << endl;
    classImplementation << "{" << endl;
    classImplementation << "    // Clear elements" << endl;
    for (auto complexType: m_sequence) {
        if ((complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) != 0) {
            classImplementation << "    for (auto element: m_" << complexType->name() << ")" << endl;
            classImplementation << "        delete element;" << endl;
        }
        classImplementation << "    m_" << complexType->name() << ".clear();" << endl;
    }
    if (!m_attributes.empty()) {
        classImplementation << "    // Clear attributes" << endl;
        for (auto itor: m_attributes) {
            WSParserAttribute& attr = *(itor.second);
            classImplementation << "    m_" << attr.name() << ".setNull(VAR_NONE);" << endl;
        }
    }
    classImplementation << "}" << endl << endl;

    // Loader from XML element
    classImplementation << "void " << className << "::load(const sptk::xml::Element* input)" << endl
                        << "{" << endl
                        << "    UniqueLock(m_mutex);" << endl
                        << "    _clear();" << endl
                        << "    m_loaded = true;" << endl;

    if (!m_attributes.empty()) {
        classImplementation << endl << "    // Load attributes" << endl;
        for (auto itor: m_attributes) {
            WSParserAttribute& attr = *itor.second;
            classImplementation << "    m_" << attr.name() << ".load(input->getAttribute(\"" << attr.name() << "\"));" << endl;
        }
    }

    if (!m_sequence.empty()) {
        classImplementation << endl << "    // Load elements" << endl;
        classImplementation << "    for (auto node: *input) {" << endl;
        classImplementation << "        auto element = dynamic_cast<xml::Element*>(node);" << endl;
        classImplementation << "        if (element == nullptr) {" << endl;
        classImplementation << "            continue;" << endl;
        classImplementation << "        }" << endl;
        Strings requiredElements;
        for (auto complexType: m_sequence) {
            classImplementation << endl;
            classImplementation << "        if (element->name() == \"" << complexType->name() << "\") {" << endl;
            if (complexType->m_restriction != nullptr)
                classImplementation << "            static const " << complexType->m_restriction->generateConstructor("restriction") << ";" << endl;
            //string optional = complexType->multiplicity() & WSM_OPTIONAL ? "true" : "false";
            if ((complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) != 0) {
                classImplementation << "            auto item = new " << complexType->className() << "(\"" << complexType->name() << "\");" << endl;
                classImplementation << "            item->load(element);" << endl;
                if (complexType->m_restriction != nullptr)
                    classImplementation << "            restriction.check(\"" << complexType->name() << "\", m_" << complexType->name() << ".asString());" << endl;
                classImplementation << "            m_" << complexType->name() << ".push_back(item);" << endl;
            }
            else {
                classImplementation << "            m_" << complexType->name() << ".load(element);" << endl;
                if (complexType->m_restriction != nullptr)
                    classImplementation << "            restriction.check(\"" << complexType->name() << "\", m_" << complexType->name() << ".asString());" << endl;
                classImplementation << "            continue;" << endl;
                if ((complexType->multiplicity() & WSM_REQUIRED) != 0)
                    requiredElements.push_back(complexType->name());
            }
            classImplementation << "        }" << endl;
        }
        classImplementation << "    }" << endl;

        if (!requiredElements.empty()) {
            classImplementation << endl << "    // Check restrictions" << endl;
            bool first = true;
            for (string& requiredElement : requiredElements) {
                if (first)
                    first = false;
                else
                    classImplementation << endl;
                classImplementation << "    if (m_" << requiredElement << ".isNull())" << endl;
                classImplementation << "        throw SOAPException(\"Element '" << requiredElement << "' is required in '" << wsClassName(m_name) << "'.\");" << endl;
            }
        }
    }
    classImplementation << "}" << endl << endl;

    RegularExpression matchStandardType("^xsd:");

    // Loader from FieldList
    classImplementation << "void " << className << "::load(const sptk::FieldList& input)" << endl
                        << "{" << endl
                        << "    UniqueLock(m_mutex);" << endl
                        << "    _clear();" << endl
                        << "    m_loaded = true;" << endl;

    stringstream fieldLoads;
    int fieldLoadCount = 0;

    if (!m_attributes.empty()) {
        fieldLoads << endl << "    // Load attributes" << endl;
        for (auto itor: m_attributes) {
            WSParserAttribute& attr = *itor.second;
            fieldLoads << "    if ((field = input.fieldByName(\"" << attr.name() << "\")) != nullptr) {" << endl;
            fieldLoads << "        m_" << attr.name() << ".load(*field);" << endl;
            fieldLoads << "    }" << endl << endl;
            fieldLoadCount++;
        }
    }

    Strings requiredElements;

    if (!m_sequence.empty()) {
        fieldLoads << endl << "    // Load elements" << endl;
        for (auto complexType: m_sequence) {
            if ((complexType->multiplicity() & WSM_REQUIRED) != 0)
                requiredElements.push_back(complexType->name());
            if (!matchStandardType.matches(complexType->m_typeName))
                continue;
            fieldLoadCount++;
            fieldLoads << "    if ((field = input.fieldByName(\"" << complexType->name() << "\")) != nullptr) {" << endl;
            if (complexType->m_restriction != nullptr)
                fieldLoads << "        static const " << complexType->m_restriction->generateConstructor("restriction") << ";" << endl;
            if ((complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) != 0) {
                fieldLoads << "        auto item = new " << complexType->className() << "(\"" << complexType->name() << "\");" << endl;
                fieldLoads << "        item->load(*field);" << endl;
                if (complexType->m_restriction != nullptr)
                    fieldLoads << "        restriction.check(\"" << complexType->name() << "\", m_" << complexType->name() << ".asString());" << endl;
                fieldLoads << "        m_" << complexType->name() << ".push_back(item);" << endl;
            }
            else {
                fieldLoads << "        m_" << complexType->name() << ".load(*field);" << endl;
                if (complexType->m_restriction != nullptr)
                    fieldLoads << "        restriction.check(\"" << complexType->name() << "\", m_" << complexType->name() << ".asString());" << endl;
            }
            fieldLoads << "    }" << endl << endl;
        }
    }

    if (fieldLoadCount != 0) {
        classImplementation << "    Field* field;" << endl;
        classImplementation << fieldLoads.str();
    }

    if (!requiredElements.empty()) {
        classImplementation << "    // Check restrictions" << endl;
        bool first = true;
        for (string& requiredElement : requiredElements) {
            if (first)
                first = false;
            else
                classImplementation << endl;
            classImplementation << "    if (m_" << requiredElement << ".isNull())" << endl;
            classImplementation << "        throw SOAPException(\"Element '" << requiredElement << "' is required in '" << wsClassName(m_name) << "'.\");" << endl;
        }
    }

    classImplementation << "}" << endl << endl;

    // Unloader to Element
    classImplementation << "void " << className << "::unload(sptk::xml::Element* output) const" << endl
                        << "{" << endl
                        << "    SharedLock(m_mutex);" << endl;
    if (!m_attributes.empty()) {
        classImplementation << "    // Unload attributes" << endl;
        for (auto itor: m_attributes) {
            WSParserAttribute& attr = *itor.second;
            classImplementation << "    output->setAttribute(\"" << attr.name() << "\", m_" << attr.name() << ".asString());" << endl;
        }
    }
    if (!m_sequence.empty()) {
        classImplementation << "    // Unload elements" << endl;
        for (auto complexType: m_sequence) {
            if ((complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) != 0) {
                classImplementation << "    for (auto element: m_" << complexType->name() << ")" << endl;
                classImplementation << "        element->addElement(output);" << endl;
            }
            else
                classImplementation << "    m_" << complexType->name() << ".addElement(output);" << endl;
        }
    }
    classImplementation << "}" << endl << endl;

    // Unloader to ParamList
    classImplementation << "void " << className << "::unload(sptk::QueryParameterList& output) const" << endl
                        << "{" << endl
                        << "    SharedLock(m_mutex);" << endl;

    if (!m_attributes.empty()) {
        classImplementation << "    // Unload attributes" << endl;
        for (auto itor: m_attributes) {
            WSParserAttribute& attr = *itor.second;
            classImplementation << "    WSComplexType::unload(output, \"" << attr.name() << "\", &m_" << attr.name() << ");" << endl;
        }
    }

    if (!m_sequence.empty()) {
        if (!m_attributes.empty())
            classImplementation << endl;
        classImplementation << "    // Unload attributes" << endl;
        for (auto complexType: m_sequence) {
            if (!complexType->isArray())
                classImplementation << "    WSComplexType::unload(output, \"" << complexType->name() << "\", dynamic_cast<const WSBasicType*>(&m_" << complexType->name() << "));" << endl;
        }
    }

    classImplementation << "}" << endl;
}

void WSParserComplexType::generate(ostream& classDeclaration, ostream& classImplementation,
                                   const String& externalHeader)
{
    if (!externalHeader.empty()) {
        classDeclaration << externalHeader << endl;
        classImplementation << externalHeader << endl;
    }

    generateDefinition(classDeclaration);
    generateImplementation(classImplementation);
}
