/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/wsdl/WSRestriction.h>
#include <sptk5/xdoc/Document.h>
#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

static const String coloursXML {
    "<xsd:element name=\"Colours\">"
    "<xsd:simpleType>"
    "<xsd:restriction base=\"xsd:string\">"
    "<xsd:enumeration value=\"Red\"/>"
    "<xsd:enumeration value=\"Green\"/>"
    "<xsd:enumeration value=\"Blue\"/>"
    "</xsd:restriction>"
    "</xsd:simpleType>"
    "</xsd:element>"};

static const String initialsXML {
    "<xsd:element name=\"Initials\">"
    "<xsd:simpleType>"
    "<xsd:restriction base=\"xsd:string\">"
    "<xsd:pattern value=\"[A-Z][A-Z]\"/>"
    "</xsd:restriction>"
    "</xsd:simpleType>"
    "</xsd:element>"};

TEST(SPTK_WSRestriction, parseEnumeration)
{
    xdoc::Document document;
    document.load(coloursXML);

    auto simpleTypeElement = document.root()->findFirst("xsd:simpleType");

    WSRestriction restrictions("Colours", simpleTypeElement);

    try
    {
        restrictions.check("Colour", "Green");
    }
    catch (const Exception&)
    {
        FAIL() << "Green is allowed colour!";
    }

    try
    {
        restrictions.check("Colour", "Yellow");
        FAIL() << "Yellow is not allowed colour!";
    }
    catch (const Exception&)
    {
        SUCCEED() << "Correctly detected not allowed colour";
    }
}

TEST(SPTK_WSRestriction, parseInitials)
{
    xdoc::Document document;
    document.load(initialsXML);

    auto simpleTypeElement = document.root()->findFirst("xsd:simpleType");

    WSRestriction restrictions("Initials", simpleTypeElement);

    try
    {
        restrictions.check("Initials", "AL");
    }
    catch (const Exception&)
    {
        FAIL() << "AL is correct initials!";
    }

    try
    {
        restrictions.check("Initials", "xY");
        FAIL() << "xY is incorrect initials!";
    }
    catch (const Exception&)
    {
        SUCCEED() << "Correctly detected incorrect initials";
    }
}
