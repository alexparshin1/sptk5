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
#include <sptk5/xdoc/Attributes.h>

using namespace std;
using namespace sptk;
using namespace xdoc;
namespace sptk {

TEST(XDocumentTests,getSetAttributes)
{
    Attributes attributes;

    attributes.set("name", "John");
    attributes.set("position", "engineer");

    EXPECT_TRUE(attributes.have("name"));
    EXPECT_STREQ("John", attributes.get("name").c_str());
    EXPECT_STREQ("engineer", attributes.get("position").c_str());

    attributes.set("position", "janitor");
    EXPECT_STREQ("janitor", attributes.get("position").c_str());
    EXPECT_EQ(attributes.size(), 2U);
}

TEST(XDocumentTests,getWithDefaultAndMissingKeys)
{
    Attributes attributes;
    attributes.set("name", "John");

    EXPECT_FALSE(attributes.have("missing"));
    EXPECT_STREQ("", attributes.get("missing").c_str());
    EXPECT_STREQ("fallback", attributes.get("missing", "fallback").c_str());
}

TEST(XDocumentTests,clearAndEmptyState)
{
    Attributes attributes;
    attributes.set("name", "John");
    attributes.set("role", "admin");

    ASSERT_FALSE(attributes.empty());
    ASSERT_EQ(2U, attributes.size());

    attributes.clear();
    EXPECT_TRUE(attributes.empty());
    EXPECT_EQ(0U, attributes.size());
    EXPECT_STREQ("fallback", attributes.get("name", "fallback").c_str());
}

TEST(XDocumentTests,setReturnsSelfForChaining)
{
    Attributes attributes;
    Attributes& returned = attributes.set("name", "John").set("role", "admin").set("name", "Alex");

    EXPECT_EQ(&attributes, &returned);
    EXPECT_EQ(2U, attributes.size());
    EXPECT_STREQ("Alex", attributes.get("name").c_str());
    EXPECT_STREQ("admin", attributes.get("role").c_str());
}

TEST(XDocumentTests,preservesInsertionOrderAndValueUpdate)
{
    Attributes attributes;
    attributes.set("first", "1");
    attributes.set("second", "2");
    attributes.set("third", "3");
    attributes.set("second", "updated");

    vector<pair<String, String>> items(attributes.begin(), attributes.end());
    ASSERT_EQ(3U, items.size());

    EXPECT_STREQ("first", items[0].first.c_str());
    EXPECT_STREQ("1", items[0].second.c_str());
    EXPECT_STREQ("second", items[1].first.c_str());
    EXPECT_STREQ("updated", items[1].second.c_str());
    EXPECT_STREQ("third", items[2].first.c_str());
    EXPECT_STREQ("3", items[2].second.c_str());
}

} // namespace sptk_test
