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

#include <sptk5/RegularExpression.h>
#include <sptk5/wsdl/WSParserComplexType.h>
#include <sptk5/wsdl/WSTypeTranslator.h>
#include <iomanip>

using namespace std;
using namespace sptk;

std::map<String, xdoc::Node*> WSParserComplexType::SimpleTypeElements;

WSParserAttribute::WSParserAttribute(const String& name, const String& typeName)
    : m_name(name),
      m_wsTypeName(typeName)
{
    m_cxxTypeName = wsTypeTranslator.toCxxType(typeName);
}

String WSParserAttribute::generate(bool initialize) const
{
    constexpr int fieldNameWidth = 40;
    stringstream str;
    str << left << setw(fieldNameWidth) << m_cxxTypeName << " m_" << m_name;
    if (initialize)
    {
        str << " {\"" << m_name << "\", true}";
    }
    return str.str();
}

WSParserComplexType::WSParserComplexType(xdoc::Node* complexTypeElement, const String& name,
                                         const String& typeName)
    : m_name(name.empty() ? (String) complexTypeElement->getAttribute("name") : name),
      m_typeName(typeName.empty() ? (String) complexTypeElement->getAttribute("type") : typeName),
      m_element(complexTypeElement)
{
    xdoc::Node* simpleTypeElement = nullptr;
    if (!m_typeName.empty())
    {
        simpleTypeElement = findSimpleType(m_typeName);
    }
    else if (complexTypeElement->name() == "xsd:element")
    {
        simpleTypeElement = m_element;
    }

    if (simpleTypeElement)
    {
        auto* restrictionElement = simpleTypeElement->findFirst("xsd:restriction",
                                                                xdoc::Node::SearchMode::Recursive);
        if (restrictionElement != nullptr)
        {
            m_typeName = restrictionElement->getAttribute("base");
            m_restriction = make_shared<WSRestriction>(m_typeName, restrictionElement->parent());
        }
    }

    if (const xdoc::Node* documentationElement = complexTypeElement->findFirst("xsd:documentation");
        documentationElement != nullptr)
    {
        m_documentation = documentationElement->text().trim();
    }

    if (m_typeName.empty())
    {
        m_typeName = m_name;
    }

    String maxOccurs;
    String minOccurs;
    if (complexTypeElement->hasAttribute("maxOccurs"))
    {
        maxOccurs = (String) complexTypeElement->getAttribute("maxOccurs");
    }
    if (complexTypeElement->hasAttribute("minOccurs"))
    {
        minOccurs = (String) complexTypeElement->getAttribute("minOccurs");
    }

    m_multiplicity = WSMultiplicity::REQUIRED;

    // Relaxed defaults, in case of incomplete or missing multiplicity
    if (minOccurs.empty())
    {
        minOccurs = "1";
    }
    if (maxOccurs.empty())
    {
        maxOccurs = "1";
    }

    if (minOccurs == "0")
    {
        m_multiplicity = maxOccurs == "1" ? WSMultiplicity::ZERO_OR_ONE : WSMultiplicity::ZERO_OR_MORE;
    }
    else if (minOccurs == "1")
    {
        m_multiplicity = maxOccurs == "1" ? WSMultiplicity::REQUIRED : WSMultiplicity::ONE_OR_MORE;
    }
}

String WSParserComplexType::className() const
{
    if (String cxxType = wsTypeTranslator.toCxxType(m_typeName, "");
        !cxxType.empty())
    {
        return cxxType;
    }
    size_t pos = m_typeName.find(':');
    return "C" + m_typeName.substr(pos + 1);
}

void WSParserComplexType::parseSequence(xdoc::Node* sequence)
{
    for (auto& node: *sequence)
    {
        if (node.name() == "xsd:element")
        {
            m_sequence.push_back(make_shared<WSParserComplexType>(&node));
        }
    }
}

void WSParserComplexType::parse()
{
    m_attributes.clear();
    if (m_element == nullptr)
    {
        return;
    }

    for (auto& node: *m_element)
    {
        if (node.name() == "xsd:attribute")
        {
            auto attrName = node.getAttribute("name");
            m_attributes[attrName] = new WSParserAttribute(attrName, node.getAttribute("type"));
        }
        else if (node.name() == "xsd:sequence")
        {
            parseSequence(&node);
        }
    }
}

String WSParserComplexType::wsClassName(const String& name)
{
    return name;
}

void WSParserComplexType::printDeclarationIncludes(ostream& classDeclaration, const set<String>& usedClasses)
{
    Strings includeFiles;
    includeFiles.push_back("#include <sptk5/sptk.h>");
    includeFiles.push_back("#include <sptk5/FieldList.h>");
    includeFiles.push_back("#include <sptk5/threads/Locks.h>");
    includeFiles.push_back("#include <sptk5/db/QueryParameterList.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSBasicTypes.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSComplexType.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSRestriction.h>");

    for (const auto& usedClass: usedClasses)
    {
        includeFiles.push_back("#include \"" + usedClass + ".h\"");
    }

    includeFiles.sort();
    classDeclaration << includeFiles.join("\n") << endl << endl;
}

WSParserComplexType::Initializer WSParserComplexType::makeInitializer() const
{
    Initializer initializer;
    if (!m_sequence.empty())
    {
        for (const auto& complexType: m_sequence)
        {
            initializer.copyCtor.push_back("m_" + complexType->name() + "(other.m_" + complexType->name() + ")");
            initializer.moveCtor.push_back(
                "m_" + complexType->name() + "(std::move(other.m_" + complexType->name() + "))");
            initializer.copyAssign.push_back("m_" + complexType->name() + " = other.m_" + complexType->name());
            initializer.moveAssign.push_back(
                "m_" + complexType->name() + " = std::move(other.m_" + complexType->name() + ")");
        }
    }

    return initializer;
}

void WSParserComplexType::generateDefinition(std::ostream& classDeclaration, sptk::Strings& fieldNames,
                                             sptk::Strings& elementNames, sptk::Strings& attributeNames,
                                             const String& serviceNamespace) const
{
    constexpr int fieldNameWidth = 40;
    String className = "C" + wsClassName(m_name);

    classDeclaration << "#pragma once" << endl;
    classDeclaration << endl;

    auto usedClasses = getUsedClasses();

    printDeclarationIncludes(classDeclaration, usedClasses);

    String tagName = makeTagName(className);

    classDeclaration << "namespace " << serviceNamespace << " {" << endl << endl;

    classDeclaration << "/**" << endl;
    classDeclaration << " * WSDL complex type class " << className << "." << endl;
    classDeclaration << " * Generated by wsdl2cxx SPTK utility." << endl;
    classDeclaration << " */" << endl;

    classDeclaration << "class " << className << " : public sptk::WSComplexType" << endl;
    classDeclaration << "{" << endl;

    if (!m_attributes.empty() || !m_sequence.empty())
    {
        classDeclaration << "public:" << endl << endl;
    }

    for (const auto&[name, value]: m_attributes)
    {
        attributeNames.push_back(name);
    }

    Initializer initializer = makeInitializer();
    if (!m_sequence.empty())
    {
        classDeclaration << "   // Elements" << endl;
        for (const auto& complexType: m_sequence)
        {
            appendMemberDocumentation(classDeclaration, complexType);

            String cxxType = complexType->className();
            string optional =
                ((int) complexType->multiplicity() & (int) WSMultiplicity::ZERO_OR_ONE) != 0 ? ", true" : ", false";
            if (complexType->isArray())
            {
                cxxType = "sptk::WSArray<" + cxxType + ">";
            }

            fieldNames.push_back(complexType->name());
            elementNames.push_back(complexType->name());

            classDeclaration << "   " << left << setw(fieldNameWidth) << cxxType << " m_" << complexType->name();
            if (complexType->isArray())
            {
                classDeclaration << " {\"" << complexType->name() << "\"}";
            }
            else
            {
                classDeclaration << " {\"" << complexType->name() << "\"" << optional << "}";
            }
            classDeclaration << ";" << endl;
        }
    }

    appendClassAttributes(classDeclaration, fieldNames, initializer);

    classDeclaration << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Constructor" << endl;
    classDeclaration << "    * @param elementName        WSDL element name" << endl;
    classDeclaration << "    * @param optional           Is element optional flag" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   explicit " << className << "(const char* elementName=\"" << tagName
                     << "\", bool optional=false) noexcept;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Copy constructor" << endl;
    classDeclaration << "    * @param other              Other object" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   " << className << "(const " << className << "& other);" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Move constructor" << endl;
    classDeclaration << "    * @param other              Other object" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   " << className << "(" << className << "&& other) noexcept;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Destructor" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   ~" << className << "() override = default;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Copy assignment" << endl;
    classDeclaration << "    * @param other              Other object" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   " << className << "& operator = (const " << className << "& other);" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Move assignment" << endl;
    classDeclaration << "    * @param other              Other object" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   " << className << "& operator = (" << className << "&& other) noexcept;" << endl << endl;

    classDeclaration << "   /**" << endl;
    classDeclaration << "    * Get complex type field names." << endl;
    classDeclaration << "    * @param group              Field group: elements, attributes, or both" << endl;
    classDeclaration << "    * @return list of fields as Strings" << endl;
    classDeclaration << "    */" << endl;
    classDeclaration << "   static const sptk::Strings& fieldNames(sptk::WSFieldIndex::Group group);" << endl;

    classDeclaration << endl;

    classDeclaration << "private:" << endl << endl;
    classDeclaration << "   /**" << endl
                     << "    * Check restrictions" << endl
                     << "    * Throws an exception if any restriction is violated." << endl
                     << "    */" << endl
                     << "   void checkRestrictions() const override;" << endl;

    classDeclaration << "};" << endl;
    classDeclaration << endl;
    classDeclaration << "}" << endl;
}

String WSParserComplexType::makeTagName(const String& className)
{
    String tagName = lowerCase(className.substr(1));
    RegularExpression matchWords("([A-Z]+[a-z]+)", "g");
    if (auto words = matchWords.m(className.substr(1));
        words)
    {
        Strings wordList;
        for (const auto& word: words.groups())
        {
            wordList.push_back(word.value);
        }
        tagName = wordList.join("_").toLowerCase();
    }
    return tagName;
}

void WSParserComplexType::generateSetFieldIndex(ostream& classDeclaration, const Strings& elementNames,
                                                const Strings& attributeNames)
{
    if (!elementNames.empty())
    {
        classDeclaration << "    WSComplexType::setElements(fieldNames(WSFieldIndex::Group::ELEMENTS), {&m_"
                         << elementNames.join(", &m_") << "});" << endl;
    }
    if (!attributeNames.empty())
    {
        classDeclaration << "    WSComplexType::setAttributes(fieldNames(WSFieldIndex::Group::ATTRIBUTES), {&m_"
                         << attributeNames.join(", &m_") << "});" << endl;
    }
}

void WSParserComplexType::appendClassAttributes(ostream& classDeclaration, Strings& fieldNames,
                                                Initializer& initializer) const
{
    if (!m_attributes.empty())
    {
        classDeclaration << "   // Attributes" << endl;
        for (const auto&[name, attr]: m_attributes)
        {
            classDeclaration << "   " << attr->generate(true) << ";" << endl;
            initializer.copyCtor.push_back("m_" + attr->name() + "(other.m_" + attr->name() + ")");
            initializer.moveCtor.push_back("m_" + attr->name() + "(std::move(other.m_" + attr->name() + "))");
            fieldNames.push_back(attr->name());
        }
    }
}

void WSParserComplexType::appendMemberDocumentation(ostream& classDeclaration,
                                                    const SWSParserComplexType& complexType)
{
    if (!complexType->m_documentation.empty())
    {
        classDeclaration << endl;
        classDeclaration << "   /**" << endl;
        Strings rows(complexType->m_documentation, "[\n\r]+", Strings::SplitMode::REGEXP);
        for (const String& row: rows)
        {
            classDeclaration << "    * " << trim(row) << endl;
        }
        classDeclaration << "    */" << endl;
    }
}

set<String> WSParserComplexType::getUsedClasses() const
{
    set<String> usedClasses;
    // determine the list of used classes
    for (const auto& complexType: m_sequence)
    {
        String cxxType = complexType->className();
        if (cxxType[0] == 'C')
        {
            usedClasses.insert(cxxType);
        }
    }
    return usedClasses;
}

void WSParserComplexType::printImplementationIncludes(ostream& classImplementation, const String& className)
{
    classImplementation << "#include \"" << className << ".h\"" << endl;
    classImplementation << "#include <sptk5/json/JsonArrayData.h>" << endl << endl;
    classImplementation << "using namespace std;" << endl;
    classImplementation << "using namespace sptk;" << endl;
}

void WSParserComplexType::printImplementationRestrictions(std::ostream& classImplementation, std::ostream& checks) const
{
    Strings requiredElements;
    std::size_t restrictionIndex = 0;
    for (const auto& complexType: m_sequence)
    {
        if (((int) complexType->multiplicity() & (int) WSMultiplicity::REQUIRED) != 0)
        {
            requiredElements.push_back(complexType->name());
        }
        if (!complexType->m_typeName.startsWith("xsd:"))
        {
            continue;
        }
        String restrictionCheck = addOptionalRestriction(classImplementation, complexType, restrictionIndex);
        if (!restrictionCheck.empty())
        {
            if (restrictionIndex == 1)
            {
                checks << "    // Check value restrictions" << endl;
            }
            checks << restrictionCheck << endl;
        }
    }

    if (restrictionIndex > 0)
    {
        classImplementation << endl;
        checks << endl;
    }

    if (!requiredElements.empty())
    {
        checks << "    // Check 'required' restrictions" << endl;
        for (const auto& requiredElement : requiredElements)
        {
            checks << "    m_" << requiredElement << ".throwIfNull(\"" << wsClassName(m_name) << "." << requiredElement
                   << "\");" << endl;
        }
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

String WSParserComplexType::addOptionalRestriction(std::ostream& implementation,
                                                   const SWSParserComplexType& complexType,
                                                   size_t& restrictionIndex) const
{
    String restrictionCheck;
    if (complexType->m_restriction != nullptr)
    {
        ++restrictionIndex;
        String restrictionName = "restriction_" + int2string(restrictionIndex);
        auto restrictionCtor = complexType->m_restriction->generateConstructor(restrictionName);
        if (!restrictionCtor.empty())
        {
            if (restrictionIndex == 1)
            {
                implementation << "    // Restrictions" << endl;
            }
            implementation << "    static const " << restrictionCtor << ";" << endl;
            if (complexType->isArray())
            {
                restrictionCheck = "    for (const auto& item: m_" + complexType->name() + ")\n" +
                                   "        " + restrictionName + ".check(\"" + wsClassName(m_name) + "." +
                                   complexType->name() + "\", item.asString());";
            }
            else
            {
                restrictionCheck =
                    "    " + restrictionName + ".check(\"" + wsClassName(m_name) + "." + complexType->name() +
                    "\", m_" + complexType->name() + ".asString());";
            }
        }
    }
    return restrictionCheck;
}

String WSParserComplexType::jsonAttributeOutputMethod(const String& wsTypeName)
{
    String attributeOutputMethod = ".asString()";
    if (wsTypeName == "xsd:boolean")
    {
        attributeOutputMethod = ".asBool()";
    }
    else if (wsTypeName == "xsd:double" || wsTypeName == "xsd:float")
    {
        attributeOutputMethod = ".asFloat()";
    }
    else if (wsTypeName == "xsd:int")
    {
        attributeOutputMethod = ".asInt64()";
    }
    return attributeOutputMethod;
}

void WSParserComplexType::generateImplementation(std::ostream& classImplementation, const Strings& fieldNames,
                                                 const Strings& elementNames,
                                                 const Strings& attributeNames, const String& serviceNamespace) const
{
    String className = "C" + wsClassName(m_name);

    printImplementationIncludes(classImplementation, className);

    classImplementation << "using namespace " << serviceNamespace << ";" << endl << endl;

    classImplementation << "const sptk::Strings& " << className << "::fieldNames(WSFieldIndex::Group group)" << endl;
    classImplementation << "{" << endl;
    classImplementation << "    static const Strings _fieldNames { \"" << fieldNames.join("\", \"") << "\" };" << endl;
    classImplementation << "    static const Strings _elementNames { \"" << elementNames.join("\", \"") << "\" };"
                        << endl;
    classImplementation << "    static const Strings _attributeNames { \"" << attributeNames.join("\", \"") << "\" };"
                        << endl << endl;
    classImplementation << "    switch (group) {" << endl;
    classImplementation << "        case WSFieldIndex::Group::ELEMENTS: return _elementNames;" << endl;
    classImplementation << "        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;" << endl;
    classImplementation << "        default: break;" << endl;
    classImplementation << "    }" << endl << endl;
    classImplementation << "    return _fieldNames;" << endl;
    classImplementation << "}" << endl << endl;

    printImplementationConstructors(classImplementation, className, elementNames, attributeNames);
    printImplementationAssignments(classImplementation, className);

    printImplementationCheckRestrictions(classImplementation, className);

    RegularExpression matchStandardType("^xsd:");
}

void WSParserComplexType::printImplementationConstructors(ostream& classImplementation, const String& className,
                                                          const Strings& elementNames,
                                                          const Strings& attributeNames) const
{
    auto tagName = makeTagName(className);
    auto initializer = makeInitializer();

    classImplementation << className << "::" << className << "(const char* elementName, bool optional) noexcept" << endl
                        << ": " << initializer.ctor.join(",\n  ") << endl
                        << "{" << endl;
    generateSetFieldIndex(classImplementation, elementNames, attributeNames);
    classImplementation << "}" << endl << endl;

    classImplementation << className << "::" << className << "(const " << className << "& other)" << endl
                        << ": " << initializer.copyCtor.join(",\n  ") << endl
                        << "{" << endl;
    generateSetFieldIndex(classImplementation, elementNames, attributeNames);
    classImplementation << "}" << endl << endl;

    classImplementation << className << "::" << "" << className << "(" << className << "&& other) noexcept" << endl
                        << ": " << initializer.moveCtor.join(",\n  ") << endl
                        << "{" << endl;
    generateSetFieldIndex(classImplementation, elementNames, attributeNames);
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::printImplementationAssignments(ostream& classImplementation, const String& className) const
{
    auto initializer = makeInitializer();

    classImplementation << className << "& " << className << "::operator = (const " << className << "& other)" << endl;
    classImplementation << "{" << endl;
    classImplementation << "    " << initializer.copyAssign.join(";\n    ") << ";" << endl;
    classImplementation << "    return *this;" << endl;
    classImplementation << "}" << endl << endl;

    classImplementation << className << "& " << className << "::operator = (" << className << "&& other) noexcept"
                        << endl;
    classImplementation << "{" << endl;
    classImplementation << "    " << initializer.moveAssign.join(";\n    ") << ";" << endl;
    classImplementation << "    return *this;" << endl;
    classImplementation << "}" << endl << endl;
}

void WSParserComplexType::generate(ostream& classDeclaration, ostream& classImplementation,
                                   const String& externalHeader, const String& serviceNamespace) const
{
    if (!externalHeader.empty())
    {
        classDeclaration << externalHeader << endl;
        classImplementation << externalHeader << endl;
    }

    Strings fieldNames;
    Strings elementNames;
    Strings attributeNames;
    generateDefinition(classDeclaration, fieldNames, elementNames, attributeNames, serviceNamespace);
    generateImplementation(classImplementation, fieldNames, elementNames, attributeNames, serviceNamespace);
}

xdoc::Node* WSParserComplexType::findSimpleType(const String& typeName)
{
    auto itor = SimpleTypeElements.find(typeName);
    if (itor == SimpleTypeElements.end())
    {
        return nullptr;
    }
    return itor->second;
}

void WSParserComplexType::ImplementationParts::print(ostream& output) const
{
    if (!declarations.str().empty())
    {
        output << declarations.str() << endl;
    }
    output << body.str();
    if (!checks.str().empty())
    {
        output << endl << checks.str();
    }
}

void WSParserComplexType::ImplementationParts::printImplementationLoadArray(const SWSParserComplexType& complexType)
{
    body << "            " << complexType->className() << " item(\"" << complexType->name() << "\", false);" << endl;
    body << "            item.load(element);" << endl;
    body << "            m_" << complexType->name() << ".push_back(move(item));" << endl;
}

sptk::String WSParserComplexType::ImplementationParts::appendRestrictionIfDefined(
    const SWSParserComplexType& complexType)
{
    String restrictionName;
    if (complexType->m_restriction != nullptr)
    {
        restrictionName = "restriction_" + to_string(restrictionNumber);
        auto restrictionCtor = complexType->m_restriction->generateConstructor(restrictionName);
        if (!restrictionCtor.empty())
        {
            ++restrictionNumber;
            declarations << "    static const " << restrictionCtor << ";" << endl;
        }
        else
        {
            restrictionName = "";
        }
    }
    return restrictionName;
}

void WSParserComplexType::ImplementationParts::printImplementationLoadField(Strings& requiredElements,
                                                                            const SWSParserComplexType& complexType)
{
    body << "            m_" << complexType->name() << ".load(element);" << endl;
    body << "            continue;" << endl;
    if (((int) complexType->multiplicity() & (int) WSMultiplicity::REQUIRED) != 0)
    {
        requiredElements.push_back(complexType->name());
    }
}
