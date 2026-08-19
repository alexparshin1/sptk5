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

#include "sptk5/xdoc/Document.h"
#include <gtest/gtest.h>
#include <sptk5/xdoc/Node.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

static const String testXmlDocument(
    "<xml encoding=\"utf-8\">"
    "<customer>"
    "<name>John</name>"
    "<address><city>Walhalla</city><street>17 Elm Street</street></address>"
    "</customer>"
    "</xml>");
namespace sptk {

TEST(XDocumentTests,typeRegexp)
{
    EXPECT_TRUE(isInteger("0"));
    EXPECT_TRUE(isInteger("+1"));
    EXPECT_TRUE(isInteger("+100"));
    EXPECT_TRUE(isInteger("-1234"));
    EXPECT_FALSE(isInteger("01234"));
    EXPECT_FALSE(isInteger("1234-11"));

    EXPECT_TRUE(isFloat("0.1"));
    EXPECT_TRUE(isFloat("+0.123"));
    EXPECT_TRUE(isFloat("-0.123"));
    EXPECT_TRUE(isFloat("-0.123e4"));
    EXPECT_TRUE(isFloat("-10.123e43"));
    EXPECT_FALSE(isFloat("00.123e43"));
    EXPECT_FALSE(isFloat("127.0.0.1"));
    EXPECT_FALSE(isFloat("127"));
}

TEST(XDocumentTests,setNameSpace)
{
    const Buffer   input(testXmlDocument);
    xdoc::Document document;
    document.load(input);

    auto customer = document.root()->findFirst(NodeName("customer"));
    EXPECT_EQ(customer->getName(), "customer");
    customer->setNamespaceRecursive("ns1");
    EXPECT_EQ(customer->getName(), "customer");
    EXPECT_EQ(customer->getNamespace(), "ns1");

    auto address = document.root()->findFirst(NodeName("address", "ns1"));
    document.exportTo(xdoc::DataFormat::XML, cout, true);
    ASSERT_TRUE(address != nullptr);
}
} // namespace sptk_test
