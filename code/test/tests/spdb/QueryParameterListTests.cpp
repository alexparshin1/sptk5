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
#include <sptk5/db/QueryParameterList.h>

using namespace std;
using namespace sptk;

namespace {
class TestQueryParameterList : public QueryParameterList
{
public:
    using QueryParameterList::add;
};
} // namespace

TEST(SPTK_QueryParameterList, addFindAndAccessAreCaseInsensitive)
{
    TestQueryParameterList params;
    const auto             p = make_shared<QueryParameter>("Person_ID");
    params.add(p);

    EXPECT_EQ(params.size(), 1);
    EXPECT_TRUE(params.find("person_id") != nullptr);
    EXPECT_TRUE(params.find("PERSON_ID") != nullptr);
    EXPECT_EQ(params["PERSON_ID"].name(), "person_id");
}

TEST(SPTK_QueryParameterList, removeUpdatesBothListAndIndex)
{
    TestQueryParameterList params;
    params.add(make_shared<QueryParameter>("id"));
    params.add(make_shared<QueryParameter>("name"));

    params.remove(0);

    EXPECT_EQ(params.size(), 1);
    EXPECT_TRUE(params.find("id") == nullptr);
    EXPECT_TRUE(params.find("name") != nullptr);
    EXPECT_EQ(params[0].name(), "name");
}

TEST(SPTK_QueryParameterList, removeInvalidIndexThrows)
{
    TestQueryParameterList params;
    params.add(make_shared<QueryParameter>("id"));

    EXPECT_THROW(params.remove(2), Exception);
}

TEST(SPTK_QueryParameterList, operatorByInvalidNameThrows)
{
    TestQueryParameterList params;
    params.add(make_shared<QueryParameter>("id"));

    EXPECT_THROW((void) params["missing"], Exception);
}

TEST(SPTK_QueryParameterList, enumerateReturnsItemsByBindIndex)
{
    TestQueryParameterList params;

    const auto p1 = make_shared<QueryParameter>("p1");
    p1->bindAdd(0);
    p1->bindAdd(2);
    params.add(p1);

    const auto p2 = make_shared<QueryParameter>("p2");
    p2->bindAdd(1);
    params.add(p2);

    ParamVector enumerated;
    params.enumerate(enumerated);

    ASSERT_EQ(enumerated.size(), 3);
    EXPECT_EQ(enumerated[0], p1);
    EXPECT_EQ(enumerated[1], p2);
    EXPECT_EQ(enumerated[2], p1);
}

TEST(SPTK_QueryParameterList, enumerateEmptyListReturnsEmptyVector)
{
    const TestQueryParameterList params;
    ParamVector                  enumerated {make_shared<QueryParameter>("seed")};

    params.enumerate(enumerated);

    EXPECT_TRUE(enumerated.empty());
}
