/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSRestriction.h - description                          ║
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

#ifndef __SPTK_WSRESTRICTION_H__
#define __SPTK_WSRESTRICTION_H__

#include <sptk5/cxml>
#include <sptk5/Variant.h>
#include <sptk5/xml/Element.h>

namespace sptk {

    /**
     * @brief WSDL Restriction
     */
    class WSRestriction
    {
        /**
         * WSDL type name
         */
        std::string m_typeName;

        /**
         * List of enumerations if any
         */
        Strings     m_enumerations;

    public:
        /**
         * @brief Constructor from WSDL (XML) definition
         * @param typeName      WSDL type name
         * @param simpleTypeElement Simple type XML node
         */
        WSRestriction(const std::string& typeName, xml::Node* simpleTypeElement);

        /**
         * @brief Constructor from WSDL (XML) definition
         * @param typeName      WSDL type name
         * @param enumerations  Enumerations or empty string
         * @param delimiter     Enumerations delimiter
         */
        WSRestriction(const std::string& typeName, const std::string& enumerations, const char* delimiter = "|");

        /**
         * @brief Restriction check
         *
         * Checks value to satisfy restriction.
         * If value violates restriction, throws exception.
         * @param typeName      Name of the checked type (for error messages)
         * @param value         Value to check
         */
        void check(const std::string& typeName, const std::string& value) const;

        /**
         * @brief Generates restriction constructor for C++ skeleton
         */
        std::string generateConstructor(const std::string& variableName) const;
    };

}
#endif

