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

#include <ranges>
#include <sptk5/cutils>
#include <sptk5/wsdl/WSParser.h>
#include <utility>

using namespace std;
using namespace sptk;
using namespace xdoc;

OpenApiGenerator::OpenApiGenerator(String title, String description, String version,
                                   Strings servers, Options options)
    : m_title(std::move(title))
    , m_description(std::move(description))
    , m_version(std::move(version))
    , m_servers(std::move(servers))
    , m_options(std::move(options))
{
}

void OpenApiGenerator::generate(std::ostream& output, const WSOperationMap& operations,
                                const WSComplexTypeMap& complexTypes,
                                const std::map<String, String>& documentation) const
{
    // Validate options
    for (const auto& name: views::keys(m_options.operationsAuth))
    {
        if (!operations.contains(name))
        {
            throw Exception("Alternative Auth operation '" + name + "' is not a part of this service");
        }
    }

    Document document;

    document.root()->set("openapi", "3.0.0");

    // Create info object
    const auto info = document.root()->pushNode("info");
    info->set("title", m_title);
    info->set("description", m_description);
    info->set("version", m_version);

    createServers(document);
    createPaths(document, operations, documentation);
    createComponents(document, complexTypes);

    document.root()->exportTo(DataFormat::JSON, output, true);
}

void OpenApiGenerator::createServers(Document& document) const
{
    // Create servers object
    auto& servers = *document.root()->pushNode("servers");
    for (const auto& url: m_servers)
    {
        const auto& server = servers.pushNode("", Node::Type::Object);
        server->set("url", url);
    }
}

void OpenApiGenerator::createPaths(Document& document, const WSOperationMap& operations,
                                   const map<String, String>& documentation) const
{
    static const map<String, String> possibleResponses = {
        {"200", "Ok"},
        {"401", "Unauthorized"},
        {"404", "Not found"},
        {"500", "Server error"},
    };

    // Create paths object
    const auto& paths = document.root()->pushNode("paths");
    for (const auto& [operationName, operation]: operations)
    {
        using enum Node::Type;

        const auto& operationElement = paths->pushNode("/" + operation.m_input->name(), Node::Type::Object);
        const auto& postElement = operationElement->pushNode("post");

        // Define operation security
        AuthMethod authMethod;

        if (auto ator = m_options.operationsAuth.find(operationName);
            ator != m_options.operationsAuth.end())
        {
            authMethod = ator->second;
        }
        else
        {
            authMethod = m_options.defaultAuthMethod;
        }

        if (authMethod != AuthMethod::NONE)
        {
            const auto& securityObject = postElement->pushNode("security", Array);
            const auto& securityMechanism = securityObject->pushNode("", Object);
            securityMechanism->pushNode(authMethodName(authMethod), Array);
        }

        if (auto dtor = documentation.find(operation.m_input->name());
            dtor != documentation.end())
        {
            postElement->set("summary", dtor->second);
        }

        postElement->set("operationId", operationName);

        const auto& requestBody = postElement->pushNode("requestBody", Object);
        const auto& content = requestBody->pushNode("content", Object);
        const auto& data = content->pushNode("application/json", Object);
        const auto& schema = data->pushNode("schema", Object);
        const String ref = "#/components/schemas/" + operation.m_input->name();
        schema->set("$ref", ref);

        const auto& responsesElement = postElement->pushNode("responses", Object);
        for (const auto& [name, description]: possibleResponses)
        {
            const auto& response = responsesElement->pushNode(name, Object);
            response->set("description", description);
            if (name == "200")
            {
                const auto& responseContent = response->pushNode("responseContent", Object);
                const auto& responseEncoding = responseContent->pushNode("application/json", Object);
                const auto& responseSchema = responseEncoding->pushNode("responseSchema", Object);
                const String responseRef = "#/components/schemas/" + operation.m_output->name();
                responseSchema->set("$responseRef", responseRef);
            }
        }
    }
}

void OpenApiGenerator::createComponents(Document& document, const WSComplexTypeMap& complexTypes)
{
    struct OpenApiType {
        String type;
        String format;
    };

    static const map<String, OpenApiType> wsTypesToOpenApiTypes = {
        {"string", {"string", ""}},
        {"datetime", {"string", "date-time"}},
        {"bool", {"boolean", ""}},
        {"integer", {"integer", "int64"}},
        {"double", {"number", "double"}}};

    using enum Node::Type;

    // Create components object
    const auto& components = document.root()->pushNode("components", Object);
    const auto& schemas = components->pushNode("schemas", Object);
    for (const auto& [complexTypeName, complexTypeInfo]: complexTypes)
    {
        const auto& complexType = schemas->pushNode(complexTypeInfo->name(), Object);
        complexType->set("type", "object");
        const auto& properties = complexType->pushNode("properties", Object);
        Strings requiredProperties;
        for (const auto& ctypeProperty: complexTypeInfo->sequence())
        {
            const auto& property = properties->pushNode(ctypeProperty->name(), Object);
            parseClassName(ctypeProperty, property);

            if (ctypeProperty->multiplicity() != WSMultiplicity::ZERO_OR_ONE)
            {
                requiredProperties.push_back(ctypeProperty->name());
            }

            parseRestriction(ctypeProperty, property);
        }
        if (!requiredProperties.empty())
        {
            const auto& required = complexType->pushNode("required", Array);
            for (const auto& property: requiredProperties)
            {
                const auto& element = required->pushNode("", Text);
                element->set(property);
            }
        }
    }

    const auto& securitySchemas = components->pushNode("securitySchemes");
    const auto& basicAuth = securitySchemas->pushNode("basicAuth",
                                                      Object); // arbitrary name for the security scheme
    basicAuth->set("type", "http");
    basicAuth->set("scheme", "basic");

    const auto& bearerAuth = securitySchemas->pushNode("bearerAuth",
                                                       Object); // arbitrary name for the security scheme
    bearerAuth->set("type", "http");
    bearerAuth->set("scheme", "bearer");
    bearerAuth->set("bearerFormat", "JWT"); // optional, arbitrary value for documentation purposes
}

void OpenApiGenerator::parseClassName(const SWSParserComplexType& ctypeProperty, const SNode& property)
{
    struct OpenApiType {
        String type;
        String format;
    };

    static const map<String, OpenApiType> wsTypesToOpenApiTypes = {
        {"string", {"string", ""}},
        {"datetime", {"string", "date-time"}},
        {"bool", {"boolean", ""}},
        {"integer", {"integer", "int64"}},
        {"double", {"number", "double"}}};

    auto className = ctypeProperty->className();
    if (className.startsWith("sptk::WS"))
    {
        className = className.replace("sptk::WS", "").toLowerCase();
        const auto ttor = wsTypesToOpenApiTypes.find(className);
        if (ttor != wsTypesToOpenApiTypes.end())
        {
            property->set("type", ttor->second.type);
            if (!ttor->second.format.empty())
            {
                property->set("format", ttor->second.format);
            }
        }
    }
    else if (className.startsWith("C"))
    {
        className = "#/components/schemas/" + className.substr(1);
        if (static_cast<int>(ctypeProperty->multiplicity()) &
            (static_cast<int>(WSMultiplicity::ZERO_OR_MORE) | static_cast<int>(WSMultiplicity::ONE_OR_MORE)))
        { //array
            property->set("type", "array");
            const auto& items = property->pushNode("items");
            items->set("$ref", className);
        }
        else
        {
            property->set("$ref", className);
        }
    }
}

void OpenApiGenerator::parseRestriction(const SWSParserComplexType& ctypeProperty, const SNode& property)
{
    using enum Node::Type;

    const auto restriction = ctypeProperty->restriction();
    if (restriction)
    {
        if (!restriction->patterns().empty())
        {
            parseRestrictionPatterns(property, restriction);
        }
        else if (!restriction->enumeration().empty())
        {
            const auto& enumArray = property->pushNode("enum", Array);
            for (const auto& str: restriction->enumeration())
            {
                const auto& element = enumArray->pushNode("", Text);
                element->set(str);
            }
        }
    }
}

void OpenApiGenerator::parseRestrictionPatterns(const SNode& property, const SWSRestriction& restriction)
{
    using enum Node::Type;

    if (restriction->patterns().size() == 1)
    {
        property->set("pattern", restriction->patterns()[0].pattern());
    }
    else
    {
        const auto& oneOf = property->pushNode("oneOf", Array);
        for (const auto& regex: restriction->patterns())
        {
            const auto& patternElement = oneOf->pushNode("", Object);
            patternElement->set("pattern", regex.pattern());
        }
    }
}

OpenApiGenerator::AuthMethod OpenApiGenerator::authMethod(const String& auth)
{
    using enum AuthMethod;

    if (auth == "none")
    {
        return NONE;
    }
    if (auth == "basic")
    {
        return BASIC;
    }
    if (auth == "bearer")
    {
        return BEARER;
    }
    throw Exception("Auth method '" + auth + "' is not supported");
}

String OpenApiGenerator::authMethodName(AuthMethod auth)
{
    using enum AuthMethod;

    switch (auth)
    {
        case BASIC:
            return "basicAuth";
        case BEARER:
            return "bearerAuth";
        default:
            return "none";
    }
}
