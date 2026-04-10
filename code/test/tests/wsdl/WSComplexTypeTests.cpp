/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/wsdl/WSComplexType.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

namespace {

class TestComplexType final : public WSComplexType
{
public:
    WSString a {"a", false};
    WSString b {"b", false};
    WSString attr {"attr", false};

    explicit TestComplexType(const char* elementName = "TestComplexType", bool optional = false)
        : WSComplexType(elementName, optional)
    {
        static const Strings elementNames {"a", "b"};
        static const Strings attributeNames {"attr"};

        setElements(elementNames, {&a, &b});
        setAttributes(attributeNames, {&attr});
    }

    void checkRestrictions() const override
    {
        // no restrictions for tests
    }
};

} // namespace
namespace sptk {

TEST(WSComplexTypeTests,defaultIsNull)
{
    TestComplexType obj("T", false);
    EXPECT_TRUE(obj.isNull());
}

TEST(WSComplexTypeTests,optionalAndNullExportToDoesNotCreateNode)
{
    Document    doc(Node::Type::Object);
    const auto& root = doc.root();

    TestComplexType obj("T", true);
    ASSERT_TRUE(obj.isNull());

    obj.exportTo(root);

    EXPECT_TRUE(root->nodes().empty());
}

TEST(WSComplexTypeTests,nonOptionalExportToCreatesChildWithTypeNameEvenWhenCustomNameProvided)
{
    Document    doc(Node::Type::Object);
    const auto& root = doc.root();

    TestComplexType obj("TypeName", false);
    obj.a = "value-a";
    obj.b = "value-b";
    obj.attr = "value-attr";

    ASSERT_FALSE(obj.isNull());

    obj.exportTo(root, "CustomElementName");

    const auto& children = root->nodes();
    ASSERT_EQ(1U, children.size());
    ASSERT_NE(nullptr, children[0]);

    EXPECT_STREQ("CustomElementName", children[0]->getQualifiedName().c_str());

    EXPECT_EQ("value-a", children[0]->getString("a"));
    EXPECT_EQ("value-b", children[0]->getString("b"));
    EXPECT_TRUE(children[0]->attributes().have("attr"));
    EXPECT_EQ("value-attr", children[0]->attributes().get("attr"));
}

TEST(WSComplexTypeTests,exportToUsesProvidedNameWhenParentIsArray)
{
    Document    doc(Node::Type::Array);
    const auto& root = doc.root();

    TestComplexType obj("TypeName", false);
    obj.a = "value-a";
    obj.b = "value-b";

    obj.exportTo(root, "ArrayElementName");

    const auto& children = root->nodes();
    ASSERT_EQ(1U, children.size());
    ASSERT_NE(nullptr, children[0]);

    EXPECT_EQ("ArrayElementName", children[0]->getQualifiedName());
    EXPECT_EQ("value-a", children[0]->getString("a"));
    EXPECT_EQ("value-b", children[0]->getString("b"));
}

TEST(WSComplexTypeTests,loadFromNodeLoadsElementsAndAttributes)
{
    Document    doc(Node::Type::Object);
    const auto& root = doc.root();

    auto t = root->pushNode("T", Node::Type::Object);
    t->set("a", "in-a");
    t->set("b", "in-b");
    t->attributes().set("attr", "in-attr");

    TestComplexType obj("T", false);
    obj.load(t);

    EXPECT_EQ("in-a", obj.a.asString());
    EXPECT_EQ("in-b", obj.b.asString());
    EXPECT_EQ("in-attr", obj.attr.asString());
}

} // namespace sptk_test
