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

#include "PostgreSQLParamValues.h"
#include "htonq.h"
#include <format>

using namespace std;
using namespace sptk;

void PostgreSQLParamValues::setParameters(const QueryParameterList& params)
{
    params.enumerate(m_params);
    m_count = m_params.size();
    resize(m_count);
    for (size_t i = 0; i < m_count; ++i)
    {
        using enum VariantDataType;
        const auto&           param = m_params[i];
        const VariantDataType ptype = param->dataType();
        PostgreSQLDataType    pgDataType;
        PostgreSQLConnection::variantTypeToPostgreType(ptype, pgDataType, param->name());
        m_types[i] = static_cast<Oid>(pgDataType);

        if ((static_cast<int>(ptype) &
             (static_cast<int>(VAR_INT) | static_cast<int>(VAR_INT64) | static_cast<int>(VAR_FLOAT) |
              static_cast<int>(VAR_BUFFER) | static_cast<int>(VAR_DATE) |
              static_cast<int>(VAR_DATE_TIME))) != 0)
        {
            m_formats[i] = 1; // Binary format
        }
        else
        {
            m_formats[i] = 0; // Text format
        }

        m_values[i] = param->conversionBuffer(); // This is a default. For VariantDataType::VAR_STRING, VariantDataType::VAR_TEXT, VariantDataType::VAR_BUFFER, and it would be replaced later

        switch (ptype)
        {
            case VAR_BOOL:
                m_lengths[i] = sizeof(bool);
                break;

            case VAR_INT:
                m_lengths[i] = sizeof(int32_t);
                break;

            case VAR_DATE:
            case VAR_DATE_TIME:
                m_lengths[i] = sizeof(int64_t);
                break;

            case VAR_FLOAT:
                m_lengths[i] = sizeof(double);
                break;

            case VAR_INT64:
                m_lengths[i] = sizeof(int64_t);
                break;

            default:
                m_lengths[i] = 0;
                break;
        }
    }
}

void PostgreSQLParamValues::setFloatParameterValue(const unsigned paramIndex, const SQueryParameter& param)
{
    auto  value = param->asFloat();
    void* ptr = &value;
    auto* uptrBuffer64 = bit_cast<uint64_t*>(param->conversionBuffer());
    *uptrBuffer64 = htonq(*bit_cast<uint64_t*>(ptr));
    setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1, PostgreSQLDataType::FLOAT8);
}

void PostgreSQLParamValues::setParameterValue(const unsigned paramIndex, const SQueryParameter& param)
{
    const VariantDataType ptype = param->dataType();

    if (param->isNull())
    {
        PostgreSQLDataType pgDataType;
        PostgreSQLConnection::variantTypeToPostgreType(ptype, pgDataType, param->name());
        setParameterValue(paramIndex, nullptr, 0, 1, pgDataType);
    }
    else
    {
        constexpr auto    hoursInDay {24};
        uint32_t*         uptrBuffer;
        uint64_t*         uptrBuffer64;
        uint64_t          days {0};
        int64_t           mcs {0};
        constexpr int64_t secondsPerDay {86400};
        constexpr int64_t microsecondsInSecond {1000000};
        switch (ptype)
        {
            case VariantDataType::VAR_BOOL:
                if (param->asBool())
                {
                    setParameterValue(paramIndex, (const uint8_t*) "t", 1, 0, PostgreSQLDataType::VARCHAR);
                }
                else
                {
                    setParameterValue(paramIndex, (const uint8_t*) "f", 1, 0, PostgreSQLDataType::VARCHAR);
                }
                break;

            case VariantDataType::VAR_INT:
                uptrBuffer = bit_cast<uint32_t*>(param->conversionBuffer());
                *uptrBuffer = htonl(static_cast<uint32_t>(param->get<int>()));
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int32_t), 1,
                                  PostgreSQLDataType::INT4);
                break;

            case VariantDataType::VAR_DATE:
                days = chrono::duration_cast<chrono::hours>(param->get<DateTime>() - m_epochDate).count() / hoursInDay;
                if (m_int64timestamps)
                {
                    int64_t dt = days * secondsPerDay * microsecondsInSecond;
                    htonq_inplace(bit_cast<uint64_t*>(&dt), bit_cast<uint64_t*>(param->conversionBuffer()));
                }
                else
                {
                    auto dt = static_cast<double>(days) * static_cast<double>(secondsPerDay);
                    htonq_inplace(bit_cast<uint64_t*>(&dt), bit_cast<uint64_t*>(param->conversionBuffer()));
                }
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1,
                                  PostgreSQLDataType::TIMESTAMP);
                break;

            case VariantDataType::VAR_DATE_TIME:
                mcs = chrono::duration_cast<chrono::microseconds>(param->get<DateTime>() - m_epochDate).count();
                if (m_int64timestamps)
                {
                    htonq_inplace(bit_cast<uint64_t*>(&mcs), bit_cast<uint64_t*>(param->conversionBuffer()));
                }
                else
                {
                    auto dt = static_cast<double>(mcs) / static_cast<double>(microsecondsInSecond);
                    htonq_inplace(bit_cast<uint64_t*>(&dt), bit_cast<uint64_t*>(param->conversionBuffer()));
                }
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1,
                                  PostgreSQLDataType::TIMESTAMP);
                break;

            case VariantDataType::VAR_INT64:
                uptrBuffer64 = bit_cast<uint64_t*>(param->conversionBuffer());
                *uptrBuffer64 = htonq(static_cast<uint64_t>(param->get<int64_t>()));
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1,
                                  PostgreSQLDataType::INT8);
                break;

            case VariantDataType::VAR_MONEY:
                setFloatParameterValue(paramIndex, param);
                break;

            case VariantDataType::VAR_FLOAT:
                uptrBuffer64 = bit_cast<uint64_t*>(param->conversionBuffer());
                *uptrBuffer64 = htonq(*bit_cast<const uint64_t*>(&param->get<double>()));
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1,
                                  PostgreSQLDataType::FLOAT8);
                break;

            case VariantDataType::VAR_STRING:
            case VariantDataType::VAR_TEXT:
                setParameterValue(paramIndex, bit_cast<const uint8_t*>(param->getText()), static_cast<unsigned>(param->dataSize()), 0,
                                  PostgreSQLDataType::VARCHAR);
                break;

            case VariantDataType::VAR_BUFFER:
                setParameterValue(paramIndex, bit_cast<const uint8_t*>(param->getText()),
                                  static_cast<unsigned>(param->dataSize()), 1, PostgreSQLDataType::BYTEA);
                break;

            default:
                throw DatabaseException(
                    format("Unsupported parameter type({}) for parameter '{}'", static_cast<int>(param->dataType()), param->name()));
        }
    }
}
