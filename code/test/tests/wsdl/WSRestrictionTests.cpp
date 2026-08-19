/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "sptk5/Printer.h"
#include "sptk5/wsdl/WSBasicTypes.h"


#include <gtest/gtest.h>
#include <sptk5/wsdl/WSRestriction.h>
#include <sptk5/xdoc/Document.h>

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
namespace sptk {

TEST(WSRestrictionTests,parseEnumeration)
{
    xdoc::Document document;
    document.load(coloursXML);

    const auto simpleTypeElement = document.root()->findFirst("xsd:simpleType");

    const WSRestriction restrictions("Colours", simpleTypeElement);

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

TEST(WSRestrictionTests,parseInitials)
{
    xdoc::Document document;
    document.load(initialsXML);

    const auto simpleTypeElement = document.root()->findFirst("xsd:simpleType");

    const WSRestriction restrictions("Initials", simpleTypeElement);

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


TEST(WSRestrictionTests,check)
{
    using enum WSRestriction::Type;

    const WSRestriction restriction(Enumeration, "xsd:string", {});
    const WSRestriction restriction_1(Enumeration, "xsd:string", {"add", "modify", "remove", "list"});
    const WSRestriction restriction_2(Pattern, "xsd:string", {"^(add|modify|remove|list)$"});
    const WSRestriction restriction_3(Pattern, "xsd:string", {"^(add|modify)$", "^(remove|list)$"});

    EXPECT_EQ(WSRestriction::Type::Unknown, restriction.type());
    // Check value restrictions
    EXPECT_NO_THROW(restriction_1.check("Control.action", "add"));
    EXPECT_ANY_THROW(restriction_1.check("Control.action", "edit"));

    EXPECT_NO_THROW(restriction_2.check("Control.action", "add"));
    EXPECT_ANY_THROW(restriction_2.check("Control.action", "edit"));

    EXPECT_NO_THROW(restriction_2.check("Control.action", "add"));
    EXPECT_NO_THROW(restriction_2.check("Control.action", "remove"));
    EXPECT_ANY_THROW(restriction_2.check("Control.action", "edit"));

    WSString action;
    EXPECT_ANY_THROW(action.throwIfNull("Control.action"));
}

TEST(WSRestrictionTests,generateConstructor)
{
    using enum WSRestriction::Type;

    const WSRestriction restriction(Enumeration, "xsd:string", {"add", "modify", "remove", "list"});

    const auto ctor = restriction.generateConstructor("restriction_1");

    EXPECT_FALSE(ctor.empty());
    EXPECT_TRUE(ctor.contains("restriction_1"));
    EXPECT_TRUE(ctor.contains(R"({ "add", "modify", "remove", "list" })"));
}

} // namespace sptk_test
