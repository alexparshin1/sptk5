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
#include <sptk5/RegularExpression.h>
#include <sptk5/xdoc/Attributes.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

static const String testXML1("<AAA><BBB/><CCC/><BBB/><BBB/><DDD><BBB/></DDD><CCC/></AAA>");
static const String testXML2("<AAA><BBB/><CCC/><BBB/><DDD><BBB/></DDD><CCC><DDD><BBB/><BBB/></DDD></CCC></AAA>");
static const String testXML3(
    "<AAA><XXX><DDD><BBB/><BBB/><EEE/><FFF/></DDD></XXX><CCC><DDD><BBB/><BBB/><EEE/><FFF/></DDD></CCC><CCC><BBB><BBB><BBB/></BBB></BBB></CCC></AAA>");
static const String testXML4("<AAA><BBB>1</BBB><BBB>2</BBB><BBB>3</BBB><BBB>4</BBB></AAA>");
static const String testXML5(R"(<AAA><BBB>1</BBB><BBB id="002">2</BBB><BBB id="003">3</BBB><BBB>4</BBB></AAA>)");

TEST(SPTK_XDocument, select)
{
    Document document;

    document.load(testXML1);

    auto elementSet = document.root()->select("/AAA");
    EXPECT_EQ(size_t(1), elementSet.size());

    elementSet = document.root()->select("/AAA/CCC");
    EXPECT_EQ(size_t(2), elementSet.size());

    elementSet = document.root()->select("/AAA/DDD/BBB");
    EXPECT_EQ(size_t(1), elementSet.size());
}

TEST(SPTK_XDocument, parent)
{
    Node::Vector elementSet;
    Document document;

    const auto& node1 = document.root()->pushNode("Node1-level1");
    const auto& node2 = node1->pushNode("Node2-level2");

    const auto& node3 = document.root()->pushNode("Node3-level1");
    const auto& node4 = node3->pushNode("Node4-level2");

    EXPECT_STREQ(node2->parent()->name().c_str(), "Node1-level1");
    EXPECT_STREQ(node4->parent()->name().c_str(), "Node3-level1");
}

TEST(SPTK_XDocument, select2)
{
    Document document;

    document.load(testXML2);

    auto elementSet = document.root()->select("//BBB");
    EXPECT_EQ(size_t(5), elementSet.size());

    elementSet = document.root()->select("//DDD/BBB");
    EXPECT_EQ(size_t(3), elementSet.size());
}

TEST(SPTK_XDocument, select3)
{
    Document document;

    document.load(testXML3);

    Buffer buff;
    document.exportTo(DataFormat::XML, buff, false);

    auto elementSet = document.root()->select("/AAA/CCC/DDD/*");
    EXPECT_EQ(size_t(4), elementSet.size());

    elementSet = document.root()->select("//*");
    EXPECT_EQ(size_t(17), elementSet.size());
}

TEST(SPTK_XDocument, select4)
{
    Document document;

    document.load(testXML4);

    auto elementSet = document.root()->select("/AAA/BBB[1]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("1", elementSet[0]->getString().c_str());

    elementSet = document.root()->select("/AAA/BBB[last()]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("4", elementSet[0]->getString().c_str());
}

TEST(SPTK_XDocument, select5)
{
    Document document;

    document.load(testXML5);

    auto elementSet = document.root()->select("//BBB[@id=002]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("2", elementSet[0]->getString().c_str());

    auto elementSet2 = document.root()->select("//BBB[@id=003]");
    EXPECT_EQ(size_t(1), elementSet2.size());
    EXPECT_STREQ("3", elementSet2[0]->getString().c_str());
}
