/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/xdoc/XMLDocType.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

TEST(SPTK_XmlXMLDocType, parseEntity)
{
    Entity entity;

    entity.parse(R"(<!ENTITY file_pic SYSTEM "file.jpg" NDATA jpg>)");
    EXPECT_STREQ(entity.name.c_str(), "file_pic");
    EXPECT_TRUE(entity.type == Entity::Type::SYSTEM);
    EXPECT_STREQ(entity.resource.c_str(), "file.jpg");

    entity.parse(R"(<!ENTITY % lists "ul | ol")");
    EXPECT_STREQ(entity.name.c_str(), "%lists");
    EXPECT_TRUE(entity.type == Entity::Type::SYSTEM);
    EXPECT_STREQ(entity.resource.c_str(), "ul | ol");

    entity.parse(R"(<!ENTITY % lists PUBLIC list_id "ul | ol")");
    EXPECT_STREQ(entity.name.c_str(), "%lists");
    EXPECT_TRUE(entity.type == Entity::Type::PUBLIC);
    EXPECT_STREQ(entity.id.c_str(), "list_id");
    EXPECT_STREQ(entity.resource.c_str(), "ul | ol");
}

TEST(SPTK_XmlXMLDocType, decodeEncodeEntities)
{
    const String testString1("<'test1'> value");
    const String testString2(R"(<v a='test1'>value</v>)");

    Buffer encoded;
    Buffer decoded;
    XMLDocType docType("x");

    docType.encodeEntities(testString1.c_str(), encoded);
    docType.decodeEntities(encoded.c_str(), static_cast<uint32_t>(encoded.size()), decoded);
    EXPECT_STREQ(testString1.c_str(), decoded.c_str());

    encoded.reset();
    decoded.reset();

    docType.encodeEntities(testString2.c_str(), encoded);
    docType.decodeEntities(encoded.c_str(), static_cast<uint32_t>(encoded.size()), decoded);
    EXPECT_STREQ(testString2.c_str(), decoded.c_str());
}
