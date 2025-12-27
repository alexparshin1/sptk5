/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/RegularExpression.h>
#include <sptk5/wsdl/WSParserComplexType.h>
#include <sptk5/wsdl/WSTypeTranslator.h>
#include <utility>

using namespace std;
using namespace sptk;

std::map<String, xdoc::SNode> WSParserComplexType::SimpleTypeElements;

WSParserAttribute::WSParserAttribute(String name, const String& typeName)
    : m_name(std::move(name))
    , m_wsTypeName(typeName)
{
    m_cxxTypeName = WSTypeTranslator::toCxxType(typeName);
}

String WSParserAttribute::generate(bool initialize) const
{
    constexpr int fieldNameWidth = 40;
    stringstream  str;
    str << left << setw(fieldNameWidth) << "sptk::WSString"
        << " m_" << m_name;
    if (initialize)
    {
        str << " {\"" << m_name << "\", true}";
    }
    return str.str();
}

WSParserComplexType::WSParserComplexType(const xdoc::SNode& complexTypeElement, const String& name,
                                         const String& typeName)
    : m_name(name.empty() ? complexTypeElement->attributes().get("name") : name)
    , m_typeName(typeName.empty() ? complexTypeElement->attributes().get("type") : typeName)
    , m_element(complexTypeElement)
{
    xdoc::SNode simpleTypeElement = nullptr;
    if (!m_typeName.empty())
    {
        simpleTypeElement = findSimpleType(m_typeName);
    }
    else if (complexTypeElement->getQualifiedName() == "xsd:element")
    {
        simpleTypeElement = m_element;
    }

    if (simpleTypeElement)
    {
        const auto& restrictionElement = simpleTypeElement->findFirst("xsd:restriction");
        if (restrictionElement != nullptr)
        {
            m_typeName = restrictionElement->attributes().get("base");
            m_restriction = make_shared<WSRestriction>(m_typeName, restrictionElement->parent());
        }
    }

    if (const auto& documentationElement = complexTypeElement->findFirst("xsd:documentation");
        documentationElement != nullptr)
    {
        m_documentation = documentationElement->getText().trim();
    }

    if (m_typeName.empty())
    {
        m_typeName = m_name;
    }

    String maxOccurs;
    String minOccurs;
    if (complexTypeElement->attributes().have("maxOccurs"))
    {
        maxOccurs = complexTypeElement->attributes().get("maxOccurs");
    }
    if (complexTypeElement->attributes().have("minOccurs"))
    {
        minOccurs = complexTypeElement->attributes().get("minOccurs");
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
    if (String cxxType = WSTypeTranslator::toCxxType(m_typeName, "");
        !cxxType.empty())
    {
        return cxxType;
    }

    const size_t pos = m_typeName.find(':');
    return "C" + m_typeName.substr(pos + 1);
}

void WSParserComplexType::parseSequence(const xdoc::SNode& sequence)
{
    for (const auto& node: sequence->nodes())
    {
        if (node->getQualifiedName() == "xsd:element")
        {
            m_sequence.push_back(make_shared<WSParserComplexType>(node));
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

    for (const auto& node: m_element->nodes())
    {
        if (node->getQualifiedName() == "xsd:attribute")
        {
            auto attrName = node->attributes().get("name");
            m_attributes[attrName] = make_shared<WSParserAttribute>(attrName, node->attributes().get("type"));
        }
        else if (node->getQualifiedName() == "xsd:sequence")
        {
            parseSequence(node);
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
    includeFiles.push_back("#include <sptk5/db/QueryParameterList.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSBasicTypes.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSComplexType.h>");
    includeFiles.push_back("#include <sptk5/wsdl/WSRestriction.h>");

    for (const auto& usedClass: usedClasses)
    {
        includeFiles.push_back("#include \"" + usedClass + ".h\"");
    }

    includeFiles.sort();
    classDeclaration << includeFiles.join("\n") << "\n\n";
}

WSParserComplexType::Initializer WSParserComplexType::makeInitializer() const
{
    Initializer initializer;

    for (const auto& complexType: m_sequence)
    {
        initializer.copyCtor.push_back("m_" + complexType->name() + "(other.m_" + complexType->name() + ")");
        initializer.moveCtor.push_back(
            "m_" + complexType->name() + "(std::move(other.m_" + complexType->name() + "))");
        initializer.copyAssign.push_back("m_" + complexType->name() + " = other.m_" + complexType->name());
        initializer.moveAssign.push_back(
            "m_" + complexType->name() + " = std::move(other.m_" + complexType->name() + ")");
    }

    for (const auto& [name, attribute]: m_attributes)
    {
        initializer.copyCtor.push_back("m_" + name + "(other.m_" + name + ")");
        initializer.moveCtor.push_back(
            "m_" + name + "(std::move(other.m_" + name + "))");
        initializer.copyAssign.push_back("m_" + name + " = other.m_" + name);
        initializer.moveAssign.push_back(
            "m_" + name + " = std::move(other.m_" + name + ")");
    }

    return initializer;
}

void WSParserComplexType::generateDefinition(std::ostream& classDeclaration, Strings& fieldNames,
                                             Strings& elementNames, Strings& attributeNames,
                                             const String& serviceNamespace) const
{
    const String className = "C" + wsClassName(m_name);

    classDeclaration << "#pragma once\n\n";

    const auto usedClasses = getUsedClasses();

    printDeclarationIncludes(classDeclaration, usedClasses);

    const String tagName = makeTagName(className);

    classDeclaration << "namespace " << serviceNamespace << " {\n\n";

    classDeclaration << "/**\n";
    classDeclaration << " * WSDL complex type class " << className << ".\n";
    classDeclaration << " * Generated by wsdl2cxx SPTK utility.\n";
    classDeclaration << " */\n";

    classDeclaration << "class WS_EXPORT " << className << " : public sptk::WSComplexType\n";
    classDeclaration << "{\n";

    classDeclaration << "public:\n\n";
    classDeclaration << "    /**\n";
    classDeclaration << "     * ID of the class\n";
    classDeclaration << "     */\n";
    classDeclaration << "    static sptk::String classId()\n";
    classDeclaration << "    {\n";
    classDeclaration << "        return \"" << className.substr(1) << "\";\n";
    classDeclaration << "    }\n\n";

    for (const auto& [name, value]: m_attributes)
    {
        attributeNames.push_back(name);
    }

    Initializer initializer = makeInitializer();
    if (!m_sequence.empty())
    {
        classDeclaration << "   // Elements\n";
        for (const auto& complexType: m_sequence)
        {
            constexpr int fieldNameWidth = 40;
            appendMemberDocumentation(classDeclaration, complexType);

            String       cxxType = complexType->className();
            const string optional =
                (static_cast<int>(complexType->multiplicity()) & static_cast<int>(WSMultiplicity::ZERO_OR_ONE)) != 0 ? ", true" : ", false";
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
            classDeclaration << ";\n";
        }
    }

    appendClassAttributes(classDeclaration, fieldNames, initializer);

    classDeclaration << "\n";

    classDeclaration << "   /**\n";
    classDeclaration << "    * Constructor\n";
    classDeclaration << "    * @param elementName        WSDL element name\n";
    classDeclaration << "    * @param optional           Is element optional flag\n";
    classDeclaration << "    */\n";
    classDeclaration << "   explicit " << className << "(const char* elementName=\"" << tagName
                     << "\", bool optional=false);\n\n";

    classDeclaration << "   /**\n";
    classDeclaration << "    * Copy constructor\n";
    classDeclaration << "    * @param other              Other object\n";
    classDeclaration << "    */\n";
    classDeclaration << "   " << className << "(const " << className << "& other);\n\n";

    classDeclaration << "   /**\n";
    classDeclaration << "    * Move constructor\n";
    classDeclaration << "    * @param other              Other object\n";
    classDeclaration << "    */\n";
    classDeclaration << "   " << className << "(" << className << "&& other) noexcept;\n\n";

    classDeclaration << "   /**\n";
    classDeclaration << "    * Destructor\n";
    classDeclaration << "    */\n";
    classDeclaration << "   ~" << className << "() override = default;\n\n";

    classDeclaration << "   /**\n";
    classDeclaration << "    * Copy assignment\n";
    classDeclaration << "    * @param other              Other object\n";
    classDeclaration << "    */\n";
    classDeclaration << "   " << className << "& operator = (const " << className << "& other) = default;\n\n";

    classDeclaration << "   /**\n";
    classDeclaration << "    * Move assignment\n";
    classDeclaration << "    * @param other              Other object\n";
    classDeclaration << "    */\n";
    classDeclaration << "   " << className << "& operator = (" << className << "&& other) noexcept = default;\n\n";

    classDeclaration << "   /**\n";
    classDeclaration << "    * Get complex type field names.\n";
    classDeclaration << "    * @param group              Field group: elements, attributes, or both\n";
    classDeclaration << "    * @return list of fields as Strings\n";
    classDeclaration << "    */\n";
    classDeclaration << "   static const sptk::Strings& fieldNames(sptk::WSFieldIndex::Group group);\n";

    classDeclaration << endl;

    classDeclaration << "private:\n\n";
    classDeclaration << "   /**\n"
                     << "    * Check restrictions\n"
                     << "    * Throws an exception if any restriction is violated.\n"
                     << "    */\n"
                     << "   void checkRestrictions() const override;\n";

    classDeclaration << "};\n";
    classDeclaration << endl;
    classDeclaration << "}\n";
}

String WSParserComplexType::makeTagName(const String& className)
{
    String                  tagName = lowerCase(className.substr(1));
    const RegularExpression matchWords("([A-Z]+[a-z]+)", "g");
    if (const auto words = matchWords.m(className.substr(1));
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
                         << elementNames.join(", &m_") << "});\n";
    }
    if (!attributeNames.empty())
    {
        classDeclaration << "    WSComplexType::setAttributes(fieldNames(WSFieldIndex::Group::ATTRIBUTES), {&m_"
                         << attributeNames.join(", &m_") << "});\n";
    }
}

void WSParserComplexType::appendClassAttributes(ostream& classDeclaration, Strings& fieldNames,
                                                Initializer& initializer) const
{
    if (!m_attributes.empty())
    {
        classDeclaration << "   // Attributes\n";
        for (const auto& [name, attr]: m_attributes)
        {
            classDeclaration << "   " << attr->generate(true) << ";\n";
            initializer.copyCtor.push_back("m_" + attr->name() + "(other.m_" + attr->name() + ")");
            initializer.moveCtor.push_back("m_" + attr->name() + "(std::move(other.m_" + attr->name() + "))");
            fieldNames.push_back(attr->name());
        }
    }
}

void WSParserComplexType::appendMemberDocumentation(ostream&                    classDeclaration,
                                                    const SWSParserComplexType& complexType)
{
    if (!complexType->m_documentation.empty())
    {
        classDeclaration << endl;
        classDeclaration << "   /**\n";

        for (const Strings rows(complexType->m_documentation, "[\n\r]+", Strings::SplitMode::REGEXP);
             const String& row: rows)
        {
            classDeclaration << "    * " << trim(row) << endl;
        }
        classDeclaration << "    */\n";
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
    classImplementation << "#include \"" << className << ".h\"\n";
    classImplementation << "using namespace std;\n";
    classImplementation << "using namespace sptk;\n";
}

void WSParserComplexType::printImplementationRestrictions(std::ostream& classImplementation, std::ostream& checks) const
{
    Strings     requiredElements;
    std::size_t restrictionIndex = 0;
    for (const auto& complexType: m_sequence)
    {
        if ((static_cast<int>(complexType->multiplicity()) & static_cast<int>(WSMultiplicity::REQUIRED)) != 0)
        {
            requiredElements.push_back(complexType->name());
        }

        if (!complexType->m_typeName.startsWith("xsd:"))
        {
            continue;
        }

        const String restrictionCheck = addOptionalRestriction(classImplementation, complexType, restrictionIndex);
        if (!restrictionCheck.empty())
        {
            if (restrictionIndex == 1)
            {
                checks << "    // Check value restrictions\n";
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
        checks << "    // Check 'required' restrictions\n";
        for (const auto& requiredElement: requiredElements)
        {
            checks << "    m_" << requiredElement << ".throwIfNull(\"" << wsClassName(m_name) << "." << requiredElement
                   << "\");\n";
        }
    }
}

void WSParserComplexType::printImplementationCheckRestrictions(ostream&      classImplementation,
                                                               const String& className) const
{
    classImplementation << "void " << className << "::checkRestrictions() const\n"
                        << "{\n";
    stringstream checks;
    printImplementationRestrictions(classImplementation, checks);
    if (checks.str().empty())
    {
        classImplementation << "    // There are no checks for restrictions\n";
    }
    else
    {
        classImplementation << checks.str();
    }
    classImplementation << "}\n\n";
}

String WSParserComplexType::addOptionalRestriction(std::ostream&               implementation,
                                                   const SWSParserComplexType& complexType,
                                                   size_t&                     restrictionIndex) const
{
    String restrictionCheck;
    if (complexType->m_restriction != nullptr)
    {
        ++restrictionIndex;
        const String restrictionName = "restriction_" + int2string(restrictionIndex);
        const auto   restrictionCtor = complexType->m_restriction->generateConstructor(restrictionName);
        if (!restrictionCtor.empty())
        {
            if (restrictionIndex == 1)
            {
                implementation << "    // Restrictions\n";
            }
            implementation << "    static const " << restrictionCtor << ";\n";
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

void WSParserComplexType::generateImplementation(std::ostream& classImplementation, const Strings& fieldNames,
                                                 const Strings& elementNames,
                                                 const Strings& attributeNames, const String& serviceNamespace) const
{
    const String className = "C" + wsClassName(m_name);

    printImplementationIncludes(classImplementation, className);

    classImplementation << "using namespace " << serviceNamespace << ";\n\n";

    classImplementation << "const sptk::Strings& " << className << "::fieldNames(WSFieldIndex::Group group)\n";
    classImplementation << "{\n";
    classImplementation << "    static const Strings _fieldNames { \"" << fieldNames.join("\", \"") << "\" };\n";
    classImplementation << "    static const Strings _elementNames { \"" << elementNames.join("\", \"") << "\" };\n";
    classImplementation << "    static const Strings _attributeNames { \"" << attributeNames.join("\", \"") << "\" };\n\n";
    classImplementation << "    switch (group) {\n";
    classImplementation << "        case WSFieldIndex::Group::ELEMENTS: return _elementNames;\n";
    classImplementation << "        case WSFieldIndex::Group::ATTRIBUTES: return _attributeNames;\n";
    classImplementation << "        default: break;\n";
    classImplementation << "    }\n\n";
    classImplementation << "    return _fieldNames;\n";
    classImplementation << "}\n\n";

    printImplementationConstructors(classImplementation, className, elementNames, attributeNames);

    printImplementationCheckRestrictions(classImplementation, className);

    const RegularExpression matchStandardType("^xsd:");
}

void WSParserComplexType::printImplementationConstructors(ostream& classImplementation, const String& className,
                                                          const Strings& elementNames,
                                                          const Strings& attributeNames) const
{
    auto       tagName = makeTagName(className);
    const auto initializer = makeInitializer();

    classImplementation << className << "::" << className << "(const char* elementName, bool optional)\n"
                        << ": " << initializer.ctor.join(",\n  ") << endl
                        << "{\n";
    generateSetFieldIndex(classImplementation, elementNames, attributeNames);
    classImplementation << "}\n\n";

    classImplementation << className << "::" << className << "(const " << className << "& other)\n"
                        << ": " << initializer.copyCtor.join(",\n  ") << endl
                        << "{\n";
    generateSetFieldIndex(classImplementation, elementNames, attributeNames);
    classImplementation << "}\n\n";

    classImplementation << className << "::"
                        << "" << className << "(" << className << "&& other) noexcept\n"
                        << ": " << initializer.moveCtor.join(",\n  ") << endl
                        << "{\n";
    generateSetFieldIndex(classImplementation, elementNames, attributeNames);
    classImplementation << "}\n\n";
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

xdoc::SNode WSParserComplexType::findSimpleType(const String& typeName)
{
    const auto itor = SimpleTypeElements.find(typeName);
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
        output << endl
               << checks.str();
    }
}
