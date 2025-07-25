/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSTypeTranslator.h - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_WSTYPETRANSLATOR_H__
#define __SPTK_WSTYPETRANSLATOR_H__

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>

namespace sptk
{

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * @brief Translates WSDL type names to C++ type names
 */
class WSTypeTranslator
{
    /**
     * WSDL to C++ type map
     */
    std::map<std::string,std::string> wsTypeToCxxTypeMap;

public:
    /**
     * @brief Constructor
     */
    WSTypeTranslator() noexcept;

    /**
     * @brief Translates WSDL type names to C++ type names
     * @param wsType            WSDL type name
     * @param defaultType       C++ type name returned when match is not found
     */
    String toCxxType(const String& wsType, const String& defaultType = "std::string") const;
};

/**
 * @brief Global WSDL translation object
 */
extern WSTypeTranslator wsTypeTranslator;

/**
 * @}
 */

}
#endif
