/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWSRestriction.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CWSRESTRICTION_H__
#define __CWSRESTRICTION_H__

#include <sptk5/cxml>
#include <sptk5/Variant.h>
#include <sptk5/xml/CXmlElement.h>

namespace sptk {

    /// @brief WSDL Restriction
    class WSRestriction
    {
        std::string m_typeName;     ///< WSDL type name
        CStrings    m_enumerations; ///< List of enumerations if any
    public:
        /// @brief Constructor from WSDL (XML) definition
        /// @param typeName std::string, WSDL type name
        /// @param simpleTypeElement CXmlNode*, Simple type XML node
        WSRestriction(std::string typeName, CXmlNode* simpleTypeElement);

        /// @brief Constructor from WSDL (XML) definition
        /// @param typeName std::string, WSDL type name
        /// @param enumerations std::string, Enumerations or empty string
        /// @param delimiter const char*, Enumerations delimiter
        WSRestriction(std::string typeName, std::string enumerations, const char* delimiter="|");
        
        /// @brief Restriction check
        ///
        /// Checks value to satisfy restriction.
        /// If value violates restriction, throws exception.
        /// @param value std::string, Value to check
        void check(std::string value) const;
        
        /// @brief Generates restriction constructor for C++ skeleton
        std::string generateConstructor(std::string variableName) const;
    };

}
#endif

