/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDatabaseField.cpp - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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
: Field(fName.c_str())
{
    m_fldType = fieldType;
    m_fldColumn = fieldColumn;
    m_fldSize = fieldLength;
    m_fldScale = fieldScale;
    visible = true;
    displayName = fName;
    alignment = ALIGN_LEFT;

    m_data.buffer.size = 0;

    switch (dataType)
    {
    case VAR_BOOL:
        setBool(false);
        view.width = 6;
        break;

    case VAR_INT:
        setInteger(0);
        view.width = 10;
        break;

    case VAR_FLOAT:
        setFloat(0);
        view.width = 16;
        view.precision = fieldScale;
        break;

    case VAR_STRING:
        setString("");
        checkSize((uint32_t)fieldLength + 1);
        view.width = fieldLength;
        break;

    case VAR_TEXT:
        setText("");
        checkSize((uint32_t)fieldLength + 1);
        view.width = fieldLength;
        break;

    case VAR_BUFFER:
        setBuffer("", 1);
        checkSize((uint32_t)fieldLength);
        view.width = 1;
        break;

    case VAR_DATE:
    case VAR_DATE_TIME:
        setDateTime(DateTime());
        Field::dataType(dataType);
        view.width = 10;
        break;

    case VAR_INT64:
        setInt64(0);
        view.width = 16;
        break;

    default:
        setString("");
        checkSize((uint32_t)fieldLength + 1);
        view.width = fieldLength;
        break;
    }
}

bool DatabaseField::checkSize(unsigned sz)
{
    if (sz > m_data.buffer.size) {
        unsigned newSize = (sz / 16 + 1) * 16;
        auto p = (char *) realloc(m_data.buffer.data, newSize + 1);
        if (p == nullptr)
            throw DatabaseException("Can't reallocate a buffer");
        m_data.buffer.data = p;
        m_data.buffer.size = newSize;
    }
    return true;
}
