/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
                             VariantDataType dataType, int fieldLength, int fieldScale)
    : Field(fName.c_str())
    , m_fldType(fieldType)
    , m_fldColumn(fieldColumn)
    , m_fldSize(fieldLength)
    , m_fldScale(fieldScale)
{
    displayName(fName);

    switch (dataType)
    {
        case VariantDataType::VAR_BOOL:
            Variant::setBool(false);
            view().width = 6;
            break;

        case VariantDataType::VAR_INT:
            Variant::setInteger(0);
            view().width = 10;
            break;

        case VariantDataType::VAR_FLOAT:
            Variant::setFloat(0);
            view().width = 16;
            view().precision = (unsigned) fieldScale;
            break;

        case VariantDataType::VAR_STRING:
            Variant::setString("");
            if (fieldLength == 0)
            {
                fieldLength = 256;
                m_fldSize = fieldLength;
            }
            checkSize((size_t) fieldLength + 1);
            view().width = fieldLength;
            break;

        case VariantDataType::VAR_TEXT:
            Variant::setBuffer((const uint8_t*) "", 1, VariantDataType::VAR_TEXT);
            checkSize((size_t) fieldLength + 1);
            view().width = fieldLength;
            break;

        case VariantDataType::VAR_BUFFER:
            Variant::setBuffer((const uint8_t*) "", 1, VariantDataType::VAR_BUFFER);
            checkSize((size_t) fieldLength);
            view().width = 1;
            break;

        case VariantDataType::VAR_DATE:
        case VariantDataType::VAR_DATE_TIME:
            Variant::setDateTime(DateTime());
            Field::dataType(dataType);
            view().width = 10;
            break;

        case VariantDataType::VAR_INT64:
            Variant::setInt64(0);
            view().width = 16;
            break;

        default:
            Variant::setString("");
            checkSize((size_t) fieldLength + 1);
            view().width = fieldLength;
            break;
    }
}

void DatabaseField::checkSize(size_t sz)
{
    m_data.get<Buffer>().checkSize(sz);
}
