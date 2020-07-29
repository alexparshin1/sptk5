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

#include <cstdlib>
#include <sptk5/db/DatabaseField.h>

using namespace std;
using namespace sptk;

DatabaseField::DatabaseField(const String& fName, int fieldColumn, int fieldType,
                             VariantType dataType, int fieldLength, int fieldScale)
: Field(fName.c_str()),
  m_fldType(fieldType),
  m_fldColumn(fieldColumn),
  m_fldSize(fieldLength),
  m_fldScale(fieldScale)
{
    displayName(fName);

    m_data.getBuffer().size = 0;

    switch (dataType)
    {
    case VAR_BOOL:
        Variant::setBool(false);
        view().width = 6;
        break;

    case VAR_INT:
        Variant::setInteger(0);
        view().width = 10;
        break;

    case VAR_FLOAT:
        Variant::setFloat(0);
        view().width = 16;
        view().precision = fieldScale;
        break;

    case VAR_STRING:
        Variant::setString("");
        checkSize((size_t)fieldLength + 1);
        view().width = fieldLength;
        break;

    case VAR_TEXT:
        Variant::setBuffer("", 1, VAR_TEXT);
        checkSize((size_t)fieldLength + 1);
        view().width = fieldLength;
        break;

    case VAR_BUFFER:
        Variant::setBuffer("", 1, VAR_BUFFER);
        checkSize((size_t)fieldLength);
        view().width = 1;
        break;

    case VAR_DATE:
    case VAR_DATE_TIME:
        Variant::setDateTime(DateTime());
        Field::dataType(dataType);
        view().width = 10;
        break;

    case VAR_INT64:
        Variant::setInt64(0);
        view().width = 16;
        break;

    default:
        Variant::setString("");
        checkSize((size_t)fieldLength + 1);
        view().width = fieldLength;
        break;
    }
}

void DatabaseField::checkSize(size_t sz)
{
    if (sz > m_data.getBuffer().size) {
        size_t newSize = (sz / 16 + 1) * 16;
        auto* p = new char[newSize + 1];
        if (p == nullptr)
            throw DatabaseException("Can't reallocate a buffer");
        if (m_data.getBuffer().data != nullptr) {
            memcpy(p, m_data.getBuffer().data, m_data.getBuffer().size);
            delete[] m_data.getBuffer().data;
        }
        m_data.getBuffer().data = p;
        m_data.getBuffer().size = newSize;
    }
}
