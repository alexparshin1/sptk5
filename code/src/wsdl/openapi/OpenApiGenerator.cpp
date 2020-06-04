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

#include <sptk5/cutils>
#include "OpenApiGenerator.h"

using namespace std;
using namespace sptk;

OpenApiGenerator::OpenApiGenerator(const String& title, const String& description, const String& version,
                                   const Strings& servers)
: m_title(title), m_description(description), m_version(version), m_servers(servers)
{
}

void OpenApiGenerator::generate(std::ostream& output, const WSOperationMap& operations,
                                const WSComplexTypeMap& complexTypes, const std::map<String,String>& documentation)
{
    static const map<String,String> possibleResponses = {
        {"200", "Ok"},
        {"404", "Not found"},
        {"500", "Server error"},
    };

    struct OpenApiType {
        String type;
        String format;
    };
    static const map<String,OpenApiType> wsTypesToOpenApiTypes = {
        { "string", { "string", "" } },
        { "datetime", { "string", "date-time" } },
        { "bool", {"boolean", ""} },
        { "integer", { "integer", "int64" } },
        { "double", { "number", "double" } }
    };

    json::Document document;

    document.root()["openapi"] = "3.0.0";

    // Create info object
    auto& info = *document.root().add_object("info");
    info["title"] = m_title;
    info["description"] = m_description;
    info["version"] = m_version;

    // Create servers object
    auto& servers = *document.root().add_array("servers");
    for (auto& url: m_servers) {
        auto& server = *servers.push_object();
        server["url"] = url;
    }

    // Create paths object
    auto& paths = *document.root().add_object("paths");
    for (auto& itor: operations) {
        auto& operation = itor.second;
        auto& operationElement = *paths.add_object("/" + itor.first);
        auto& postElement = *operationElement.add_object("post");

        auto dtor = documentation.find(operation.m_input->name());
        if (dtor != documentation.end())
            postElement["summary"] = dtor->second;

        postElement["operationId"] = itor.first;

        auto& requestBody = *postElement.add_object("requestBody");
        auto& content = *requestBody.add_object("content");
        auto& data = *content.add_object("application/json");
        auto& schema = *data.add_object("schema");
        schema["$ref"] = "#/components/schemas/" + operation.m_input->name();

        auto& responsesElement = *postElement.add_object("responses");
        for (auto& itor: possibleResponses) {
            auto& response = *responsesElement.add_object(itor.first);
            response["description"] = itor.second;
        }
    }

    // Create components object
    auto& components = *document.root().add_object("components");
    auto& schemas = *components.add_object("schemas");
    for (auto& itor: complexTypes) {
        auto& complexTypeInfo = itor.second;
        auto& complexType = *schemas.add_object(complexTypeInfo->name());
        complexType["type"] = "object";
        auto& properties = *complexType.add_object("properties");
        Strings requiredProperties;
        for (auto ctypeProperty: complexTypeInfo->sequence()) {
            if (ctypeProperty->name() == "type")
                cout << "";
            auto& property = *properties.add_object(ctypeProperty->name());
            auto className = ctypeProperty->className();
            if (className.startsWith("sptk::WS")) {
                className = className.replace("sptk::WS", "").toLowerCase();
                auto ttor = wsTypesToOpenApiTypes.find(className);
                if (ttor != wsTypesToOpenApiTypes.end()) {
                    property["type"] = ttor->second.type;
                    if (!ttor->second.format.empty())
                        property["format"] = ttor->second.format;
                }
            }
            else if (className.startsWith("C")) {
                className = "#/components/schemas/" + className.substr(1);
                property["$ref"] = className;
            }

            if (ctypeProperty->multiplicity() != WSM_OPTIONAL)
                requiredProperties.push_back(ctypeProperty->name());

            auto restriction = ctypeProperty->restriction();
            if (restriction) {
                if (!restriction->pattern().empty())
                    property["pattern"] = restriction->pattern();
                else if (!restriction->enumeration().empty()) {
                    auto& enumArray = *property.add_array("enum");
                    for (auto& str: restriction->enumeration())
                        enumArray.push_back(str);
                }
            }
        }
        if (!requiredProperties.empty()) {
            auto& required = *complexType.add_array("required");
            for (auto& property: requiredProperties)
                required.push_back(property);
        }
    }

    document.exportTo(output);
}
