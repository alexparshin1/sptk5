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

#include <sptk5/wsdl/WSTypeTranslator.h>

using namespace std;
using namespace sptk;

const WSTypeTranslator sptk::wsTypeTranslator;

const std::map<String, String> WSTypeTranslator::wsTypeToCxxTypeMap
{
    { "xsd:boolean",  "sptk::WSBool" },
    { "xsd:date",     "sptk::WSDate" },
    { "xsd:dateTime", "sptk::WSDateTime" },
    { "xsd:double",   "sptk::WSDouble" },
    { "xsd:float",    "sptk::WSDouble" },
    { "xsd:int",      "sptk::WSInteger" },
    { "xsd:string",   "sptk::WSString" },
    { "xsd:time",     "sptk::WSTime" }
};

String WSTypeTranslator::toCxxType(const String& wsType, const String& defaultType) const
{
    auto itor = wsTypeToCxxTypeMap.find(wsType);
    if (itor == wsTypeToCxxTypeMap.end())
        return defaultType;
    return itor->second;
}
