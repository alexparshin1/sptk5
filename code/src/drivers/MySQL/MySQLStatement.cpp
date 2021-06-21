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

#include <sptk5/FieldList.h>
#include <sptk5/db/MySQLConnection.h>

using namespace std;
using namespace sptk;

// When TEXT field is large, fetch in chunks:
constexpr unsigned FETCH_BUFFER = 256;
constexpr unsigned SMALL_BUFFER = 16;

#if MYSQL_HAS_MYBOOL == 0
using my_bool = bool;
constexpr my_bool MY_BOOL_FALSE = false;
#else
constexpr my_bool MY_BOOL_FALSE = 0;
#endif

// MySQL-specific database field
class sptk::MySQLStatementField : public DatabaseField
{
public:

    MySQLStatementField(const string& fieldName, int fieldColumn, enum_field_types fieldType, VariantType dataType,
                        int fieldSize)
            : DatabaseField(fieldName, fieldColumn, (int) fieldType, dataType, fieldSize)
    {
    }

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
    unsigned long               m_cbLength {0};
    my_bool                     m_cbNull {MY_BOOL_FALSE};
    my_bool                     m_cbError {MY_BOOL_FALSE};

    // MySQL time conversion buffer
    MYSQL_TIME                  m_timeBuffer {};
    array<char, SMALL_BUFFER>   m_tempBuffer {};
};


MySQLStatement::MySQLStatement(MySQLConnection* connection, String sql, bool autoPrepare)
        : DatabaseStatement<MySQLConnection, MYSQL_STMT>(connection), m_sql(move(sql))
{
    if (autoPrepare) {
        auto* stmt = mysql_stmt_init((MYSQL*) connection->handle());
        m_stmt = shared_ptr<MYSQL_STMT>(stmt, [](auto* ptr) { mysql_stmt_close(ptr); });
        statement(m_stmt.get());
    } else {
        statement(nullptr); // direct execution
    }
}

void MySQLStatement::dateTimeToMySQLDate(MYSQL_TIME& mysqlDate, DateTime timestamp, VariantType timeType)
{
    short year = 0;
    short month = 0;
    short day = 0;
    short wday = 0;
    short yday = 0;
    short hour = 0;
    short minute = 0;
    short second = 0;
    short msecond = 0;

    memset(&mysqlDate, 0, sizeof(MYSQL_TIME));
    timestamp.decodeDate(&year, &month, &day, &wday, &yday);
    mysqlDate.year = (unsigned) year;
    mysqlDate.month = (unsigned) month;
    mysqlDate.day = (unsigned) day;
    if (timeType == VAR_DATE)
        mysqlDate.time_type = MYSQL_TIMESTAMP_DATE;
    else {
        timestamp.decodeTime(&hour, &minute, &second, &msecond);
        mysqlDate.hour = (unsigned) hour;
        mysqlDate.minute = (unsigned) minute;
        mysqlDate.second = (unsigned) second;
        mysqlDate.second_part = (unsigned) msecond;
        mysqlDate.time_type = MYSQL_TIMESTAMP_DATETIME;
    }
}

void MySQLStatement::mysqlDateToDateTime(DateTime& timestamp, const MYSQL_TIME& mysqlDate)
{
    if (mysqlDate.time_type == MYSQL_TIMESTAMP_DATE)
        timestamp = DateTime(
                (short) mysqlDate.year, (short) mysqlDate.month, (short) mysqlDate.day,
                (short) 0, (short) 0, (short) 0);
    else
        timestamp = DateTime(
                (short) mysqlDate.year, (short) mysqlDate.month, (short) mysqlDate.day,
                (short) mysqlDate.hour, (short) mysqlDate.minute, (short) mysqlDate.second);
}

void MySQLStatement::enumerateParams(QueryParameterList& queryParams)
{
    DatabaseStatement<MySQLConnection, MYSQL_STMT>::enumerateParams(queryParams);
    auto paramCount = enumeratedParams().size();
    m_paramBuffers.resize(paramCount);
    m_paramLengths.resize(paramCount);
    MYSQL_BIND* paramBuffers = &m_paramBuffers[0];
    if (paramCount != 0) {
        memset(paramBuffers, 0, sizeof(MYSQL_BIND) * paramCount);
        for (unsigned paramIndex = 0; paramIndex < paramCount; ++paramIndex) {
            m_paramBuffers[paramIndex].length = &m_paramLengths[paramIndex];
        }
    }
}

VariantType MySQLStatement::mySQLTypeToVariantType(enum_field_types mysqlType)
{
    switch (mysqlType) {

        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_TINY:
            return VAR_BOOL;

        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_LONG:
            return VAR_INT;

        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_NEWDECIMAL:
            return VAR_FLOAT;

        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_BLOB:
            return VAR_BUFFER;

        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_DATE:
            return VAR_DATE;

        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_TIMESTAMP:
            return VAR_DATE_TIME;

        case MYSQL_TYPE_LONGLONG:
            return VAR_INT64;

            // Anything we don't know about - treat as string
        default:
            return VAR_STRING;
    }
}

enum_field_types MySQLStatement::variantTypeToMySQLType(VariantType dataType)
{
    switch (dataType) {

        case VAR_NONE:
            return MYSQL_TYPE_VARCHAR;

        case VAR_BOOL:
            return MYSQL_TYPE_TINY;

        case VAR_INT:
            return MYSQL_TYPE_LONG;

        case VAR_FLOAT:
            return MYSQL_TYPE_DOUBLE;

        case VAR_STRING:
            //return MYSQL_TYPE_VARCHAR; // Stopped working after upgrade to MariaDB?
            return MYSQL_TYPE_STRING;

        case VAR_TEXT:
        case VAR_BUFFER:
            return MYSQL_TYPE_BLOB;

        case VAR_DATE:
        case VAR_DATE_TIME:
            return MYSQL_TYPE_TIMESTAMP;

        case VAR_INT64:
            return MYSQL_TYPE_LONGLONG;

            // Anything we don't know about - treat as string
        default:
            return MYSQL_TYPE_STRING;
    }
}

void MySQLStatement::setParameterValues()
{
    static my_bool nullValue = true;

    auto paramCount = enumeratedParams().size();
    for (unsigned paramIndex = 0; paramIndex < paramCount; ++paramIndex) {
        QueryParameter* param = enumeratedParams()[paramIndex];
        bool setNull = param->isNull();
        MYSQL_BIND& bind = m_paramBuffers[paramIndex];

        bind.buffer = (void*) param->getBuffer();
        bind.buffer_type = variantTypeToMySQLType(param->dataType());

        switch (param->dataType()) {

            case VAR_NONE:
                m_paramLengths[paramIndex] = 0;
                param->setNull();
                break;

            case VAR_BOOL:
            case VAR_INT:
            case VAR_FLOAT:
            case VAR_INT64:
                m_paramLengths[paramIndex] = 0;
                bind.buffer = (void*) &param->getInt64();
                break;

            case VAR_STRING:
            case VAR_TEXT:
            case VAR_BUFFER:
                m_paramLengths[paramIndex] = ULONG_CAST(param->dataSize());
                break;

            case VAR_DATE:
            case VAR_DATE_TIME:
                m_paramLengths[paramIndex] = sizeof(MYSQL_TIME);
                bind.buffer = (void*) param->conversionBuffer();
                if (param->isNull())
                    m_paramLengths[paramIndex] = 0;
                else
                    dateTimeToMySQLDate(*(MYSQL_TIME*) bind.buffer, param->getDateTime(), param->dataType());
                break;

            default:
                throw DatabaseException(
                        "Unsupported parameter type(" + to_string(param->dataType()) + ") for parameter '" +
                        param->name() + "'");
        }
        if (setNull)
            bind.is_null = &nullValue;
        else
            bind.is_null = nullptr;
        bind.error = nullptr;
    }
    /// Bind the buffers
    if (mysql_stmt_bind_param(statement(), &m_paramBuffers[0]) != 0)
        throwMySQLError();
}

void MySQLStatement::MySQLStatement::prepare(const String& sql) const
{
    if (mysql_stmt_prepare(statement(), sql.c_str(), ULONG_CAST(sql.length())) != 0)
        throwMySQLError();
}

void MySQLStatement::execute(bool)
{
    state().eof = false;
    m_result.reset();
    if (statement() != nullptr) {
        if (mysql_stmt_execute(statement()) != 0)
            throwMySQLError();
        state().columnCount = mysql_stmt_field_count(statement());
        if (state().columnCount != 0U) {
            auto* result = mysql_stmt_result_metadata(statement());
            m_result = shared_ptr<MYSQL_RES>(result, [](auto* ptr) { mysql_free_result(ptr); });
        }
    } else {
        MYSQL* conn = connection()->m_connection.get();
        if (mysql_query(conn, m_sql.c_str()) != 0) {
            string error = mysql_error(conn);
            throw DatabaseException(error);
        }
        state().columnCount = mysql_field_count(conn);
        if (state().columnCount != 0U) {
            auto* result = mysql_store_result(conn);
            m_result = shared_ptr<MYSQL_RES>(result, [](auto* ptr) { mysql_free_result(ptr); });
        }
    }
}

void MySQLStatement::bindResult(FieldList& fields)
{
    fields.clear();
    if (m_result == nullptr)
        return;

    String columnName;
    for (unsigned columnIndex = 0; columnIndex < state().columnCount; ++columnIndex) {
        const MYSQL_FIELD* fieldMetadata = mysql_fetch_field(m_result.get());
        if (fieldMetadata == nullptr)
            throwMySQLError();

        columnName = fieldMetadata->name;
        if (columnName.empty())
            columnName = "column_" + to_string(columnIndex + 1);

        VariantType fieldType = mySQLTypeToVariantType(fieldMetadata->type);
        auto fieldLength = (unsigned) fieldMetadata->length;
        if (fieldLength > FETCH_BUFFER)
            fieldLength = FETCH_BUFFER;
        fields.push_back(new MySQLStatementField(columnName, (int) columnIndex, fieldMetadata->type, fieldType,
                                                 (int) fieldLength));
    }

    if (statement() != nullptr) {
        // Bind initialized fields to MySQL bind buffers
        m_fieldBuffers.resize(state().columnCount);
        for (unsigned columnIndex = 0; columnIndex < state().columnCount; ++columnIndex) {
            auto* field = (MySQLStatementField*) &fields[(int) columnIndex];
            MYSQL_BIND& bind = m_fieldBuffers[columnIndex];

            bind.buffer_type = (enum_field_types) field->fieldType();

            switch (bind.buffer_type) {
                // Fixed length buffer - integers
                case MYSQL_TYPE_BIT:
                case MYSQL_TYPE_TINY:
                case MYSQL_TYPE_SHORT:
                case MYSQL_TYPE_YEAR:
                    bind.buffer = (void*) &field->getInteger();
                    bind.buffer_length = sizeof(int32_t);
                    break;

                    // Fixed length buffer - floats
                case MYSQL_TYPE_FLOAT:
                case MYSQL_TYPE_DOUBLE:
                    bind.buffer = (void*) &field->getFloat();
                    bind.buffer_length = sizeof(double);
                    break;

                    // Fixed length date buffer
                case MYSQL_TYPE_DATE:
                case MYSQL_TYPE_DATETIME:
                case MYSQL_TYPE_TIME:
                case MYSQL_TYPE_TIMESTAMP:
                    bind.buffer = (void*) &field->getTimeBuffer();
                    bind.buffer_length = sizeof(MYSQL_TIME);
                    break;

                    // Fixed length buffer - long integers
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_LONGLONG:
                    bind.buffer = (void*) &field->getInt64();
                    bind.buffer_length = sizeof(uint64_t);
                    break;

                    // Using temp buffer of the size defined by field size
                case MYSQL_TYPE_NEWDECIMAL:
                    bind.buffer_length = field->fieldSize();
                    bind.buffer = (void*) field->getTempBuffer();
                    break;

                    // Variable length buffer - will be extended during fetch if needed
                default:
                    bind.buffer_length = field->fieldSize();
                    bind.buffer = (void*) field->getBuffer();
                    break;
            }

            field->bindCallbacks(&bind);
        }
        if (mysql_stmt_bind_result(statement(), &m_fieldBuffers[0]) != 0)
            throwMySQLError();
    }
}

void MySQLStatement::readResultRow(FieldList& fields)
{
    if (statement() != nullptr)
        readPreparedResultRow(fields);
    else
        readUnpreparedResultRow(fields);
}

void MySQLStatement::readUnpreparedResultRow(FieldList& fields) const
{
    auto fieldCount = (int) fields.size();
    const auto* lengths = mysql_fetch_lengths(m_result.get());

    for (int fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex) {

        auto* field = (MySQLStatementField*) &fields[fieldIndex];

        VariantType fieldType = field->dataType();

        const char* data = m_row[fieldIndex];
        if (data == nullptr) {
            // Field data is null, no more processing of the field
            field->setNull(fieldType);
            continue;
        }

        auto dataLength = (uint32_t) lengths[fieldIndex];

        switch (fieldType) {

            case VAR_BOOL:
                field->setBool(strchr("YyTt1", data[0]) != nullptr);
                break;

            case VAR_INT:
                field->setInteger(string2int(data));
                break;

            case VAR_DATE:
                field->setDateTime(DateTime(data), true);
                break;

            case VAR_DATE_TIME:
                if (strncmp(data, "0000-00", 7) == 0)
                    field->setNull(VAR_DATE_TIME);
                else
                    field->setDateTime(DateTime(data));
                break;

            case VAR_FLOAT:
                field->setFloat(string2double(data));
                break;

            case VAR_STRING:
            case VAR_TEXT:
            case VAR_BUFFER:
                field->setBuffer((const uint8_t*) data, dataLength, fieldType);
                break;

            case VAR_INT64:
                field->setInt64(string2int64(data));
                break;

            default: throwDatabaseException("Unsupported Variant type: " + int2string(fieldType))
        }
    }
}

void MySQLStatement::decodeMySQLTime(Field* _field, const MYSQL_TIME& mysqlTime, VariantType fieldType)
{
    auto* field = dynamic_cast<MySQLStatementField*>(_field);
    if (mysqlTime.day == 0 && mysqlTime.month == 0) {
        // Date returned as 0000-00-00
        field->setNull(fieldType);
    } else {
        DateTime dt(short(mysqlTime.year), short(mysqlTime.month), short(mysqlTime.day),
                    short(mysqlTime.hour), short(mysqlTime.minute), short(mysqlTime.second));
        field->setDateTime(dt, fieldType == VAR_DATE);
        field->setDataSize(sizeof(int64_t));
    }
}

void MySQLStatement::decodeMySQLFloat(Field* _field, MYSQL_BIND& bind)
{
    auto* field = dynamic_cast<MySQLStatementField*>(_field);
    if (bind.buffer_type == MYSQL_TYPE_NEWDECIMAL) {
        double value = string2double((char*) bind.buffer);
        field->setFloat(value);
    } else {
        auto dataLength = (uint32_t) *(bind.length);
        if (dataLength == sizeof(float)) {
            float value = *(float*) bind.buffer;
            field->setFloat(value);
        }
        field->setDataSize(dataLength);
    }
}

void MySQLStatement::readPreparedResultRow(FieldList& fields)
{
    auto fieldCount = (int) fields.size();
    bool fieldSizeChanged = false;
    for (int fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex) {
        auto* field = (MySQLStatementField*) &fields[fieldIndex];
        MYSQL_BIND& bind = m_fieldBuffers[fieldIndex];

        VariantType fieldType = field->dataType();

        if (*(bind.is_null)) {
            // Field data is null, no more processing
            field->setNull(fieldType);
            continue;
        }

        auto dataLength = (uint32_t) *(bind.length);

        switch (fieldType) {

            case VAR_BOOL:
            case VAR_INT:
                field->setDataSize(dataLength);
                break;

            case VAR_DATE:
            case VAR_DATE_TIME:
                decodeMySQLTime(field, *(MYSQL_TIME*) bind.buffer, fieldType);
                break;

            case VAR_FLOAT:
                decodeMySQLFloat(field, bind);
                break;

            case VAR_STRING:
            case VAR_TEXT:
            case VAR_BUFFER:
                fieldSizeChanged = bindVarCharField(bind, field, (size_t) fieldIndex, dataLength);
                break;

            case VAR_INT64:
                field->setDataSize(dataLength);
                break;

            default: throwDatabaseException("Unsupported Variant type: " + int2string(fieldType))
        }
    }

    if (fieldSizeChanged && mysql_stmt_bind_result(statement(), m_fieldBuffers.data()) != 0)
        throwMySQLError();
}

bool MySQLStatement::bindVarCharField(MYSQL_BIND& bind, MySQLStatementField* field, size_t fieldIndex,
                                      uint32_t dataLength) const
{
    bool fieldSizeChanged = false;
    if (bind.buffer_length < dataLength) {
        /// Fetch truncated, enlarge buffer and fetch again
        field->checkSize(dataLength);
        bind.buffer = field->getBuffer();
        bind.buffer_length = ULONG_CAST(field->bufferSize());
        if (mysql_stmt_fetch_column(statement(), &bind, (unsigned) fieldIndex, 0) != 0)
            throwMySQLError();
        fieldSizeChanged = true;
    }

    ((char*) bind.buffer)[dataLength] = 0;
    field->setDataSize(dataLength);

    return fieldSizeChanged;
}

void MySQLStatement::close()
{
    if (m_result != nullptr) {
        // Read all the results until EOF
        while (!state().eof) {
            fetch();
        }
        m_result.reset();
    }
}

void MySQLStatement::fetch()
{
    if (statement() != nullptr) {
        int rc = mysql_stmt_fetch(statement());
        switch (rc) {
            case 0: // Successful, the data has been fetched to application data buffers
            case MYSQL_DATA_TRUNCATED: // Successful, but one or mode fields were truncated
                state().eof = false;
                break;

            case MYSQL_NO_DATA: // All data fetched
                state().eof = true;
                break;

            default: // Error during fetch, retrieving error
                throwMySQLError();
        }
    } else {
        m_row = mysql_fetch_row(m_result.get());
        if (m_row == nullptr) {
            if (mysql_errno(connection()->m_connection.get()) != 0)
                throwMySQLError();
            state().eof = true;
        }
    }
}
