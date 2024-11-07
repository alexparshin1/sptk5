/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>
#include <sptk5/xdoc/Node.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

TEST(SPTK_XDocument, nodeName_create)
{
    NodeName nodeName("test");

    EXPECT_STREQ("test", nodeName.getQualifiedName().c_str());
    EXPECT_STREQ("", nodeName.getNameSpace().c_str());

    NodeName nodeName2("ns1:test");

    EXPECT_STREQ("test", nodeName2.getName().c_str());
    EXPECT_STREQ("ns1", nodeName2.getNameSpace().c_str());
}

TEST(SPTK_XDocument, nodeName_nameSpace)
{
    NodeName nodeName("test");
    EXPECT_STREQ("", nodeName.getNameSpace().c_str());

    nodeName.setNameSpace("ns1");
    EXPECT_STREQ("ns1", nodeName.getNameSpace().c_str());

    NodeName nodeName2("ns2:test");
    EXPECT_STREQ("ns2", nodeName2.getNameSpace().c_str());

    nodeName2.setNameSpace("ns1");
    EXPECT_STREQ("ns1", nodeName2.getNameSpace().c_str());
}

TEST(SPTK_XDocument, nodeName_ctors)
{
    NodeName nodeName("ns2:test");
    NodeName nodeName2(nodeName);
    EXPECT_EQ(nodeName, nodeName2);

    NodeName nodeName3(std::move(nodeName));
    EXPECT_EQ(nodeName2, nodeName3);
}

TEST(SPTK_XDocument, nodeName_assign)
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

TEST(SPTK_XDocument, nodeName_compare)
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
