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

#include <sptk5/MemoryDS.h>

using namespace std;
using namespace sptk;

Field& MemoryDS::operator[](const String& field_name)
{
    const scoped_lock lock(m_mutex);
    if (m_current == m_list.end())
    {
        throw Exception("At the end of the data");
    }
    auto& row = *m_current;
    return row[field_name];
}

size_t MemoryDS::recordCount() const
{
    const scoped_lock lock(m_mutex);
    return m_list.size();
}

// how many fields do we have in the current record?

size_t MemoryDS::fieldCount() const
{
    const scoped_lock lock(m_mutex);
    if (m_current == m_list.end())
    {
        throw Exception("At the end of the data");
    }
    return m_current->size();
}

// access to the field by number, 0..field.size()-1
Field& MemoryDS::operator[](size_t index)
{
    const scoped_lock lock(m_mutex);
    if (m_current == m_list.end())
    {
        throw Exception("At the end of the data");
    }
    auto& row = *m_current;
    return row[(int) index];
}

// read this field data into external value
bool MemoryDS::readField(const char* fieldName, Variant& fieldValue)
{
    const scoped_lock lock(m_mutex);
    try
    {
        fieldValue = *(Variant*) &(*this)[fieldName];
    }
    catch (const Exception&)
    {
        return false;
    }
    return true;
}

// write this field data from external value
bool MemoryDS::writeField(const char* fieldName, const Variant& fieldValue)
{
    const scoped_lock lock(m_mutex);
    try
    {
        (*this)[fieldName] = fieldValue;
    }
    catch (const Exception&)
    {
        return false;
    }
    return true;
}

bool MemoryDS::open()
{
    const scoped_lock lock(m_mutex);
    if (m_list.empty())
    {
        m_current = m_list.end();
    }
    else
    {
        m_current = m_list.begin();
    }
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
    const scoped_lock lock(m_mutex);

    if (!m_list.empty())
    {
        m_current = m_list.begin();
        return true;
    }
    m_current = m_list.end();
    return false;
}

bool MemoryDS::last()
{
    const scoped_lock lock(m_mutex);

    if (!m_list.empty())
    {
        m_current = m_list.end();
        m_current--;
        return true;
    }
    m_current = m_list.end();
    return false;
}

bool MemoryDS::next()
{
    const scoped_lock lock(m_mutex);

    if (m_current != m_list.end())
    {
        ++m_current;
        return true;
    }
    return false;
}

bool MemoryDS::prior()
{
    const scoped_lock lock(m_mutex);

    if (m_current != m_list.begin())
    {
        --m_current;
        return true;
    }
    return false;
}

bool MemoryDS::find(const String& fieldName, const Variant& position)
{
    const scoped_lock lock(m_mutex);

    const String value = position.asString();
    for (auto itor = m_list.begin(); itor != m_list.end(); ++itor)
    {
        FieldList& entry = *itor;
        if (entry[fieldName].asString() == value)
        {
            m_current = itor;
            return true;
        }
    }

    return false;
}

void MemoryDS::clear()
{
    const scoped_lock lock(m_mutex);

    m_list.clear();

    m_current = m_list.end();
}

void MemoryDS::push_back(FieldList&& fieldList)
{
    const scoped_lock lock(m_mutex);

    m_list.push_back(std::move(fieldList));
    if (m_list.size() == 1)
    {
        m_current = m_list.begin();
    }
}

bool MemoryDS::empty() const
{
    const scoped_lock lock(m_mutex);
    return m_list.empty();
}
