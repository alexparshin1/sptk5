/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-08                                             ║
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

#include <iomanip>
#include <sptk5/db/DatabaseField.h>
#include <sstream> // Included directly, not for this file alone: libstdc++ happens to
                   // reach <sstream> through other standard headers and libc++ does not,
                   // so without it this translation unit fails to compile on FreeBSD and
                   // anywhere else libc++ is the standard library. Do not remove it as
                   // redundant - it is only redundant on one implementation.

using namespace std;
using namespace sptk;

DatabaseField::DatabaseField(const String& fieldName, const int fieldType,
                             const VariantDataType dataType, size_t fieldLength, const int fieldScale)
    : Field(fieldName)
    , m_fldType(fieldType)
    , m_fldSize(fieldLength)
    , m_fldScale(fieldScale)
{
    displayName(fieldName);

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
            checkSize(fieldLength + 1);
            view().width = static_cast<int>(fieldLength);
            break;

        case VAR_TEXT:
        case VAR_BUFFER:
            Variant::setBuffer(reinterpret_cast<const uint8_t*>(""), 1, dataType);
            checkSize(fieldLength + 1);
            view().width = dataType == VAR_BUFFER ? 1 : static_cast<int>(fieldLength);
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
            checkSize(fieldLength + 1);
            view().width = static_cast<int>(fieldLength);
            break;
    }
}

void DatabaseField::checkSize(const size_t sz)
{
    m_data.get<Buffer>().reserve(sz);
}

String DatabaseField::doubleDataToString() const
{
    stringstream output;
    output << fixed << setprecision(m_fldScale) << m_data.get<double>();
    return output.str();
}

void DatabaseField::setNull(const VariantDataType vtype)
{
    m_data.setNull(true, vtype, false);
}
