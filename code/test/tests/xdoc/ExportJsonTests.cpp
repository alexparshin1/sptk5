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

#include "test/TestData.h"
#include <sptk5/xdoc/ExportJSON.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;
namespace sptk {

TEST(XDocumentTests,xmlToJson)
{
    auto testFile = TestData::DataDirectory() / "menu.xml";
    if (!filesystem::exists(testFile))
    {
        GTEST_SKIP() << "File " << testFile.string() << " not found";
    }

    xdoc::Document document;
    Buffer         buffer;

    buffer.loadFromFile(testFile);

    document.load(buffer, false);
    document.exportTo(xdoc::DataFormat::JSON, buffer, true);
}

TEST(XDocumentTests,exportJsonAttributesOnly)
{
    SNode node = Node::createNode("root", Node::Type::Object);
    node->attributes().set("attr1", "val1");

    Buffer buffer;
    ExportJSON::exportToJSON(node.get(), buffer, false);
    string json = static_cast<String>(buffer);

    EXPECT_EQ(json, "{\"attributes\":{\"attr1\":\"val1\"},\"value\":{}}");
}

TEST(XDocumentTests,exportJsonSpecialChars)
{
    SNode node = Node::createNode("root", Node::Type::Text);
    node->set("Special \n\t\"\\ characters \x01\x1F");

    Buffer buffer;
    xdoc::ExportJSON::exportToJSON(node.get(), buffer, false);
    string json = static_cast<String>(buffer);

    EXPECT_EQ(json, "\"Special \\n\\t\\\"\\\\ characters \\u0001\\u001f\"");
}

TEST(XDocumentTests,exportJsonNumbers)
{
    SNode node = Node::createNode("root", Node::Type::Number);
    node->set(123.456);

    Buffer buffer;
    ExportJSON::exportToJSON(node.get(), buffer, false);
    string json = static_cast<String>(buffer);
    EXPECT_EQ(json, "123.456");

    node->set(123.0);
    ExportJSON::exportToJSON(node.get(), buffer, false);
    json = static_cast<String>(buffer);
    EXPECT_EQ(json, "123");
}

} // namespace sptk_test
