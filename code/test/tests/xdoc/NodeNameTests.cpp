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

#include <gtest/gtest.h>
#include <sptk5/xdoc/Node.h>

using namespace std;
using namespace sptk;
using namespace xdoc;
namespace sptk {

TEST(XDocumentTests, nodeNameCreate)
{
    NodeName nodeName("test");

    EXPECT_STREQ("test", nodeName.getQualifiedName().c_str());
    EXPECT_EQ("", nodeName.getNamespace());

    NodeName nodeName2("ns1:test");

    EXPECT_EQ("test", nodeName2.getName());
    EXPECT_EQ("ns1", nodeName2.getNamespace());
}

TEST(XDocumentTests, nodeNameNameSpace)
{
    NodeName nodeName("test");
    EXPECT_EQ("", nodeName.getNamespace());

    nodeName.setNameSpace("ns1");
    EXPECT_EQ("ns1", nodeName.getNamespace());
    EXPECT_STREQ("ns1:test", nodeName.getQualifiedName().c_str());

    NodeName nodeName2("ns2:test");
    EXPECT_EQ("ns2", nodeName2.getNamespace());

    nodeName2.setNameSpace("ns1");
    EXPECT_EQ("ns1", nodeName2.getNamespace());
    EXPECT_STREQ("ns1:test", nodeName2.getQualifiedName().c_str());
}

TEST(XDocumentTests, nodeNameCtors)
{
    NodeName nodeName("ns2:test");
    NodeName nodeName2(nodeName);
    EXPECT_EQ(nodeName, nodeName2);

    NodeName nodeName3(std::move(nodeName));
    EXPECT_EQ(nodeName2, nodeName3);
}

TEST(XDocumentTests, nodeNameAssign)
{
    NodeName nodeName("ns1:test");
    NodeName nodeName2("something");

    nodeName2 = nodeName;
    EXPECT_EQ(nodeName, nodeName2);

    nodeName = String("ns2:test");
    EXPECT_EQ(nodeName.getQualifiedName(), "ns2:test");

    nodeName2 = std::move(nodeName);
    EXPECT_EQ(nodeName2.getQualifiedName(), "ns2:test");
}

TEST(XDocumentTests, nodeNameCompare)
{
    NodeName nodeName("test");
    EXPECT_TRUE(nodeName == "test");

    NodeName nodeName2("ns1:test");
    EXPECT_TRUE(nodeName2 == "ns1:test");

    NodeName nodeName3("test", "ns1");
    EXPECT_TRUE(nodeName3 == "ns1:test");

    EXPECT_NE(nodeName, nodeName2);
    EXPECT_EQ(nodeName2, nodeName3);
}

} // namespace sptk
