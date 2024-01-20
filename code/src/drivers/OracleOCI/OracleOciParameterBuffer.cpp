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

OracleOciParameterBuffer::OracleOciParameterBuffer(VariantDataType type, const std::shared_ptr<ocilib::Connection>& connection)
    : m_bindType(type)
{
    ocilib::ostring* str = nullptr;
    switch (m_bindType)
    {
        using enum VariantDataType;
        case VAR_INT:
            m_bindBuffer = makeBuffer<int>();
            break;
        case VAR_INT64:
            m_bindBuffer = makeBuffer<big_int>();
            break;
        case VAR_FLOAT:
            m_bindBuffer = makeBuffer<double>();
            break;
        case VAR_STRING:
            str = new ocilib::ostring();
            str->reserve(MaxStringLength);
            m_bindBuffer = bit_cast<uint8_t*>(str);
            break;
        case VAR_BOOL:
            m_bindBuffer = makeBuffer<int>();
            break;
        case VAR_DATE_TIME:
        case VAR_DATE:
            m_bindBuffer = makeBuffer<ocilib::Date>(true);
            break;
        case VariantDataType::VAR_TEXT:
            m_bindBuffer = makeBuffer<ocilib::Clob>(*connection);
            break;
        case VariantDataType::VAR_BUFFER:
            m_bindBuffer = makeBuffer<ocilib::Blob>(*connection);
            break;
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
            deleteBuffer<big_int>(m_bindBuffer);
            break;
        case VAR_FLOAT:
            deleteBuffer<double>(m_bindBuffer);
            break;
        case VAR_STRING:
            deleteBuffer<ocilib::ostring>(m_bindBuffer);
            break;
        case VAR_BOOL:
            deleteBuffer<bool>(m_bindBuffer);
            break;
        case VAR_DATE_TIME:
        case VAR_DATE:
            deleteBuffer<ocilib::Date>(m_bindBuffer);
            break;
        case VAR_TEXT:
            deleteBuffer<ocilib::Clob>(m_bindBuffer);
            break;
        case VAR_BUFFER:
            deleteBuffer<ocilib::Blob>(m_bindBuffer);
            break;
        default:
            break;
    }
}

void OracleOciParameterBuffer::setValue(const QueryParameter& value)
{
    if (value.dataType() != m_bindType)
    {
        throw DatabaseException("Parameter data type has changed.");
    }
    switch (m_bindType)
    {
        using enum VariantDataType;
        case VAR_INT:
            setValue(value.asInteger());
            break;
        case VAR_INT64:
            setValue<big_int>(value.asInt64());
            break;
        case VAR_FLOAT:
            setValue(value.asFloat());
            break;
        case VAR_STRING:
            if (value.dataSize() > MaxStringLength)
            {
                throw Exception("Parameter data size is too big.");
            }
            setValue<ocilib::ostring>(value.asString());
            break;
        case VAR_BOOL:
            getValue<int>() = value.asBool();
            break;
        case VAR_DATE_TIME: {
            ocilib::Date date;
            auto dateValue = value.asDateTime();
            short year;
            short month;
            short day;
            short hour;
            short minute;
            short wday;
            short yearDate;
            dateValue.decodeDate(&year, &month, &day, &wday, &yearDate, false);
            short second;
            short millisecond;
            dateValue.decodeTime(&hour, &minute, &second, &millisecond, false);
            getValue<ocilib::Date>().SetDateTime(year, month, day, hour, minute, second);
            break;
        }
        case VAR_DATE: {
            auto dateValue = value.asDateTime();
            short year;
            short month;
            short day;
            short wday;
            short yearDate;
            dateValue.decodeDate(&year, &month, &day, &wday, &yearDate, false);
            getValue<ocilib::Date>().SetDateTime(year, month, day, 0, 0, 0);
            break;
        }
        case VAR_TEXT:
            getValue<ocilib::Clob>().Write(value.getText());
            break;
        case VAR_BUFFER: {
            vector<uint8_t> view(value.getText(), value.getText() + value.dataSize());
            getValue<ocilib::Blob>().Write(view);
            break;
        }
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
            statement.Bind(parameterMark, getValue<big_int>(), bindDirection);
            break;
        case VAR_FLOAT:
            statement.Bind(parameterMark, getValue<double>(), bindDirection);
            break;
        case VAR_STRING:
            statement.Bind(parameterMark, getValue<ocilib::ostring>(), MaxStringLength, bindDirection);
            break;
        case VAR_BOOL:
            statement.Bind(parameterMark, getValue<int>(), bindDirection);
            break;
        case VAR_DATE_TIME:
        case VAR_DATE: {
            auto& date = getValue<ocilib::Date>();
            cout << date.ToString() << endl;
            statement.Bind(parameterMark, getValue<ocilib::Date>(), bindDirection);
        }
        break;
        case VAR_TEXT:
            statement.Bind(parameterMark, getValue<ocilib::Clob>(), bindDirection);
            break;
        case VAR_BUFFER:
            statement.Bind(parameterMark, getValue<ocilib::Blob>(), bindDirection);
            break;
        default:
            throw Exception("Unknown parameter data type.");
    }
}
