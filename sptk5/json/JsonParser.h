/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonParser.h - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
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

#ifndef __JSON__PARSER_H__
#define __JSON__PARSER_H__

#include <sptk5/json/JsonElement.h>
#include <sptk5/json/JsonArrayData.h>

namespace sptk { namespace json {

/// @addtogroup JSON
/// @{

/**
 * JSON Parser
 *
 * Loads JSON text into JSON element
 */
class Parser
{
    friend class Element;

public:
    /**
     * Constructor
     */
    Parser() = default;

    /**
     * Parse JSON text
     * Root element should have JDT_NULL type (empty element) before calling this method.
     * @param jsonElement Element&, JSON element
     * @param json const std::string&, JSON text
     */
    void parse(Element& jsonElement, const std::string& json);
};

}}

#endif
