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

#include <iomanip>
#include <sptk5/db/DatabaseField.h>

using namespace std;
using namespace sptk;

DatabaseField::DatabaseField(string_view fName, int fieldType,
                             VariantDataType dataType, int fieldLength, int fieldScale)
    : Field(fName.data())
    , m_fldType(fieldType)
    , m_fldSize(fieldLength)
    , m_fldScale(fieldScale)
{
    displayName(fName.data());

    switch (dataType)
    {
        using enum VariantDataType;
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
            view().precision = static_cast<unsigned>(fieldScale);
            break;

        case VAR_STRING:
            if (fieldLength == 0)
            {
                fieldLength = 256;
                m_fldSize = fieldLength;
            }
            Variant::setBuffer(reinterpret_cast<const uint8_t*>(""), 1, VAR_BUFFER);
            checkSize(static_cast<size_t>(fieldLength) + 1);
            view().width = dataType == VAR_BUFFER ? 1 : fieldLength;
            break;

        case VAR_TEXT:
        case VAR_BUFFER:
            Variant::setBuffer(reinterpret_cast<const uint8_t*>(""), 1, dataType);
            checkSize(static_cast<size_t>(fieldLength) + 1);
            view().width = dataType == VAR_BUFFER ? 1 : fieldLength;
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
            checkSize(static_cast<size_t>(fieldLength) + 1);
            view().width = fieldLength;
            break;
    }
}

void DatabaseField::checkSize(size_t sz)
{
    m_data.get<Buffer>().checkSize(sz);
}

String DatabaseField::doubleDataToString() const
{
    stringstream output;
    output << fixed << setprecision(m_fldScale) << m_data.get<double>();
    return output.str();
}

void DatabaseField::setNull(VariantDataType vtype)
{
    m_data.setNull(true, vtype, false);
}
