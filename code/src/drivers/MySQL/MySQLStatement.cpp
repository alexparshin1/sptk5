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

#include <sptk5/FieldList.h>
#include <sptk5/db/MySQLConnection.h>

using namespace std;
using namespace sptk;

// When TEXT field is large, fetch in chunks:
constexpr unsigned FETCH_BUFFER = 256;
constexpr unsigned SMALL_BUFFER = 16;

#ifndef MYSQL_HAS_MYBOOL
using my_bool = bool;
constexpr my_bool MY_BOOL_FALSE = false;
#else
constexpr my_bool MY_BOOL_FALSE = 0;
#endif

// MySQL-specific database field
class sptk::MySQLStatementField
    : public DatabaseField
{
public:
    MySQLStatementField(const string& fieldName, enum_field_types fieldType, VariantDataType dataType, int fieldSize)
        : DatabaseField(fieldName, static_cast<int>(fieldType), dataType, fieldSize)
    {
    }

    using DatabaseField::operator=;

    void bindCallbacks(MYSQL_BIND* bind)
    {
        bind->length = &m_cbLength;
        bind->is_null = &m_cbNull;
        bind->error = &m_cbError;
    }

    MYSQL_TIME& getTimeBuffer()
    {
        return m_timeBuffer;
    }

    char* getTempBuffer()
    {
        return m_tempBuffer.data();
    }

private:
    // Callback variables
    unsigned long m_cbLength {0};
    my_bool m_cbNull {MY_BOOL_FALSE};
    my_bool m_cbError {MY_BOOL_FALSE};

    // MySQL time conversion buffer
    MYSQL_TIME m_timeBuffer {};
    array<char, SMALL_BUFFER> m_tempBuffer {};
};


MySQLStatement::MySQLStatement(MySQLConnection* connection, String sql, bool autoPrepare)
    : DatabaseStatement<MySQLConnection, MYSQL_STMT>(connection)
    , m_sql(std::move(sql))
{
    if (autoPrepare)
    {
        auto* stmt = mysql_stmt_init(bit_cast<MYSQL*>(connection->handle()));
        m_stmt = shared_ptr<MYSQL_STMT>(stmt, [](auto* ptr) {
            mysql_stmt_close(ptr);
        });
        statement(m_stmt.get());
    }
    else
    {
        statement(nullptr); // direct execution
    }
}

void MySQLStatement::dateTimeToMySQLDate(MYSQL_TIME& mysqlDate, const DateTime& timestamp, VariantDataType timeType)
{
    short year = 0;
    short month = 0;
    short day = 0;
    short weekDay = 0;
    short yearDay = 0;
    short hour = 0;
    short minute = 0;
    short second = 0;
    short millisecond = 0;

    memset(&mysqlDate, 0, sizeof(MYSQL_TIME));
    timestamp.decodeDate(&year, &month, &day, &weekDay, &yearDay);
    mysqlDate.year = static_cast<unsigned>(year);
    mysqlDate.month = static_cast<unsigned>(month);
    mysqlDate.day = static_cast<unsigned>(day);
    if (timeType == VariantDataType::VAR_DATE)
    {
        mysqlDate.time_type = MYSQL_TIMESTAMP_DATE;
    }
    else
    {
        timestamp.decodeTime(&hour, &minute, &second, &millisecond);
        mysqlDate.hour = static_cast<unsigned>(hour);
        mysqlDate.minute = static_cast<unsigned>(minute);
        mysqlDate.second = static_cast<unsigned>(second);
        mysqlDate.second_part = static_cast<unsigned>(millisecond);
        mysqlDate.time_type = MYSQL_TIMESTAMP_DATETIME;
    }
}

void MySQLStatement::enumerateParams(QueryParameterList& queryParams)
{
    DatabaseStatement<MySQLConnection, MYSQL_STMT>::enumerateParams(queryParams);
    const auto paramCount = enumeratedParams().size();
    m_paramBuffers.resize(paramCount);
    m_paramLengths.resize(paramCount);
    MYSQL_BIND* paramBuffers = m_paramBuffers.data();
    if (paramCount != 0)
    {
        memset(paramBuffers, 0, sizeof(MYSQL_BIND) * paramCount);
        for (unsigned paramIndex = 0; paramIndex < paramCount; ++paramIndex)
        {
            m_paramBuffers[paramIndex].length = &m_paramLengths[paramIndex];
        }
    }
}

VariantDataType MySQLStatement::mySQLTypeToVariantType(enum_field_types mysqlType)
{
    switch (mysqlType)
    {
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_TINY:
            return VariantDataType::VAR_BOOL;

        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_LONG:
            return VariantDataType::VAR_INT;

        case MYSQL_TYPE_LONGLONG:
            return VariantDataType::VAR_INT64;

        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_NEWDECIMAL:
            return VariantDataType::VAR_FLOAT;

        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
            return VariantDataType::VAR_BUFFER;

        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_DATE:
            return VariantDataType::VAR_DATE;

        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_TIMESTAMP:
            return VariantDataType::VAR_DATE_TIME;

        // Anything we don't know about - treat as string
        default:
            return VariantDataType::VAR_STRING;
    }
}

enum_field_types MySQLStatement::variantTypeToMySQLType(VariantDataType dataType)
{
    switch (dataType)
    {
        case VariantDataType::VAR_NONE:
            return MYSQL_TYPE_VARCHAR;

        case VariantDataType::VAR_BOOL:
            return MYSQL_TYPE_TINY;

        case VariantDataType::VAR_INT:
            return MYSQL_TYPE_LONG;

        case VariantDataType::VAR_FLOAT:
            return MYSQL_TYPE_DOUBLE;

        case VariantDataType::VAR_STRING:
            return MYSQL_TYPE_STRING;

        case VariantDataType::VAR_TEXT:
        case VariantDataType::VAR_BUFFER:
            return MYSQL_TYPE_BLOB;

        case VariantDataType::VAR_DATE:
        case VariantDataType::VAR_DATE_TIME:
            return MYSQL_TYPE_TIMESTAMP;

        case VariantDataType::VAR_INT64:
            return MYSQL_TYPE_LONGLONG;

            // Anything we don't know about - treat as string
        default:
            return MYSQL_TYPE_STRING;
    }
}

void MySQLStatement::setParameterValues()
{
    static my_bool nullValue = true;

    const auto paramCount = enumeratedParams().size();
    for (unsigned paramIndex = 0; paramIndex < paramCount; ++paramIndex)
    {
        auto* param = enumeratedParams()[paramIndex].get();
        MYSQL_BIND& bind = m_paramBuffers[paramIndex];

        bind.buffer = nullptr;
        bind.buffer_type = variantTypeToMySQLType(param->dataType());

        switch (param->dataType())
        {
            case VariantDataType::VAR_BOOL:
                m_paramLengths[paramIndex] = 0;
                bind.buffer = static_cast<void*>(&param->get<bool>());
                break;

            case VariantDataType::VAR_INT:
                m_paramLengths[paramIndex] = 0;
                bind.buffer = static_cast<void*>(&param->get<int>());
                break;

            case VariantDataType::VAR_FLOAT:
                m_paramLengths[paramIndex] = 0;
                bind.buffer = static_cast<void*>(&param->get<double>());
                break;

            case VariantDataType::VAR_INT64:
                m_paramLengths[paramIndex] = 0;
                bind.buffer = static_cast<void*>(&param->get<int64_t>());
                break;

            case VariantDataType::VAR_STRING:
                m_paramLengths[paramIndex] = static_cast<unsigned long>(param->dataSize());
                if (param->isNull())
                {
                    bind.buffer = nullptr;
                }
                else
                {
                    bind.buffer = bit_cast<void*>(param->get<String>().c_str());
                }
                break;

            case VariantDataType::VAR_TEXT:
            case VariantDataType::VAR_BUFFER:
                m_paramLengths[paramIndex] = static_cast<unsigned long>(param->dataSize());
                if (param->isNull())
                {
                    bind.buffer = nullptr;
                }
                else
                {
                    bind.buffer = bit_cast<void*>(param->getText());
                }
                break;

            case VariantDataType::VAR_DATE:
            case VariantDataType::VAR_DATE_TIME:
                m_paramLengths[paramIndex] = sizeof(MYSQL_TIME);
                bind.buffer = static_cast<void*>(param->conversionBuffer());
                if (param->isNull())
                {
                    m_paramLengths[paramIndex] = 0;
                }
                else
                {
                    dateTimeToMySQLDate(*static_cast<MYSQL_TIME*>(bind.buffer), param->get<DateTime>(), param->dataType());
                }
                break;

            default:
                throw DatabaseException(
                    "Unsupported parameter type(" + to_string(static_cast<int>(param->dataType())) + ") for parameter '" +
                    param->name() + "'");
        }

        if (param->isNull())
        {
            bind.is_null = &nullValue;
        }
        else
        {
            bind.is_null = nullptr;
        }
        bind.error = nullptr;
    }

    // Bind the buffers
    if (mysql_stmt_bind_param(statement(), m_paramBuffers.data()) != 0)
    {
        throwMySQLError();
    }
}

void MySQLStatement::MySQLStatement::prepare(const String& sql) const
{
    if (mysql_stmt_prepare(statement(), sql.c_str(), static_cast<unsigned long>(sql.length())) != 0)
    {
        throwMySQLError();
    }
}

void MySQLStatement::execute(bool)
{
    state().eof = false;
    m_result.reset();
    if (statement() != nullptr)
    {
        if (mysql_stmt_execute(statement()) != 0)
        {
            throwMySQLError();
        }
        state().columnCount = mysql_stmt_field_count(statement());
        if (state().columnCount != 0U)
        {
            auto* result = mysql_stmt_result_metadata(statement());
            m_result = shared_ptr<MYSQL_RES>(result, [](auto* ptr) {
                mysql_free_result(ptr);
            });
        }
    }
    else
    {
        MYSQL* conn = connection()->m_connection.get();
        if (mysql_query(conn, m_sql.c_str()) != 0)
        {
            const String error(mysql_error(conn));
            throw DatabaseException(error);
        }
        state().columnCount = mysql_field_count(conn);
        if (state().columnCount != 0U)
        {
            auto* result = mysql_store_result(conn);
            m_result = shared_ptr<MYSQL_RES>(result, [](auto* ptr) {
                mysql_free_result(ptr);
            });
        }
    }
}

void MySQLStatement::bindResult(FieldList& fields)
{
    fields.clear();
    if (m_result == nullptr)
    {
        return;
    }

    for (unsigned columnIndex = 0; columnIndex < state().columnCount; ++columnIndex)
    {
        const MYSQL_FIELD* fieldMetadata = mysql_fetch_field(m_result.get());
        if (fieldMetadata == nullptr)
        {
            throwMySQLError();
        }

        String columnName = fieldMetadata->name;
        if (columnName.empty())
        {
            columnName = "column_" + to_string(columnIndex + 1);
        }

        const VariantDataType fieldType = mySQLTypeToVariantType(fieldMetadata->type);
        auto fieldLength = static_cast<unsigned>(fieldMetadata->length);
        if (fieldLength > FETCH_BUFFER)
        {
            fieldLength = FETCH_BUFFER;
        }

        auto field = make_shared<MySQLStatementField>(columnName, fieldMetadata->type, fieldType, static_cast<int>(fieldLength));
        fields.push_back(field);
    }

    if (statement() != nullptr)
    {
        // Bind initialized fields to MySQL bind buffers
        m_fieldBuffers.resize(state().columnCount);
        for (unsigned columnIndex = 0; columnIndex < state().columnCount; ++columnIndex)
        {
            auto* field = dynamic_cast<MySQLStatementField*>(&fields[static_cast<int>(columnIndex)]);
            MYSQL_BIND& bind = m_fieldBuffers[columnIndex];

            bind.buffer_type = static_cast<enum_field_types>(field->fieldType());

            switch (bind.buffer_type)
            {
                // Fixed length buffer - integers
                case MYSQL_TYPE_BIT:
                case MYSQL_TYPE_TINY:
                    bind.buffer = static_cast<void*>(&field->get<bool>());
                    bind.buffer_length = sizeof(bool);
                    break;

                case MYSQL_TYPE_SHORT:
                case MYSQL_TYPE_YEAR:
                    bind.buffer = static_cast<void*>(&field->get<int>());
                    bind.buffer_length = sizeof(int32_t);
                    break;

                    // Fixed length buffer - floats
                case MYSQL_TYPE_FLOAT:
                case MYSQL_TYPE_DOUBLE:
                    bind.buffer = static_cast<void*>(&field->get<double>());
                    bind.buffer_length = sizeof(double);
                    break;

                    // Fixed length date buffer
                case MYSQL_TYPE_DATE:
                case MYSQL_TYPE_DATETIME:
                case MYSQL_TYPE_TIME:
                case MYSQL_TYPE_TIMESTAMP:
                    bind.buffer = static_cast<void*>(&field->getTimeBuffer());
                    bind.buffer_length = sizeof(MYSQL_TIME);
                    break;

                    // Fixed length buffer - long integers
                case MYSQL_TYPE_LONG:
                    bind.buffer = static_cast<void*>(&field->get<int>());
                    bind.buffer_length = sizeof(int);
                    break;

                case MYSQL_TYPE_LONGLONG:
                    bind.buffer = static_cast<void*>(&field->get<int64_t>());
                    bind.buffer_length = sizeof(int64_t);
                    break;

                    // Using temp buffer of the size defined by field size
                case MYSQL_TYPE_NEWDECIMAL:
                    bind.buffer_length = field->fieldSize();
                    bind.buffer = static_cast<void*>(field->getTempBuffer());
                    break;

                    // Variable length buffer - will be extended during fetch if needed
                default:
                    bind.buffer_length = field->fieldSize();
                    bind.buffer = bit_cast<void*>(field->getText());
                    break;
            }

            field->bindCallbacks(&bind);
        }
        if (mysql_stmt_bind_result(statement(), m_fieldBuffers.data()) != 0)
        {
            throwMySQLError();
        }
    }
}

void MySQLStatement::readResultRow(FieldList& fields)
{
    if (statement() != nullptr)
    {
        readPreparedResultRow(fields);
    }
    else
    {
        readUnpreparedResultRow(fields);
    }
}

void MySQLStatement::readUnpreparedResultRow(FieldList& fields) const
{
    const auto fieldCount = static_cast<int>(fields.size());
    const auto* lengths = mysql_fetch_lengths(m_result.get());

    for (int fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
    {

        auto* field = dynamic_cast<MySQLStatementField*>(&fields[fieldIndex]);

        const VariantDataType fieldType = field->dataType();

        const char* data = m_row[fieldIndex];
        if (data == nullptr)
        {
            // Field data is null, no more processing of the field
            field->setNull(fieldType);
            continue;
        }

        const auto dataLength = static_cast<uint32_t>(lengths[fieldIndex]);

        switch (fieldType)
        {

            case VariantDataType::VAR_BOOL:
                field->setBool(strchr("YyTt1", data[0]) != nullptr);
                break;

            case VariantDataType::VAR_INT:
                field->setInteger(string2int(data));
                break;

            case VariantDataType::VAR_DATE:
                field->setDateTime(DateTime(data), true);
                break;

            case VariantDataType::VAR_DATE_TIME:
                if (strncmp(data, "0000-00", 7) == 0)
                {
                    field->setNull(VariantDataType::VAR_DATE_TIME);
                }
                else
                {
                    field->setDateTime(DateTime(data));
                }
                break;

            case VariantDataType::VAR_FLOAT:
                field->setFloat(string2double(data));
                break;

            case VariantDataType::VAR_STRING:
            case VariantDataType::VAR_TEXT:
            case VariantDataType::VAR_BUFFER:
                field->setBuffer(reinterpret_cast<const uint8_t*>(data), dataLength, fieldType);
                break;

            case VariantDataType::VAR_INT64:
                field->setInt64(string2int64(data));
                break;

            default:
                throw DatabaseException("Unsupported Variant type: " + int2string(static_cast<int>(fieldType)));
        }
    }
}

void MySQLStatement::decodeMySQLTime(Field* _field, const MYSQL_TIME& mysqlTime, VariantDataType fieldType)
{
    auto* field = dynamic_cast<MySQLStatementField*>(_field);
    if (mysqlTime.day == 0 && mysqlTime.month == 0)
    {
        // Date returned as 0000-00-00
        field->setNull(fieldType);
    }
    else
    {
        const DateTime dt(static_cast<short>(mysqlTime.year), static_cast<short>(mysqlTime.month), static_cast<short>(mysqlTime.day),
                          static_cast<short>(mysqlTime.hour), static_cast<short>(mysqlTime.minute), static_cast<short>(mysqlTime.second));
        field->setDateTime(dt, fieldType == VariantDataType::VAR_DATE);
        field->setDataSize(sizeof(int64_t));
    }
}

void MySQLStatement::decodeMySQLFloat(Field* _field, MYSQL_BIND& bind)
{
    auto* field = dynamic_cast<MySQLStatementField*>(_field);
    if (bind.buffer_type == MYSQL_TYPE_NEWDECIMAL)
    {
        const double value = string2double(static_cast<char*>(bind.buffer));
        field->setFloat(value);
    }
    else
    {
        const auto dataLength = static_cast<uint32_t>(*(bind.length));
        if (dataLength == sizeof(float))
        {
            const float value = *static_cast<float*>(bind.buffer);
            field->setFloat(value);
        }
        field->setDataSize(dataLength);
    }
}

void MySQLStatement::readPreparedResultRow(FieldList& fields)
{
    const auto fieldCount = static_cast<int>(fields.size());
    bool fieldSizeChanged = false;
    for (int fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
    {
        auto* field = dynamic_cast<MySQLStatementField*>(&fields[fieldIndex]);
        MYSQL_BIND& bind = m_fieldBuffers[fieldIndex];

        const VariantDataType fieldType = field->dataType();

        if (*(bind.is_null))
        {
            // Field data is null, no more processing
            field->setNull(fieldType);
            continue;
        }

        const auto dataLength = static_cast<uint32_t>(*(bind.length));

        switch (fieldType)
        {
            case VariantDataType::VAR_BOOL:
            case VariantDataType::VAR_INT:
            case VariantDataType::VAR_INT64:
                field->setDataSize(dataLength);
                break;

            case VariantDataType::VAR_DATE:
            case VariantDataType::VAR_DATE_TIME:
                decodeMySQLTime(field, *static_cast<MYSQL_TIME*>(bind.buffer), fieldType);
                break;

            case VariantDataType::VAR_FLOAT:
                decodeMySQLFloat(field, bind);
                break;

            case VariantDataType::VAR_STRING:
            case VariantDataType::VAR_TEXT:
            case VariantDataType::VAR_BUFFER:
                fieldSizeChanged = bindVarCharField(bind, field, static_cast<size_t>(fieldIndex), dataLength);
                break;

            default:
                throw DatabaseException("Unsupported Variant type: " + int2string(static_cast<int>(fieldType)));
        }
    }

    if (fieldSizeChanged && mysql_stmt_bind_result(statement(), m_fieldBuffers.data()) != 0)
    {
        throwMySQLError();
    }
}

bool MySQLStatement::bindVarCharField(MYSQL_BIND& bind, MySQLStatementField* field, size_t fieldIndex,
                                      uint32_t dataLength) const
{
    bool fieldSizeChanged = false;

    if (field->isNull() || field->bufferSize() < dataLength)
    {
        if (field->isNull())
        {
            field->setBuffer(nullptr, dataLength, VariantDataType::VAR_BUFFER);
        }
        else
        {
            /// Fetch truncated, enlarge buffer and fetch again
            field->checkSize(dataLength);
        }
        bind.buffer = field->get<Buffer>().data();
        bind.buffer_length = static_cast<unsigned long>(field->bufferSize());
        if (mysql_stmt_fetch_column(statement(), &bind, static_cast<unsigned>(fieldIndex), 0) != 0)
        {
            throwMySQLError();
        }
        fieldSizeChanged = true;
    }

    static_cast<char*>(bind.buffer)[dataLength] = 0;
    field->setDataSize(dataLength);

    return fieldSizeChanged;
}

void MySQLStatement::close()
{
    if (m_result != nullptr)
    {
        // Read all the results until EOF
        while (!state().eof)
        {
            fetch();
        }
        m_result.reset();
    }
}

void MySQLStatement::fetch()
{
    if (statement() != nullptr)
    {
        switch (mysql_stmt_fetch(statement()))
        {
            case 0:                    // successful, the data has been fetched to application data buffers
            case MYSQL_DATA_TRUNCATED: // successful, but one or mode fields were truncated
                state().eof = false;
                break;

            case MYSQL_NO_DATA: // All data fetched
                state().eof = true;
                break;

            default: // Error during fetch, retrieving error
                throwMySQLError();
        }
    }
    else
    {
        m_row = mysql_fetch_row(m_result.get());
        if (m_row == nullptr)
        {
            if (mysql_errno(connection()->m_connection.get()) != 0)
            {
                throwMySQLError();
            }
            state().eof = true;
        }
    }
}
