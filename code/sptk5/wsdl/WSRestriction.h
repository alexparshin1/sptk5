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

#pragma once

#include <sptk5/cxml>
#include <sptk5/Variant.h>
#include <sptk5/xml/Element.h>

namespace sptk {

    /**
     * WSDL Restriction
     */
    class SP_EXPORT WSRestriction
    {
    public:

        enum class Type: uint8_t
        {
            Unknown,
            Enumeration,
            Pattern
        };

        /**
         * Constructor from WSDL (XML) definition
         * @param typeName                  WSDL type name
         * @param simpleTypeElement         Simple type XML node
         */
        WSRestriction(const std::string& typeName, xml::Node* simpleTypeElement);

        /**
         * Constructor from WSDL (XML) definition
         * @param type                      Restriction type
         * @param wsdlTypeName              WSDL type name
         * @param enumerationsOrPatternss   Enumerations or patterns
         */
        WSRestriction(Type type, const String& wsdlTypeName, const Strings& enumerationsOrPattern);

        /**
         * Get restriction type
         * @return restriction type
         */
        Type type() const;

        /**
         * Restriction check
         *
         * Checks value to satisfy restriction.
         * If value violates restriction, throws exception.
         * @param typeName      Name of the checked type (for error messages)
         * @param value         Value to check
         */
        void check(const String& typeName, const String& value) const;

        /**
         * Generates restriction constructor for C++ skeleton
         */
        String generateConstructor(const String& variableName) const;

        /**
         * Optional regular expression to match
         * @return regular expression string
         */
        const std::vector<RegularExpression>& patterns() const { return m_patterns; }

        /**
         * Optional enumeration to match
         * @return enumeration
         */
        Strings enumeration() const { return m_enumeration; }

    private:

        Type                            m_type { Type::Unknown }; ///< Restriction type
        String                          m_wsdlTypeName;           ///< WSDL type name
        Strings                         m_enumeration;            ///< List of enumerations if any
        std::vector<RegularExpression>  m_patterns;               ///< Patterns
    };

    using SWSRestriction = std::shared_ptr<WSRestriction>;
}
