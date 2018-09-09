/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CMySQLStatement.cpp - description                      ║
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

#include <sptk5/FieldList.h>
#include <sptk5/db/MySQLConnection.h>

using namespace std;
using namespace sptk;

#define throwMySQLError throw DatabaseException(mysql_stmt_error(m_statement))

// When TEXT field is large, fetch in chunks:
#define FETCH_BUFFER 256

class CMySQLStatementField: public DatabaseField
{
public:
    // Callback variables
    unsigned long   m_cbLength;
    my_bool         m_cbNull;
    my_bool         m_cbError;
    // MySQL time conversion buffer
    MYSQL_TIME      m_timeBuffer {};
    char*           m_tempBuffer;

public:
    CMySQLStatementField(const string& fieldName, int fieldColumn, enum_field_types fieldType, VariantType dataType, int fieldSize) :
        DatabaseField(fieldName, fieldColumn, (int) fieldType, dataType, fieldSize),
        m_cbLength(0), m_cbNull(0), m_cbError(0)
    {
        memset(&m_timeBuffer, 0, sizeof(MYSQL_TIME));
        if (fieldType == MYSQL_TYPE_NEWDECIMAL)
            m_tempBuffer = new char[fieldSize];
        else
            m_tempBuffer = nullptr;
    }

    ~CMySQLStatementField() override
    {
        delete [] m_tempBuffer;
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
        return m_tempBuffer;
    }
};


MySQLStatement::MySQLStatement(MySQLConnection* connection, const string& sql, bool autoPrepare)
: DatabaseStatement<MySQLConnection,MYSQL_STMT>(connection), m_sql(sql), m_result(nullptr), m_row{}
{
    if (autoPrepare)
        m_statement = mysql_stmt_init((MYSQL*)connection->handle());
    else
        m_statement = nullptr; // direct execution
}

MySQLStatement::~MySQLStatement()
{
    if (m_statement != nullptr)
       mysql_stmt_close(m_statement);
    if (m_result != nullptr)
        mysql_free_result(m_result);
}

void MySQLStatement::dateTimeToMySQLDate(MYSQL_TIME& mysqlDate, DateTime timestamp, VariantType timeType)
{
    short year, month, day, wday, yday, hour, minute, second, msecond;
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
    DatabaseStatement<MySQLConnection,MYSQL_STMT>::enumerateParams(queryParams);
    auto paramCount = (uint32_t) m_enumeratedParams.size();
    m_paramBuffers.resize(paramCount);
    m_paramLengths.resize(paramCount);
    MYSQL_BIND* paramBuffers = &m_paramBuffers[0];
    if (paramCount != 0) {
        memset(paramBuffers, 0, sizeof(MYSQL_BIND) * paramCount);
        for (unsigned paramIndex = 0; paramIndex < paramCount; paramIndex++)
            m_paramBuffers[paramIndex].length = &m_paramLengths[paramIndex];
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
        return MYSQL_TYPE_VARCHAR;

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

    auto paramCount = (unsigned) m_enumeratedParams.size();
    for (unsigned paramIndex = 0; paramIndex < paramCount; paramIndex++) {
        QueryParameter*     param = m_enumeratedParams[paramIndex];
        bool                setNull = param->isNull();
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
            m_paramLengths[paramIndex] = 0;
            bind.buffer = (void*) &param->getInt64();
            break;

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            m_paramLengths[paramIndex] = param->dataSize();
            break;

        case VAR_DATE:
        case VAR_DATE_TIME:
            m_paramLengths[paramIndex] = sizeof(MYSQL_TIME);
            bind.buffer = (void*) param->conversionBuffer();
            if (param->isNull())
                m_paramLengths[paramIndex] = 0;
            else
                dateTimeToMySQLDate(*(MYSQL_TIME*)bind.buffer, param->getDateTime(), param->dataType());
            break;

        case VAR_INT64:
            m_paramLengths[paramIndex] = 0;
            bind.buffer = (void*) &param->getInt64();
            break;

        default:
            throw DatabaseException("Unsupported parameter type(" + to_string(param->dataType()) + ") for parameter '" + param->name() + "'");
        }
        if (setNull)
            bind.is_null = &nullValue;
        else
            bind.is_null = nullptr;
        bind.error = nullptr;
    }
        /// Bind the buffers
    if (mysql_stmt_bind_param(m_statement, &m_paramBuffers[0]) != 0)
        throwMySQLError;
}

void MySQLStatement::MySQLStatement::prepare(const string& sql)
{
    if (mysql_stmt_prepare(m_statement, sql.c_str(), sql.length()) != 0)
        throwMySQLError;
}

void MySQLStatement::execute(bool)
{
    m_state.eof = false;
    if (m_result != nullptr) {
        mysql_free_result(m_result);
        m_result = nullptr;
    }
    if (m_statement != nullptr) {
        if (mysql_stmt_execute(m_statement) != 0)
            throwMySQLError;
        m_state.columnCount = mysql_stmt_field_count(m_statement);
        if (m_state.columnCount != 0)
            m_result = mysql_stmt_result_metadata(m_statement);
    } else {
        MYSQL* conn = m_connection->m_connection;
        if (mysql_query(conn, m_sql.c_str()) != 0) {
            string error = mysql_error(conn);
            throw DatabaseException(error);
        }
        m_state.columnCount = mysql_field_count(conn);
        if (m_state.columnCount != 0)
            m_result = mysql_store_result(conn);
    }
}

void MySQLStatement::bindResult(FieldList& fields)
{
    fields.clear();
    if (m_result == nullptr)
        return;

    char columnName[256];
    for (unsigned columnIndex = 0; columnIndex < m_state.columnCount; columnIndex++) {
        MYSQL_FIELD *fieldMetadata = mysql_fetch_field(m_result);
        if (fieldMetadata == nullptr)
            throwMySQLError;
        strncpy(columnName, fieldMetadata->name, sizeof(columnName));
        columnName[sizeof(columnName)-1] = 0;
        if (columnName[0] == 0)
            snprintf(columnName, sizeof(columnName), "column_%02u", columnIndex + 1);
        VariantType fieldType = mySQLTypeToVariantType(fieldMetadata->type);
        auto fieldLength = (unsigned) fieldMetadata->length;
        if (fieldLength > FETCH_BUFFER)
            fieldLength = FETCH_BUFFER;
        fields.push_back(new CMySQLStatementField(columnName, (int) columnIndex, fieldMetadata->type, fieldType, (int) fieldLength));
    }

    if (m_statement != nullptr) {
        // Bind initialized fields to MySQL bind buffers
        m_fieldBuffers.resize(m_state.columnCount);
        for (unsigned columnIndex = 0; columnIndex < m_state.columnCount; columnIndex++) {
            auto        field = (CMySQLStatementField*) &fields[columnIndex];
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
        if (mysql_stmt_bind_result(m_statement, &m_fieldBuffers[0]) != 0)
            throwMySQLError;
    }
}

void MySQLStatement::readResultRow(FieldList& fields)
{
    if (m_statement != nullptr)
        readPreparedResultRow(fields);
    else
        readUnpreparedResultRow(fields);
}

void MySQLStatement::readUnpreparedResultRow(FieldList& fields)
{
    uint32_t        fieldCount = fields.size();
    unsigned long*  lengths = mysql_fetch_lengths(m_result);
    for (uint32_t fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {

        auto field = (CMySQLStatementField*) &fields[fieldIndex];

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
            field->setDate(DateTime(data));
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
            field->setString(data, dataLength);
            break;
        case VAR_BUFFER:
            field->setBuffer(data, dataLength);
            break;

        case VAR_INT64:
            field->setInt64(string2int64(data));
            break;

        default:
            throwDatabaseException("Unsupported Variant type: " + int2string(fieldType));
        }
    }
}

void MySQLStatement::readPreparedResultRow(FieldList& fields)
{
    uint32_t    fieldCount = fields.size();
    bool        fieldSizeChanged = false;
    for (uint32_t fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {
        auto        field = (CMySQLStatementField*) &fields[fieldIndex];
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
            {
                MYSQL_TIME& mysqlTime = *(MYSQL_TIME*) bind.buffer;
                if (mysqlTime.day == 0 && mysqlTime.month == 0) {
                    // Date returned as 0000-00-00
                    field->setNull(fieldType);
                } else {
                    DateTime dt(short(mysqlTime.year), short(mysqlTime.month), short(mysqlTime.day),
                                short(mysqlTime.hour), short(mysqlTime.minute), short(mysqlTime.second));
                    if (fieldType == VAR_DATE)
                        field->setDate(dt);
                    else
                        field->setDateTime(dt);
                    field->setDataSize(sizeof(int64_t));
                }
            }
            break;

        case VAR_FLOAT:
            if (bind.buffer_type == MYSQL_TYPE_NEWDECIMAL) {
                double value = string2double((char*)bind.buffer);
                field->setFloat(value);
            } else {
                if (dataLength == sizeof(float)) {
                    float value = *(float*) bind.buffer;
                    field->setFloat(value);
                }
                field->setDataSize(dataLength);
            }
            break;

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
            if (dataLength == 0) {
                // Empty string
                auto data = (char*) field->getBuffer();
                *data = 0;
                field->setDataSize(0);
            } else {
                if (bind.buffer_length < dataLength) {
                    /// Fetch truncated, enlarge buffer and fetch remaining part
                    auto remainingBytes = uint32_t(dataLength - bind.buffer_length);
                    auto offset = (uint32_t) bind.buffer_length;
                    field->checkSize(dataLength+1);
                    bind.buffer = (char*) field->getBuffer() + offset;
                    bind.buffer_length = remainingBytes;
                    if (mysql_stmt_fetch_column(m_statement, &bind, fieldIndex, offset) != 0)
                        throwMySQLError;
                    bind.buffer_length = field->bufferSize();
                    bind.buffer = (void*) field->getBuffer();
                    fieldSizeChanged = true;
                }
                ((char *)bind.buffer)[dataLength] = 0;
                field->setDataSize(dataLength);
            }
            break;

        case VAR_INT64:
            field->setDataSize(dataLength);
            break;

        default:
            throwDatabaseException("Unsupported Variant type: " + int2string(fieldType));
        }
    }
    if (fieldSizeChanged && mysql_stmt_bind_result(m_statement, &m_fieldBuffers[0]) != 0)
        throwMySQLError;
}

void MySQLStatement::close()
{
    if (m_result != nullptr) {
        // Read all the results until EOF
        while (!m_state.eof)
            fetch();
        mysql_free_result(m_result);
        m_result = nullptr;
    }
}

void MySQLStatement::fetch()
{
    if (m_statement != nullptr) {
        int rc = mysql_stmt_fetch(m_statement);
        switch (rc) {
        case 0: // Successful, the data has been fetched to application data buffers
        case MYSQL_DATA_TRUNCATED: // Successful, but one or mode fields were truncated
            m_state.eof = false;
            break;

        case MYSQL_NO_DATA: // All data fetched
            m_state.eof = true;
            break;

        default: // Error during fetch, retrieving error
            throwMySQLError;
        }
    } else {
        m_row = mysql_fetch_row(m_result);
        if (m_row == nullptr) {
            int err = mysql_errno(m_connection->m_connection);
            if (err != 0)
                throwMySQLError;
            m_state.eof = true;
        }
    }
}
