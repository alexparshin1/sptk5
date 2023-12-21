/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/wsdl/WSOperation.h>
#include <sptk5/xdoc/Document.h>

namespace sptk {

/**
 * Generator of OpenAPI service description
 */
class SP_EXPORT OpenApiGenerator
{
public:
    /**
     * Authentication method
     */
    enum class AuthMethod : uint8_t
    {
        NONE,  ///< No authentication
        BASIC, ///< Authorization: Basic ZGVtbzpwQDU1dzByZA==
        BEARER ///< Authorization: Bearer <token>
    };

    /**
     * Generation options
     */
    struct Options {
        /**
         * Default authentication method, used by most operations
         */
        AuthMethod defaultAuthMethod {AuthMethod::BEARER};

        /**
         * Authentication method, in format: Operation name -> Auth method
         */
        std::map<String, AuthMethod> operationsAuth;

        /**
         * Output OpenAPI file
         */
        String openApiFile;
    };

    /**
     * Constructor
     * @param title             Service title
     * @param description       Service description
     * @param version           Service version
     * @param servers           Servers that provide this service
     * @param options           Service options
     */
    OpenApiGenerator(String title, String description, String version,
                     Strings servers, Options options);

    /**
     * Generate OpenAPI service description
     * @param output            Output stream
     * @param operations        Service operations
     * @param complexTypes      Service types
     * @param documentation     Service documentation (by operation)
     */
    void generate(std::ostream& output, const WSOperationMap& operations, const WSComplexTypeMap& complexTypes,
                  const std::map<String, String>& documentation) const;

    static AuthMethod authMethod(const String& auth);

    static String authMethodName(AuthMethod auth);

private:
    /**
     * Create component object of OpenAPI service description
     * @param document          Output JSON
     * @param complexTypes      Service types
     */
    void createComponents(xdoc::Document& document, const WSComplexTypeMap& complexTypes) const;

    /**
     * Create paths object of OpenAPI service description
     * @param document          Output JSON
     * @param operations        Service operations
     * @param documentation     Service documentation (by operation)
     */
    void createPaths(xdoc::Document& document, const WSOperationMap& operations,
                     const std::map<String, String>& documentation) const;

    /**
     * Create servers object of OpenAPI service description
     * @param document          Output JSON
     */
    void createServers(xdoc::Document& document) const;

    const String m_title;       ///< Service title
    const String m_description; ///< Service description
    const String m_version;     ///< Service version
    const Strings m_servers;    ///< Service servers
    const Options m_options;    ///< Service options

    static void parseClassName(const SWSParserComplexType& ctypeProperty, const xdoc::SNode& property);

    static void parseRestriction(const SWSParserComplexType& ctypeProperty, const xdoc::SNode& property);

    static void parseRestrictionPatterns(const xdoc::SNode& property, const SWSRestriction& restriction);
};

} // namespace sptk
