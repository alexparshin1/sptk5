/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWSComplexType.cpp - description                       ║
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

#include <sptk5/wsdl/WSComplexType.h>

using namespace std;
using namespace sptk;

void WSComplexType::copyFrom(const WSComplexType& other)
{
    xml::Document xml;
    auto element = new xml::Element(xml, "temp");
    other.unload(element);
    load(element);
}

void WSComplexType::unload(QueryParameterList& output, const char* paramName, const WSBasicType* elementOrAttribute)
{
    if (elementOrAttribute == nullptr)
        return;
    sptk::QueryParameter* param = output.find(paramName);
    if (param != nullptr)
        *param = *elementOrAttribute;
}

void WSComplexType::addElement(xml::Element* parent) const
{
    unload(new xml::Element(parent, m_name.c_str()));
}
