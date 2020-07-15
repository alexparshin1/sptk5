/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/MemoryDS.h>

using namespace std;
using namespace sptk;

Field& MemoryDS::operator[](const String& field_name)
{
    UniqueLock(m_mutex);
    if (m_current == m_list.end())
        throw Exception("At the end of the data");
    auto& row = *(*m_current);
    return row[field_name];
}

size_t MemoryDS::recordCount() const
{
    SharedLock(m_mutex);
    return m_list.size();
}

// how many fields do we have in the current record?

size_t MemoryDS::fieldCount() const
{
    SharedLock(m_mutex);
    if (m_current == m_list.end())
        throw Exception("At the end of the data");
    return (*m_current)->size();
}

// access to the field by number, 0..field.size()-1
Field& MemoryDS::operator[](size_t index)
{
    SharedLock(m_mutex);
    if (m_current == m_list.end())
        throw Exception("At the end of the data");
    auto& row = *(*m_current);
    return row[index];
}

// read this field data into external value
bool MemoryDS::readField(const char* fname, Variant& fvalue)
{
    SharedLock(m_mutex);
    try {
        fvalue = *(Variant*) &(*this)[fname];
    }
    catch (const Exception&) {
        return false;
    }
    return true;
}

// write this field data from external value
bool MemoryDS::writeField(const char* fname, const Variant& fvalue)
{
    UniqueLock(m_mutex);
    try {
        (*this)[fname] = fvalue;
    }
    catch (const Exception&) {
        return false;
    }
    return true;
}

bool MemoryDS::open()
{
    if (m_list.empty())
        m_current = m_list.end();
    else
        m_current = m_list.begin();
    return true;
}

bool MemoryDS::close()
{
    MemoryDS::clear();
    m_current = m_list.end();
    return true;
}

bool MemoryDS::first()
{
    UniqueLock(m_mutex);

    if (!m_list.empty()) {
        m_current = m_list.begin();
        return true;
    }
    m_current = m_list.end();
    return false;
}

bool MemoryDS::last()
{
    UniqueLock(m_mutex);

    if (!m_list.empty()) {
        int index = m_list.size() - 1;
        m_current = m_list.begin() + index;
        return true;
    }
    m_current = m_list.end();
    return false;
}

bool MemoryDS::next()
{
    UniqueLock(m_mutex);

    if (m_current != m_list.end()) {
        ++m_current;
        return true;
    }
    return false;
}

bool MemoryDS::prior()
{
    UniqueLock(m_mutex);

    if (m_current != m_list.begin()) {
        --m_current;
        return true;
    }
    return false;
}

bool MemoryDS::find(const String& fieldName, const Variant& position)
{
    SharedLock(m_mutex);

    String value = position.asString();
    for (auto itor = m_list.begin(); itor != m_list.end(); ++itor) {
        FieldList& entry = *(*itor);
        if (entry[fieldName].asString() == value) {
            m_current = itor;
            return true;
        }
    }

    return false;
}

void MemoryDS::clear()
{
    UniqueLock(m_mutex);

    for (auto* row: m_list)
        delete row;
    m_list.clear();

    m_current = m_list.end();
}

void MemoryDS::push_back(FieldList* fieldList)
{
    UniqueLock(m_mutex);
    m_list.push_back(fieldList);
    if (m_list.size() == 1)
        m_current = m_list.begin();
}

bool MemoryDS::empty() const
{
    SharedLock(m_mutex);
    return m_list.empty();
}

#if USE_GTEST

static struct {
    String  name;
    int     age {0};
} people[] = {
    { "John", 30 },
    { "Jane", 28 },
    { "Bob", 6 }
};

TEST(SPTK_MemoryDS, createAndVerify)
{
    MemoryDS ds;

    EXPECT_TRUE(ds.empty());

    for (int i = 0; i < 3; i++) {
        auto* row = new FieldList;

        auto* name = new Field("name");
        *name = people[i].name;
        row->push_back(name);

        auto* age = new Field("age");
        *age = people[i].age;
        row->push_back(age);

        ds.push_back(row);
    }

    EXPECT_EQ(ds.recordCount(), size_t(3));

    ds.open();

    int i = 0;
    while (!ds.eof()) {
        EXPECT_EQ(ds.fieldCount(), size_t(2));
        EXPECT_STREQ(ds["name"].asString().c_str(), people[i].name.c_str());
        EXPECT_EQ(ds["age"].asInteger(), people[i].age);
        ++i;
        ds.next();
    }

    EXPECT_FALSE(ds.find("age", 31));

    EXPECT_TRUE(ds.find("age", 28));
    EXPECT_STREQ(ds["name"].asString().c_str(), "Jane");
    EXPECT_EQ(ds[1].asInteger(), 28);

    ds.prior();
    EXPECT_STREQ(ds["name"].asString().c_str(), "John");

    ds.last();
    EXPECT_STREQ(ds["name"].asString().c_str(), "Bob");

    ds.first();
    EXPECT_STREQ(ds["name"].asString().c_str(), "John");

    ds.close();

    ds.clear();
    EXPECT_TRUE(ds.empty());
}

#endif