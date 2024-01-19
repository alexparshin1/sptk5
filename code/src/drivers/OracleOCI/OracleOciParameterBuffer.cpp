/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "sptk5/db/QueryParameter.h"
#include <sptk5/db/OracleOciParameterBuffer.h>

using namespace std;
using namespace sptk;

OracleOciParameterBuffer::OracleOciParameterBuffer(VariantDataType type)
    : m_bindType(type)
    , m_bindBuffer(nullptr)
{
    switch (m_bindType)
    {
        using enum VariantDataType;
        case VAR_INT:
            m_bindBuffer = makeBuffer<int>();
            break;
        case VAR_INT64:
        case VAR_FLOAT:
            m_bindBuffer = makeBuffer<double>();
            break;
        case VAR_STRING: {
            auto* str = new ocilib::ostring();
            str->reserve(MaxStringLength);
            m_bindBuffer = bit_cast<uint8_t*>(str);
            break;
        }
        default:
            throw Exception("Unknown parameter data type.");
    }
}

OracleOciParameterBuffer::~OracleOciParameterBuffer()
{
    switch (m_bindType)
    {
        using enum VariantDataType;
        case VAR_INT:
            deleteBuffer<int>(m_bindBuffer);
            break;
        case VAR_INT64:
        case VAR_FLOAT:
            deleteBuffer<double>(m_bindBuffer);
            break;
        case VAR_STRING:
            delete bit_cast<ocilib::ostring*>(m_bindBuffer);
            break;
        default:
            break;
    }
}

void OracleOciParameterBuffer::setValue(const QueryParameter& value)
{
    if (value.dataType() != m_bindType)
    {
        throw Exception("Parameter data type has changed.");
    }
    switch (m_bindType)
    {
        using enum VariantDataType;
        case VAR_INT:
            setValue(value.asInteger());
            break;
        case VAR_INT64:
        case VAR_FLOAT:
            setValue(value.asFloat());
            break;
        case VAR_STRING:
            if (value.dataSize() > MaxStringLength)
            {
                throw Exception("Parameter data size is too big.");
            }
            setValue(value.asString());
            break;
        default:
            throw Exception("Unknown parameter data type.");
    }
}

void OracleOciParameterBuffer::bind(ocilib::Statement statement, const ocilib::ostring& parameterMark, ocilib::BindInfo::BindDirectionValues bindDirection)
{
    switch (m_bindType)
    {
        using enum VariantDataType;
        case VAR_INT:
            statement.Bind(parameterMark, getValue<int>(), bindDirection);
            break;
        case VAR_INT64:
        case VAR_FLOAT:
            statement.Bind(parameterMark, getValue<double>(), bindDirection);
            break;
        case VAR_STRING:
            statement.Bind(parameterMark, getValue<ocilib::ostring>(), MaxStringLength, bindDirection);
            break;
        default:
            throw Exception("Unknown parameter data type.");
    }
}
