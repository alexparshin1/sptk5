/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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

#include <sptk5/RegularExpression.h>
#include <sptk5/wsdl/WSParserComplexType.h>
#include <sptk5/wsdl/WSTypeTranslator.h>
#include <iomanip>

using namespace std;
using namespace sptk;

std::map<String, const xml::Element*> WSParserComplexType::SimpleTypeElements;

WSParserAttribute::WSParserAttribute(const String& name, const String& typeName)
: m_name(name),
  m_wsTypeName(typeName)
{
    m_cxxTypeName = wsTypeTranslator.toCxxType(typeName);
}

String WSParserAttribute::generate(bool initialize) const
{
    stringstream str;
    str << left << setw(30) << m_cxxTypeName << " m_" << m_name;
    if (initialize)
        str << " {\"" << m_name << "\"}";
    return str.str();
}

WSParserComplexType::WSParserComplexType(const xml::Element* complexTypeElement, const String& name,
                                         const String& typeName)
: m_name(name.empty() ? (String) complexTypeElement->getAttribute("name") : name),
  m_typeName(typeName.empty() ? (String) complexTypeElement->getAttribute("type") : typeName),
  m_element(complexTypeElement)
{
    const xml::Element* simpleTypeElement = nullptr;
    if (!m_typeName.empty())
        simpleTypeElement = findSimpleType(m_typeName);
    else if (complexTypeElement->name() == "xsd:element")
        simpleTypeElement = m_element;

    if (simpleTypeElement) {
        const auto* restrictionElement = simpleTypeElement->findFirst("xsd:restriction");
        if (restrictionElement != nullptr) {
            m_typeName = (String) restrictionElement->getAttribute("base");
            m_restriction = make_shared<WSRestriction>(m_typeName, (xml::Element*) restrictionElement->parent());
        }
    }

    const xml::Node* documentationElement = complexTypeElement->findFirst("xsd:documentation");
    if (documentationElement != nullptr)
        m_documentation = documentationElement->text().trim();

    if (m_typeName.empty())
        m_typeName = m_name;

    String maxOccurs;
    String minOccurs;
    if (complexTypeElement->hasAttribute("maxOccurs"))
        maxOccurs = (String) complexTypeElement->getAttribute("maxOccurs");
    if (complexTypeElement->hasAttribute("minOccurs"))
        minOccurs = (String) complexTypeElement->getAttribute("minOccurs");

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

String WSParserComplexType::className() const
{
    String cxxType = wsTypeTranslator.toCxxType(m_typeName, "");
    if (!cxxType.empty())
        return cxxType;
    size_t pos = m_typeName.find(':');
    return "C" + m_typeName.substr(pos + 1);
}

void WSParserComplexType::parseSequence(const xml::Element* sequence)
{
    for (auto* node: *sequence) {
        if (node->type() != xml::Node::DOM_ELEMENT)
            throw Exception("The node " + node->name() + " is not an XML element");
        auto* element = dynamic_cast<xml::Element*>(node);
        string elementName = element->name();
        if (elementName == "xsd:element")
            m_sequence.push_back(make_shared<WSParserComplexType>(element));
    }
}

void WSParserComplexType::parse()
{
    m_attributes.clear();
    if (m_element == nullptr)
        return;
    for (auto* node: *m_element) {
        if (node->type() != xml::Node::DOM_ELEMENT)
            throw Exception("The node " + node->name() + " is not an XML element");
        const auto* element = dynamic_cast<xml::Element*>(node);
        if (element->name() == "xsd:attribute") {
            String attrName = (String) element->getAttribute("name");
            m_attributes[attrName] = new WSParserAttribute(attrName, (String) element->getAttribute("type"));
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

void WSParserComplexType::printDeclarationIncludes(ostream& classDeclaration, const set<String>& usedClasses) const
{
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
}

void WSParserComplexType::generateDefinition(std::ostream& classDeclaration, sptk::Strings& fieldNames,
                                             sptk::Strings& elementNames,
                                             const String& serviceNamespace) const
{
    String className = "C" + wsClassName(m_name);

    String defineName = "__" + serviceNamespace.toUpperCase() + "_" + className.toUpperCase() + "__";
    classDeclaration << "#ifndef " << defineName << endl;
    classDeclaration << "#define " << defineName << endl;
    classDeclaration << endl;

    auto usedClasses = getUsedClasses();

    printDeclarationIncludes(classDeclaration, usedClasses);

    String  tagName = lowerCase(className.substr(1));
    RegularExpression matchWords("([A-Z]+[a-z]+)", "g");
    auto words = matchWords.m(className.substr(1));
    if (words) {
        Strings wordList;
        for (auto& word: words.groups())
            wordList.push_back(word.value);
        tagName = wordList.join("_").toLowerCase();
    }

    classDeclaration << "namespace " << serviceNamespace << " {" << endl << endl;

    classDeclaration << "/**" << endl;
    classDeclaration << " * WSDL complex type class " << className << "." << endl;
    classDeclaration << " * Generated by wsdl2cxx SPTK utility." << endl;
    classDeclaration << " */" << endl;

    classDeclaration << "class " << className << " : public sptk::WSComplexType" << endl;
    classDeclaration << "{" << endl;

    if (!m_attributes.empty() || !m_sequence.empty())
        classDeclaration << "public:" << endl << endl;

    Strings ctorInitializer;
    Strings copyInitializer;
    Strings moveInitializer;
    ctorInitializer.push_back(string("sptk::WSComplexType(elementName, optional)"));
    copyInitializer.push_back(string("sptk::WSComplexType(other)"));
    moveInitializer.push_back(string("sptk::WSComplexType(std::move(other))"));
    if (!m_sequence.empty()) {
        classDeclaration << "   // Elements" << endl;
        for (const auto& complexType: m_sequence) {
            appendMemberDocumentation(classDeclaration, complexType);

            String cxxType = complexType->className();
            string optional = (complexType->multiplicity() & WSM_OPTIONAL) != 0 ? ", true" : ", false";
            if (complexType->isArray())
                cxxType = "sptk::WSArray<" + cxxType + ">";
            else
                fieldNames.push_back(complexType->name());

            elementNames.push_back(complexType->name());

            copyInitializer.push_back("m_" + complexType->name() + "(other.m_" + complexType->name() + ")");
            moveInitializer.push_back("m_" + complexType->name() + "(std::move(other.m_" + complexType->name() + "))");

            classDeclaration << "   " << left << setw(30) << cxxType << " m_" << complexType->name();
            if (!complexType->isArray())
                classDeclaration
                    << " {\"" << complexType->name() << "\"" << optional << "}";
            classDeclaration << ";" << endl;
        }
    }

    appendClassAttributes(classDeclaration, fieldNames, copyInitializer, moveInitializer);

    classDeclaration << endl;
    classDeclaration << "   // Field names of simple types, that can be used to build SQL queries" << endl;
    classDeclaration << "   static const sptk::Strings m_fieldNames;" << endl;
    classDeclaration << "   static const sptk::Strings m_elementNames;" << endl;
    classDeclaration << "   static const sptk::Strings m_attributeNames;" << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Constructor" << endl;
    classDeclaration << "    * @param elementName        WSDL element name" << endl;
    classDeclaration << "    * @param optional bool, Is element optional flag" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   explicit " << className << "(const char* elementName=\"" << tagName << "\", bool optional=false) noexcept" << endl
                     << "   : " << ctorInitializer.join(",\n     ") << endl << "   {}" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Load content from XML node" << endl;
    classDeclaration << "    *" << endl;
    classDeclaration << "    * Complex WSDL type members are loaded recursively." << endl;
    classDeclaration << "    * @param input              XML node containing " << className << " data" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void load(const sptk::xml::Element* input) override;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Load content from JSON element" << endl;
    classDeclaration << "    *" << endl;
    classDeclaration << "    * Complex WSDL type members are loaded recursively." << endl;
    classDeclaration << "    * @param input              JSON element containing " << className << " data" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void load(const sptk::json::Element* input) override;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Load content from FieldList" << endl;
    classDeclaration << "    *" << endl;
    classDeclaration << "    * Only simple WSDL type members are loaded." << endl;
    classDeclaration << "    * @param input              Query field list containing " << className << " data" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void load(const sptk::FieldList& input) override;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Unload content to existing XML node" << endl;
    classDeclaration << "    * @param output             Existing XML node" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void unload(sptk::xml::Element* output) const override;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Unload content to existing JSON node" << endl;
    classDeclaration << "    * @param output             Existing JSON node" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void unload(sptk::json::Element* output) const override;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Unload content to Query's parameters" << endl;
    classDeclaration << "    * @param output             Query parameters" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   void unload(sptk::QueryParameterList& output) const override;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Check if null" << endl;
    classDeclaration << "    * @return true if all elements and attributes are null" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   bool isNull() const override;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Get simple field names that can be used to build SQL queries." << endl;
    classDeclaration << "    * Return list of fields doesn't include fields of complex type." << endl;
    classDeclaration << "    * @return list of fields as string vector" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   static const sptk::Strings& fieldNames() { return m_fieldNames; }" << endl;

    classDeclaration << endl;
    classDeclaration << "protected:" << endl << endl;

    classDeclaration << "   /**" << endl
                     << "    * Clear content and release allocated memory (internal)" << endl
                     << "    */" << endl
                     << "   void _clear() override;" << endl;

    classDeclaration << "private:" << endl << endl;
    classDeclaration << "   /**" << endl
                     << "    * Check restrictions" << endl
                     << "    * Throws an exception if any restriction is violated." << endl
                     << "    */" << endl
                     << "   void checkRestrictions() const;" << endl;

    classDeclaration << "};" << endl;
    classDeclaration << endl;
    classDeclaration << "typedef std::shared_ptr<" << className << "> " << "S" << wsClassName(m_name) << ";" << endl;
    classDeclaration << endl;
    classDeclaration << "}" << endl << endl;
    classDeclaration << "#endif" << endl;
}

void WSParserComplexType::appendClassAttributes(ostream& classDeclaration, Strings& fieldNames,
                                                Strings& copyInitializer, Strings& moveInitializer) const
{
    if (!m_attributes.empty()) {
        classDeclaration << "   // Attributes" << endl;
        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *itor.second;
            classDeclaration << "   " << attr.generate(true) << ";" << endl;
            copyInitializer.push_back("m_" + attr.name() + "(other.m_" + attr.name() + ")");
            moveInitializer.push_back("m_" + attr.name() + "(std::move(other.m_" + attr.name() + "))");
            fieldNames.push_back(attr.name());
        }
    }
}

void WSParserComplexType::appendMemberDocumentation(ostream& classDeclaration,
                                                    const SWSParserComplexType& complexType) const
{
    if (!complexType->m_documentation.empty()) {
        classDeclaration << endl;
        classDeclaration << "   /**" << endl;
        Strings rows(complexType->m_documentation, "[\n\r]+", Strings::SM_REGEXP);
        for (const String& row: rows) {
            classDeclaration << "    * " << trim(row) << endl;
        }
        classDeclaration << "    */" << endl;
    }
}

set<String> WSParserComplexType::getUsedClasses() const
{
    set<String> usedClasses;
    // determine the list of used classes
    for (auto& complexType: m_sequence) {
        String cxxType = complexType->className();
        if (cxxType[0] == 'C') {
            usedClasses.insert(cxxType);
        }
    }
    return usedClasses;
}

void WSParserComplexType::printImplementationIncludes(ostream& classImplementation, const String& className) const
{
    classImplementation << "#include \"" << className << ".h\"" << endl;
    classImplementation << "#include <sptk5/json/JsonArrayData.h>" << endl << endl;
    classImplementation << "using namespace std;" << endl;
    classImplementation << "using namespace sptk;" << endl;
}

void WSParserComplexType::printImplementationClear(ostream& classImplementation, const String& className) const
{
    classImplementation << "void " << className << "::_clear()" << endl;
    classImplementation << "{" << endl;
    classImplementation << "    // Clear elements" << endl;
    for (auto& complexType: m_sequence) {
        classImplementation << "    m_" << complexType->name() << ".clear();" << endl;
    }
    if (!m_attributes.empty()) {
        classImplementation << "    // Clear attributes" << endl;
        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *(itor.second);
            classImplementation << "    m_" << attr.name() << ".setNull(VAR_NONE);" << endl;
        }
    }
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::printImplementationRestrictions(std::ostream& classImplementation, std::ostream& checks) const
{
    Strings requiredElements;
    std::size_t restrictionIndex = 0;
    for (auto& complexType: m_sequence) {
        if ((complexType->multiplicity() & WSM_REQUIRED) != 0)
            requiredElements.push_back(complexType->name());
        if (!complexType->m_typeName.startsWith("xsd:"))
            continue;
        String restrictionCheck = addOptionalRestriction(classImplementation, complexType, restrictionIndex);
        if (!restrictionCheck.empty()) {
            if (restrictionIndex == 1)
                checks << "    // Check value restrictions" << endl;
            checks << restrictionCheck << endl;
        }
    }

    if (restrictionIndex > 0) {
        classImplementation << endl;
        checks << endl;
    }

    if (!requiredElements.empty()) {
        checks << "    // Check 'required' restrictions" << endl;
        for (const auto& requiredElement : requiredElements)
            checks << "    m_" << requiredElement << ".throwIfNull(\"" << wsClassName(m_name) << "." << requiredElement << "\");" << endl;
    }
}

void WSParserComplexType::printImplementationCheckRestrictions(ostream& classImplementation,
                                                               const String& className) const
{
    classImplementation << "void " << className << "::checkRestrictions() const" << endl
                        << "{" << endl;
    stringstream checks;
    printImplementationRestrictions(classImplementation, checks);
    classImplementation << checks.str();
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::printImplementationLoadXML(ostream& classImplementation, const String& className) const
{
    ImplementationParts implementationParts;
    bool hideInputParameterName = m_attributes.empty() && m_sequence.empty();
    classImplementation << "void " << className << "::load(const xml::Element*"
                        << (hideInputParameterName? "": " input") << ")" << endl
                        << "{" << endl;

    implementationParts.body << "    _clear();" << endl
                             << "    setLoaded(true);" << endl;

    printImplementationLoadXMLAttributes(implementationParts);
    printImplementationLoadXMLFields(implementationParts);

    implementationParts.print(classImplementation);

    classImplementation << "    checkRestrictions();" << endl;
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::printImplementationLoadXMLAttributes(ImplementationParts& implementationParts) const
{
    if (!m_attributes.empty()) {
        implementationParts.body << endl << "    // Load attributes" << endl;
        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *itor.second;
            implementationParts.body << "    m_" << attr.name() << ".load((String) input->getAttribute(\"" << attr.name() << "\"));" << endl;
        }
    }
}

void WSParserComplexType::printImplementationLoadXMLFields(ImplementationParts& implementationParts) const
{
    if (!m_sequence.empty()) {
        implementationParts.body << endl << "    // Load elements" << endl;
        implementationParts.body << "    for (const auto* node: *input) {" << endl;
        implementationParts.body << "        const auto* element = dynamic_cast<const xml::Element*>(node);" << endl;
        implementationParts.body << "        if (element == nullptr) continue;" << endl;
        Strings requiredElements;
        for (auto& complexType: m_sequence) {
            implementationParts.body << endl;
            implementationParts.body << "        if (element->name() == \"" << complexType->name() << "\") {" << endl;
            if (complexType->isArray())
                implementationParts.printImplementationLoadArray(complexType);
            else
                implementationParts.printImplementationLoadField(requiredElements, complexType);
            implementationParts.body << "        }" << endl;
        }
        implementationParts.body << "    }" << endl;
    }
}

void WSParserComplexType::printImplementationLoadJSON(ostream& classImplementation, const String& className) const
{
    ImplementationParts implementationParts;
    bool hideInputParameterName = m_attributes.empty() && m_sequence.empty();
    classImplementation << "void " << className << "::load(const json::Element*"
                        << (hideInputParameterName? "": " input") << ")" << endl
                        << "{" << endl;

    implementationParts.body
                        << "    _clear();" << endl
                        << "    setLoaded(true);" << endl
                        << "    if (!input->is(json::JDT_OBJECT))" << endl
                        << "        return;" << endl;

    printImplementationLoadJSONAttributes(implementationParts);

    if (!m_sequence.empty()) {
        implementationParts.body << endl << "    // Load elements" << endl;
        implementationParts.body << "    for (auto& itor: input->getObject()) {" << endl;
        implementationParts.body << "        const auto& elementName = itor.name();" << endl;
        implementationParts.body << "        const auto* element = itor.element();" << endl;
        Strings requiredElements;
        for (auto& complexType: m_sequence) {
            implementationParts.body << endl;
            implementationParts.body << "        if (elementName == \"" << complexType->name() << "\") {" << endl;
            if (complexType->isArray()) {
                implementationParts.body << "            for (const auto* arrayElement: element->getArray()) {" << endl;
                implementationParts.body << "                " << complexType->className() << " item(\"" << complexType->name() << "\", false);" << endl;
                implementationParts.body << "                item.load(arrayElement);" << endl;
                implementationParts.body << "                m_" << complexType->name() << ".push_back(move(item));" << endl;
                implementationParts.body << "            }" << endl;
            }
            else {
                implementationParts.printImplementationLoadField(requiredElements, complexType);
            }
            implementationParts.body << "        }" << endl;
        }
        implementationParts.body << "    }" << endl;
    }

    implementationParts.print(classImplementation);

    classImplementation << "    checkRestrictions();" << endl;
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::printImplementationLoadJSONAttributes(ImplementationParts& implementationParts) const
{
    if (!m_attributes.empty()) {
        implementationParts.body << endl << "    // Load attributes" << endl;
        implementationParts.body << "    auto* attributes = input->find(\"attributes\");" << endl;
        implementationParts.body << "    if (attributes != nullptr) {" << endl;
        implementationParts.body << "        const json::Element* attribute;" << endl;
        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *itor.second;
            implementationParts.body << "        attribute = attributes->find(\"" << attr.name() << "\");" << endl;
            implementationParts.body << "        if (attribute != nullptr)" << endl;
            implementationParts.body << "            m_" << attr.name() << ".load(attribute);" << endl;
        }
        implementationParts.body << endl << "    }" << endl;
    }
}

void WSParserComplexType::printImplementationLoadFields(ostream& classImplementation, const String& className) const
{
    Strings             requiredElements;
    stringstream        fieldLoads;
    stringstream        checks;
    int                 fieldLoadCount = 0;

    makeImplementationLoadAttributes(fieldLoads, fieldLoadCount);
    makeImplementationLoadFields(fieldLoads, fieldLoadCount, requiredElements);

    bool hideInputParameter = fieldLoadCount == 0;
    classImplementation << "void " << className << "::load(const FieldList&"
                        << (hideInputParameter? "" : " input") << ")" << endl
                        << "{" << endl;

    classImplementation << "    _clear();" << endl
                        << "    setLoaded(true);" << endl;

    if (fieldLoadCount != 0) {
        classImplementation << "    const Field* field;" << endl;
        classImplementation << fieldLoads.str();
    }

    classImplementation << checks.str();

    classImplementation << "    checkRestrictions();" << endl;
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::makeImplementationLoadFields(stringstream& fieldLoads, int& fieldLoadCount,
                                                       Strings& requiredElements) const
{
    RegularExpression matchStandardType("^xsd:");
    size_t            restrictionIndex = 0;

    if (!m_sequence.empty()) {
        fieldLoads << endl << "    // Load elements" << endl;
        for (auto& complexType: m_sequence) {
            if ((complexType->multiplicity() & WSM_REQUIRED) != 0)
                requiredElements.push_back(complexType->name());
            if (!matchStandardType.matches(complexType->m_typeName))
                continue;
            ++fieldLoadCount;
            fieldLoads << "    if ((field = input.findField(\"" << complexType->name() << "\")) != nullptr) {" << endl;
            String restrictionCheck = addOptionalRestriction(fieldLoads, complexType, restrictionIndex);

            if (complexType->isArray()) {
                fieldLoads << "        " << complexType->className() << " item(\"" << complexType->name() << "\", false);" << endl;
                fieldLoads << "        item.load(*field);" << endl;
                fieldLoads << "        m_" << complexType->name() << ".push_back(move(item));" << endl;
            }
            else {
                fieldLoads << "        m_" << complexType->name() << ".load(*field);" << endl;
            }

            if (restrictionCheck.empty())
                fieldLoads << restrictionCheck << endl;

            fieldLoads << "    }" << endl << endl;
        }
    }
}

String WSParserComplexType::addOptionalRestriction(std::ostream& implementation,
                                                   const SWSParserComplexType& complexType,
                                                   size_t& restrictionIndex) const
{
    String restrictionCheck;
    if (complexType->m_restriction != nullptr) {
        ++restrictionIndex;
        String restrictionName = "restriction_" + int2string(restrictionIndex);
        auto restrictionCtor = complexType->m_restriction->generateConstructor(restrictionName);
        if (!restrictionCtor.empty()) {
            if (restrictionIndex == 1)
                implementation << "    // Restrictions" << endl;
            implementation << "    static const " << restrictionCtor << ";" << endl;
            if (complexType->isArray())
                restrictionCheck = "    for (const auto& item: m_" + complexType->name() + ")\n" +
                                   "        " + restrictionName + ".check(\"" + wsClassName(m_name) + "." + complexType->name() + "\", item.asString());";
            else
                restrictionCheck = "    " + restrictionName + ".check(\"" + wsClassName(m_name) + "." + complexType->name() + "\", m_" + complexType->name() + ".asString());";
        }
    }
    return restrictionCheck;
}

void WSParserComplexType::makeImplementationLoadAttributes(stringstream& fieldLoads, int& fieldLoadCount) const
{
    if (!m_attributes.empty()) {
        fieldLoads << endl << "    // Load attributes" << endl;
        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *itor.second;
            fieldLoads << "    if ((field = input.findField(\"" << attr.name() << "\")) != nullptr) {" << endl;
            fieldLoads << "        m_" << attr.name() << ".load(*field);" << endl;
            fieldLoads << "    }" << endl << endl;
            ++fieldLoadCount;
        }
    }
}

void WSParserComplexType::printImplementationUnloadXML(ostream& classImplementation, const String& className) const
{
    bool hideOutputParameterName = m_attributes.empty() && m_sequence.empty();
    classImplementation << "void " << className << "::unload(xml::Element*"
                        << (hideOutputParameterName? "": " output") << ") const" << endl
                        << "{" << endl;

    if (!m_attributes.empty()) {
        classImplementation << endl << "    // Unload attributes" << endl;
        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *itor.second;
            classImplementation << "    output->setAttribute(\"" << attr.name() << "\", m_" << attr.name() << ".asString());" << endl;
        }
    }

    if (!m_sequence.empty()) {
        classImplementation << endl << "    // Unload elements" << endl;
        for (auto& complexType: m_sequence) {
            if ((complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) != 0) {
                classImplementation << "    for (auto& element: m_" << complexType->name() << ")" << endl;
                classImplementation << "        element.addElement(output, \"" << complexType->name() <<  "\");" << endl;
            }
            else
                classImplementation << "    m_" << complexType->name() << ".addElement(output);" << endl;
        }
    }
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::printImplementationUnloadJSON(ostream& classImplementation, const String& className) const
{
    bool hideOutputParameterName = m_attributes.empty() && m_sequence.empty();
    classImplementation << "void " << className << "::unload(json::Element*"
                        << (hideOutputParameterName? "": " output") << ") const" << endl
                        << "{" << endl;

    if (!m_attributes.empty()) {
        classImplementation << endl << "    // Unload attributes" << endl;

        classImplementation << "    auto* attributes = output->add_object(\"attributes\");" << endl;

        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *itor.second;

            String attributeOutputMethod = ".asString()";
            if (attr.wsTypeName() == "xsd:boolean")
                attributeOutputMethod = ".asBool()";
            else if (attr.wsTypeName() == "xsd:double" || attr.wsTypeName() == "xsd:float")
                attributeOutputMethod = ".asFloat()";
            else if (attr.wsTypeName() == "xsd:int")
                attributeOutputMethod = ".asInt64()";

            classImplementation << "    attributes->set(\"" << attr.name() << "\", m_" << attr.name() << attributeOutputMethod << ");" << endl;
        }
    }

    if (!m_sequence.empty()) {
        classImplementation << endl << "    // Unload elements" << endl;
        for (auto& complexType: m_sequence) {
            if ((complexType->multiplicity() & (WSM_ZERO_OR_MORE | WSM_ONE_OR_MORE)) != 0) {
                String outputArrayName = complexType->name() + "_array";
                classImplementation
                    << "    auto " << outputArrayName << " = output->add_array(\"" << complexType->name() << "\");" << endl
                    << "    for (auto& element: m_" << complexType->name() << ")" << endl
                    << "        element.addElement(" << outputArrayName << ");" << endl;
            }
            else
                classImplementation << "    m_" << complexType->name() << ".addElement(output);" << endl;
        }
    }
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::printImplementationIsNull(ostream& classImplementation, const String& className) const
{
    classImplementation << "bool " << className << "::isNull() const" << endl
                        << "{" << endl;

    String indent = "\n        ";
    if (!m_attributes.empty()) {
        classImplementation << "    // Check attributes" << endl;
        classImplementation << "    bool attributesAreNull = ";

        Strings attributeChecks;
        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *itor.second;
            attributeChecks.push_back("m_" + attr.name() + ".isNull()");
        }
        classImplementation << indent << attributeChecks.join(indent + "&& ") << ";";
        classImplementation << "    if (!attributesAreNull)" << endl
                            << "        return false;" << endl;
        classImplementation << endl;
    }

    if (!m_sequence.empty()) {
        classImplementation << "    // Check elements" << endl;
        classImplementation << "    bool elementsAreNull = ";
        Strings elementChecks;
        for (auto& complexType: m_sequence) {
            elementChecks.push_back("m_" + complexType->name() + ".isNull()");
        }
        classImplementation << indent << elementChecks.join(indent + "&& ") << ";" << endl << endl;
    }
    classImplementation << "    return elementsAreNull;" << endl;
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::printImplementationUnloadParamList(ostream& classImplementation, const String& className) const
{
    stringstream    unloadList;
    size_t          unloadListCount = 0;

    if (!m_attributes.empty()) {
        unloadList << endl << "    // Unload attributes" << endl;
        for (auto& itor: m_attributes) {
            const WSParserAttribute& attr = *itor.second;
            unloadList << "    WSComplexType::unload(output, \"" << attr.name() << "\", &m_" << attr.name() << ");" << endl;
            ++unloadListCount;
        }
    }

    if (!m_sequence.empty()) {
        unloadList << endl << "    // Unload attributes" << endl;
        for (auto& complexType: m_sequence) {
            if (!complexType->isArray()) {
                unloadList << "    WSComplexType::unload(output, \"" << complexType->name()
                           << "\", dynamic_cast<const WSBasicType*>(&m_" << complexType->name() << "));" << endl;
                ++unloadListCount;
            }
        }
    }

    bool hideOutputParameterName = unloadListCount == 0;
    classImplementation << "void " << className << "::unload(QueryParameterList&"
                        << (hideOutputParameterName? "": " output") << ") const" << endl
                        << "{" << endl;
    classImplementation << unloadList.str();
    classImplementation << "}" << endl;
}

void WSParserComplexType::generateImplementation(std::ostream& classImplementation, const Strings& fieldNames,
                                                 const Strings& elementNames,
                                                 const Strings& attributeNames, const String& serviceNamespace) const
{
    String className = "C" + wsClassName(m_name);

    printImplementationIncludes(classImplementation, className);

    classImplementation << "using namespace " << serviceNamespace << ";" << endl << endl;

    classImplementation << "const Strings " << className << "::m_fieldNames { \"" << fieldNames.join("\", \"")
                        << "\" };" << endl;
    classImplementation << "const Strings " << className << "::m_elementNames { \"" << elementNames.join("\", \"")
                        << "\" };" << endl;
    classImplementation << "const Strings " << className << "::m_attributeNames { \"" << attributeNames.join("\", \"")
                        << "\" };" << endl << endl;

    printImplementationCheckRestrictions(classImplementation, className);
    printImplementationClear(classImplementation, className);
    printImplementationLoadXML(classImplementation, className);
    printImplementationLoadJSON(classImplementation, className);

    RegularExpression matchStandardType("^xsd:");
    printImplementationLoadFields(classImplementation, className);

    printImplementationUnloadXML(classImplementation, className);
    printImplementationUnloadJSON(classImplementation, className);
    printImplementationIsNull(classImplementation, className);
    printImplementationUnloadParamList(classImplementation, className);
}

void WSParserComplexType::generate(ostream& classDeclaration, ostream& classImplementation,
                                   const String& externalHeader, const String& serviceNamespace) const
{
    if (!externalHeader.empty()) {
        classDeclaration << externalHeader << endl;
        classImplementation << externalHeader << endl;
    }

    Strings fieldNames;
    Strings elementNames;
    Strings attributeNames;
    generateDefinition(classDeclaration, fieldNames, elementNames, serviceNamespace);
    generateImplementation(classImplementation, fieldNames, elementNames, attributeNames, serviceNamespace);
}

const xml::Element* WSParserComplexType::findSimpleType(const String& typeName)
{
    auto itor = SimpleTypeElements.find(typeName);
    if (itor == SimpleTypeElements.end())
        return nullptr;
    return itor->second;
}

void WSParserComplexType::ImplementationParts::print(ostream& output) const
{
    if (!declarations.str().empty())
        output << declarations.str() << endl;
    output << body.str();
    if (!checks.str().empty())
        output << endl << checks.str();
}

void WSParserComplexType::ImplementationParts::printImplementationLoadArray(const SWSParserComplexType& complexType)
{
    body << "            " << complexType->className() << " item(\"" << complexType->name() << "\", false);" << endl;
    body << "            item.load(element);" << endl;
    body << "            m_" << complexType->name() << ".push_back(move(item));" << endl;
}

sptk::String WSParserComplexType::ImplementationParts::appendRestrictionIfDefined(const SWSParserComplexType& complexType)
{
    String restrictionName;
    if (complexType->m_restriction != nullptr) {
        restrictionName = "restriction_" + to_string(restrictionNumber);
        auto restrictionCtor = complexType->m_restriction->generateConstructor(restrictionName);
        if (!restrictionCtor.empty()) {
            ++restrictionNumber;
            declarations << "    static const " << restrictionCtor << ";" << endl;
        } else
            restrictionName = "";
    }
    return restrictionName;
}

void WSParserComplexType::ImplementationParts::printImplementationLoadField(Strings& requiredElements,
                                                                            const SWSParserComplexType& complexType)
{
    body << "            m_" << complexType->name() << ".load(element);" << endl;
    body << "            continue;" << endl;
    if ((complexType->multiplicity() & WSM_REQUIRED) != 0)
        requiredElements.push_back(complexType->name());
}
