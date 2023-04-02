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

#include <array>
#include <iomanip>
#include <sptk5/Printer.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/ODBCConnection.h>
#include <sptk5/db/Query.h>

constexpr size_t MAX_BUF = 1024;
constexpr size_t MAX_ERROR_LEN = 1024;

using namespace std;
using namespace sptk;

namespace sptk {

class ODBCField
    : public DatabaseField
{
    friend class ODBCConnection;

public:
    ODBCField(const string& fieldName, int fieldColumn, int fieldType, VariantDataType dataType, int fieldLength,
              int fieldScale)
        : DatabaseField(fieldName, fieldColumn, fieldType, dataType, fieldLength, fieldScale)
    {
    }
};

} // namespace sptk

namespace {
SQLRETURN ODBC_readStringOrBlobField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column, int16_t fieldType, SQLLEN& dataLength);
SQLRETURN ODBC_readTimestampField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column, int16_t fieldType, SQLLEN& dataLength);
void ODBC_queryBindParameter(const Query* query, QueryParameter* parameter);
} // namespace

ODBCConnection::ODBCConnection(const String& connectionString, std::chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::GENERIC_ODBC, connectTimeout)
{
}

String ODBCConnection::nativeConnectionString() const
{
    const DatabaseConnectionString& connString = connectionString();
    stringstream connectionString;
    connectionString << "DSN=" << connString.hostName();
    if (!connString.userName().empty())
    {
        connectionString << ";UID=" << connString.userName();
    }
    if (!connString.password().empty())
    {
        connectionString << ";PWD=" << connString.password();
    }
    if (!connString.databaseName().empty())
    {
        connectionString << ";DATABASE=" << connString.databaseName();
    }
    return connectionString.str();
}

void ODBCConnection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
        setInTransaction(false);
        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        String finalConnectionString;
        m_connect->connect(nativeConnectionString(), finalConnectionString, false);
        if (m_connect->driverDescription().find("Microsoft SQL Server") != string::npos)
        {
            connectionType(DatabaseConnectionType::MSSQL_ODBC);
        }
    }
}

void ODBCConnection::closeDatabase()
{
    disconnectAllQueries();
    m_connect->freeConnect();
}

DBHandle ODBCConnection::handle() const
{
    return (DBHandle) m_connect->handle();
}

bool ODBCConnection::active() const
{
    return m_connect->isConnected();
}

void ODBCConnection::driverBeginTransaction()
{
    if (!m_connect->isConnected())
    {
        open();
    }

    if (getInTransaction())
    {
        logAndThrow("ODBCConnection::driverBeginTransaction", "Transaction already started.");
    }

    m_connect->beginTransaction();
    setInTransaction(true);
}

void ODBCConnection::driverEndTransaction(bool commit)
{
    if (!getInTransaction())
    {
        if (commit)
        {
            logAndThrow("ODBCConnection::driverEndTransaction", "Can't commit - transaction isn't started.");
        }
        else
        {
            logAndThrow("ODBCConnection::driverEndTransaction", "Can't rollback - transaction isn't started.");
        }
    }

    if (commit)
    {
        m_connect->commit();
    }
    else
    {
        m_connect->rollback();
    }

    m_connect->setConnectOption(SQL_ATTR_AUTOCOMMIT, (UDWORD) true);
    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------
namespace {
inline bool successful(int ret)
{
    return ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO;
}
} // namespace

String ODBCConnection::queryError(SQLHSTMT stmt) const
{
    array<SQLCHAR, SQL_MAX_MESSAGE_LENGTH> errorDescription = {};
    array<SQLCHAR, SQL_MAX_MESSAGE_LENGTH> errorState = {};

    SWORD pcnmsg = 0;
    SQLINTEGER nativeError = 0;

    String error;
    int resultCode = SQLError(SQL_NULL_HENV, handle(), stmt, errorState.data(), &nativeError, errorDescription.data(),
                              (SQLSMALLINT) errorDescription.size(), &pcnmsg);

    if (resultCode == SQL_SUCCESS)
    {
        error = (const char*) errorDescription.data();
    }
    else
    {
        resultCode = SQLError(SQL_NULL_HENV, handle(), nullptr, errorState.data(), &nativeError, errorDescription.data(),
                              (SQLSMALLINT) errorDescription.size(), &pcnmsg);
        if (resultCode == SQL_SUCCESS)
        {
            error = (const char*) errorDescription.data();
        }
    }

    if (resultCode != SQL_SUCCESS)
    {
        error = "Unknown error";
    }

    return removeDriverIdentification(error.c_str());
}

String ODBCConnection::queryError(const Query* query) const
{
    return queryError(query->statement());
}

void ODBCConnection::queryAllocStmt(Query* query)
{
    const scoped_lock lock(*m_connect);

    auto* hStmt = (SQLHSTMT) query->statement();
    if (hStmt != SQL_NULL_HSTMT)
    {
        SQLFreeStmt(hStmt, SQL_DROP);
    }

    auto* hdb = handle();

    if (const int result = SQLAllocStmt(hdb, &hStmt);
        result != SQL_SUCCESS)
    {
        const String error = queryError(query);
        querySetStmt(query, SQL_NULL_HSTMT);
        logAndThrow("ODBCConnection::queryAllocStmt", error);
    }

    auto statement = shared_ptr<uint8_t>((StmtHandle) hStmt,
                                         [](StmtHandle ptr) {
                                             SQLFreeStmt((SQLHSTMT) ptr, SQL_DROP);
                                         });
    querySetStmt(query, statement);
}

void ODBCConnection::queryFreeStmt(Query* query)
{
    const scoped_lock lock(*m_connect);

    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void ODBCConnection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(*m_connect);

    SQLFreeStmt(query->statement(), SQL_CLOSE);
}

void ODBCConnection::queryPrepare(Query* query)
{
    const scoped_lock lock(*m_connect);

    query->fields().clear();

    char* sql = query->sql().empty() ? nullptr : query->sql().data();
    if (!successful(SQLPrepare(query->statement(), (SQLCHAR*) sql, SQL_NTS)))
    {
        THROW_QUERY_ERROR(query, queryError(query))
    }
}

void ODBCConnection::queryExecute(Query* query)
{
    const scoped_lock lock(*m_connect);

    int result = 0;
    if (query->prepared())
    {
        result = SQLExecute(query->statement());
    }
    else
    {
        if (query->sql().empty())
        {
            return;
        }
        result = SQLExecDirect(query->statement(), (SQLCHAR*) query->sql().data(), SQL_NTS);
    }

    if (successful(result))
    {
        return;
    }

    if (result == SQL_NEED_DATA)
    {
        THROW_QUERY_ERROR(query, "Invalid data size")
    }

    constexpr int diagRecordSize = 16;
    array<SQLCHAR, diagRecordSize> state = {};
    array<SQLCHAR, MAX_ERROR_LEN> text = {};
    SQLINTEGER nativeError = 0;
    SQLSMALLINT recordCount = 0;
    SQLSMALLINT textLength = 0;

    result = SQLGetDiagField(SQL_HANDLE_STMT, query->statement(), 1, SQL_DIAG_NUMBER, &recordCount, sizeof(recordCount),
                             &textLength);

    Strings errors;
    const SQLSMALLINT recordNumber = 1;
    while (successful(result))
    {
        result = SQLGetDiagRec(SQL_HANDLE_STMT, query->statement(), recordNumber, state.data(), &nativeError,
                               text.data(), (SQLSMALLINT) text.size(), &textLength);
        if (successful(result))
        {
            errors.push_back(removeDriverIdentification((const char*) text.data()));
        }
    }

    if (!errors.empty())
        THROW_QUERY_ERROR(query, errors.join("; "))

    if (!successful(result))
        THROW_QUERY_ERROR(query, queryError(query))
}

int ODBCConnection::queryColCount(Query* query)
{
    const scoped_lock lock(*m_connect);

    int16_t count = 0;
    if (!successful(SQLNumResultCols(query->statement(), &count)))
        THROW_QUERY_ERROR(query, queryError(query))

    return count;
}

void ODBCConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
    const scoped_lock lock(*m_connect);
    SQLLEN result = 0;

    if (!successful(
            SQLColAttributes(query->statement(), (SQLUSMALLINT) column, (SQLUSMALLINT) descType, nullptr, 0, nullptr,
                             &result)))
    {
        THROW_QUERY_ERROR(query, queryError(query))
    }

    value = (int32_t) result;
}

void ODBCConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, LPSTR buff, int len)
{
    int16_t available = 0;
    if (buff == nullptr || len <= 0)
        THROW_QUERY_ERROR(query, "Invalid buffer or buffer len")

    const scoped_lock lock(*m_connect);

    if (!successful(
            SQLColAttributes(query->statement(), (SQLUSMALLINT) column, (SQLUSMALLINT) descType, buff, (int16_t) len,
                             &available, nullptr)))
        THROW_QUERY_ERROR(query, queryError(query))
}

namespace {
bool dateTimeToTimestamp(TIMESTAMP_STRUCT* timestampStruct, const DateTime& dateTime, bool dateOnly)
{
    if (!dateTime.zero())
    {
        short weekDay = 0;
        short yearDay = 0;
        short milliseconds = 0;
        dateTime.decodeDate(&timestampStruct->year, (int16_t*) &timestampStruct->month, (int16_t*) &timestampStruct->day,
                            &weekDay, &yearDay);
        if (dateOnly)
        {
            timestampStruct->hour = timestampStruct->minute = timestampStruct->second = 0;
        }
        else
        {
            dateTime.decodeTime((int16_t*) &timestampStruct->hour, (int16_t*) &timestampStruct->minute, (int16_t*) &timestampStruct->second, &milliseconds);
        }
        const int msInSecond = 1000;
        timestampStruct->fraction = milliseconds * msInSecond;
        return true;
    }
    return false;
}

void ODBC_queryBindParameter(const Query* query, QueryParameter* param)
{
    static const int dateAccuracy = 19;
    static SQLLEN cbNullValue = SQL_NULL_DATA;

    const VariantDataType ptype = param->dataType();
    for (unsigned j = 0; j < param->bindCount(); ++j)
    {
        int16_t paramType = 0;
        int16_t valueType = 0;
        const int16_t scale = 0;
        void* buff = nullptr;
        SQLLEN len = 0;
        auto paramNumber = int16_t(param->bindIndex(j) + 1);

        SQLLEN* cbValue = nullptr;
        if (param->isNull())
        {
            cbValue = &cbNullValue;
            len = 0;
        }

        const int16_t inputOutputMode = SQL_PARAM_INPUT;
        switch (ptype)
        {
            case VariantDataType::VAR_BOOL:
                paramType = SQL_C_BIT;
                valueType = SQL_BIT;
                buff = (void*) &param->get<bool>();
                break;

            case VariantDataType::VAR_INT:
                paramType = SQL_C_SLONG;
                valueType = SQL_INTEGER;
                buff = (void*) &param->get<int>();
                break;

            case VariantDataType::VAR_INT64:
                paramType = SQL_C_SBIGINT;
                valueType = SQL_BIGINT;
                buff = (void*) &param->get<int64_t>();
                break;

            case VariantDataType::VAR_FLOAT:
                paramType = SQL_C_DOUBLE;
                valueType = SQL_DOUBLE;
                buff = (void*) &param->get<double>();
                break;

            case VariantDataType::VAR_STRING:
                len = (long) param->dataSize();
                paramType = SQL_C_CHAR;
                valueType = SQL_WVARCHAR;
                buff = (void*) param->getText();
                break;

            case VariantDataType::VAR_TEXT:
                len = (long) param->dataSize();
                paramType = SQL_C_CHAR;
                valueType = SQL_WLONGVARCHAR;
                buff = (void*) param->getText();
                break;

            case VariantDataType::VAR_DATE:
                paramType = SQL_C_TIMESTAMP;
                valueType = SQL_TIMESTAMP;
                len = sizeof(TIMESTAMP_STRUCT);
                buff = param->conversionBuffer();
                if (!dateTimeToTimestamp((TIMESTAMP_STRUCT*) param->conversionBuffer(),
                                         param->get<DateTime>(), true))
                {
                    paramType = SQL_C_CHAR;
                    valueType = SQL_CHAR;
                    *(char*) buff = 0;
                }
                break;

            case VariantDataType::VAR_DATE_TIME:
                paramType = SQL_C_TIMESTAMP;
                valueType = SQL_TIMESTAMP;
                len = dateAccuracy;
                buff = param->conversionBuffer();
                if (!dateTimeToTimestamp((TIMESTAMP_STRUCT*) param->conversionBuffer(),
                                         param->get<DateTime>(), false))
                {
                    paramType = SQL_C_CHAR;
                    valueType = SQL_CHAR;
                    *static_cast<char*>(buff) = 0;
                }
                break;

            case VariantDataType::VAR_BUFFER:
                paramType = SQL_C_BINARY;
                valueType = SQL_LONGVARBINARY;
                len = static_cast<SQLLEN>(param->dataSize());
                buff = (void*) param->getText();
#ifdef _WIN32
                param->callbackLength() = (long) len;
                cbValue = (SQLLEN*) &param->callbackLength();
#else
                param->callbackLength() = len;
                cbValue = &param->callbackLength();
#endif
                break;

            default:
                throw DatabaseException(
                    "Unsupported parameter type(" + to_string((int) param->dataType()) + ") for parameter '" +
                    param->name() + "'");
        }

        const auto resultCode = SQLBindParameter(query->statement(), (SQLUSMALLINT) paramNumber, inputOutputMode, paramType, valueType,
                                                 len, scale, buff, len, cbValue);
        if (resultCode != SQL_SUCCESS)
        {
            param->binding().reset(false);
            THROW_QUERY_ERROR(query, "Can't bind parameter " << paramNumber)
        }
    }
}
} // namespace

void ODBCConnection::queryBindParameters(Query* query)
{
    const scoped_lock lock(*m_connect);

    for (uint32_t i = 0; i < query->paramCount(); ++i)
    {
        QueryParameter* param = &query->param(i);
        ODBC_queryBindParameter(query, param);
    }
}

void ODBCConnection::ODBCtypeToCType(int32_t odbcType, int32_t& cType, VariantDataType& dataType)
{
    switch (odbcType)
    {
        case SQL_TINYINT:
        case SQL_SMALLINT:
            cType = SQL_C_SSHORT;
            dataType = VariantDataType::VAR_INT;
            break;

        case SQL_INTEGER:
        case SQL_BIGINT:
            cType = SQL_C_SLONG;
            dataType = VariantDataType::VAR_INT64;
            break;

        case SQL_NUMERIC:
        case SQL_REAL:
        case SQL_DECIMAL:
        case SQL_DOUBLE:
        case SQL_FLOAT:
            cType = SQL_C_DOUBLE;
            dataType = VariantDataType::VAR_FLOAT;
            break;

        case SQL_DATE:      // ODBC 2.0 only
        case SQL_TYPE_DATE: // ODBC 3.0 only
            cType = SQL_C_TIMESTAMP;
            dataType = VariantDataType::VAR_DATE;
            break;

        case SQL_TIME:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIME:
        case SQL_TYPE_TIMESTAMP:
            cType = SQL_C_TIMESTAMP;
            dataType = VariantDataType::VAR_DATE_TIME;
            break;

        case SQL_BINARY:
        case SQL_LONGVARBINARY:
        case SQL_VARBINARY:
            cType = SQL_C_BINARY;
            dataType = VariantDataType::VAR_BUFFER;
            break;

        case SQL_BIT:
            cType = SQL_C_BIT;
            dataType = VariantDataType::VAR_BOOL;
            break;

        default:
            cType = SQL_C_CHAR;
            dataType = VariantDataType::VAR_STRING;
            break;
    }
}

void ODBCConnection::parseColumns(Query* query, int count)
{
    // Reading the column attributes
    array<char, MAX_NAME_LEN> columnName = {};
    int32_t columnType = 0;
    int32_t columnLength = 0;
    int32_t columnScale = 0;
    int32_t cType = 0;
    VariantDataType dataType = VariantDataType::VAR_NONE;

    stringstream columnNameStr;
    columnNameStr.fill('0');

    constexpr int largeTextSize = 65536;
    constexpr int maxColumnScale = 20;

    for (int16_t column = 1; column <= int16_t(count); ++column)
    {
        queryColAttributes(query, column, SQL_COLUMN_NAME, columnName.data(), MAX_NAME_LEN - 1);
        queryColAttributes(query, column, SQL_COLUMN_TYPE, columnType);
        queryColAttributes(query, column, SQL_COLUMN_LENGTH, columnLength);
        queryColAttributes(query, column, SQL_COLUMN_SCALE, columnScale);
        ODBCtypeToCType(columnType, cType, dataType);

        if (dataType == VariantDataType::VAR_STRING && columnLength >= largeTextSize)
        {
            dataType = VariantDataType::VAR_TEXT;
        }

        if (columnName[0] != 0)
        {
            columnNameStr.str(columnName.data());
        }
        else
        {
            columnNameStr.str("column");
            columnNameStr << setw(2) << column;
        }

        if (columnLength > FETCH_BUFFER_SIZE)
        {
            columnLength = FETCH_BUFFER_SIZE;
        }

        if (dataType == VariantDataType::VAR_FLOAT && (columnScale < 0 || columnScale > maxColumnScale))
        {
            columnScale = 0;
        }

        auto field = make_shared<ODBCField>(columnNameStr.str(), column, cType, dataType, columnLength, columnScale);
        query->fields().push_back(field);
    }
}

void ODBCConnection::queryOpen(Query* query)
{
    if (!active())
    {
        open();
    }

    if (query->active())
    {
        return;
    }

    if (query->statement() == nullptr)
    {
        queryAllocStmt(query);
    }

    try
    {
        queryBindParameters(query);
    }
    catch (const DatabaseException& e)
    {
        throw DatabaseException(e.what());
    }

    if (query->autoPrepare() && !query->prepared())
    {
        queryPrepare(query);
        querySetPrepared(query, true);
    }

    queryExecute(query);

    const int count = queryColCount(query);
    if (count < 1)
    {
        queryCloseStmt(query);
        return;
    }

    querySetActive(query, true);

    if (query->fieldCount() == 0)
    {
        parseColumns(query, count);
    }

    querySetEof(query, false);
    queryFetch(query);
}

namespace {
uint32_t trimField(char* str, uint32_t size)
{
    char* ptr = str + size - 1;
    const char chr = str[0];
    str[0] = '!';

    while (*ptr == ' ')
    {
        --ptr;
    }
    *(++ptr) = 0;

    if (chr == ' ' && str[1] == 0)
    {
        str[0] = 0;
        return 0;
    }

    str[0] = chr;
    return uint32_t(ptr - str);
}

SQLRETURN ODBC_readStringOrBlobField(SQLHSTMT statement, DatabaseField* dbField, SQLUSMALLINT column,
                                     int16_t fieldType, SQLLEN& dataLength)
{
    auto* field = dynamic_cast<ODBCField*>(dbField);

    auto& buffer = field->get<Buffer>();

    // Get data length
    auto resultCode = SQLGetData(statement, column, fieldType, buffer.data(), SQLINTEGER(0), &dataLength);
    if (!successful(resultCode))
    {
        return resultCode;
    }

    if (dataLength == 0 || (dataLength < 0 && dataLength != SQL_NO_TOTAL))
    {
        return SQL_SUCCESS;
    }

    if (dataLength != SQL_NO_TOTAL)
    {
        field->checkSize(uint32_t(dataLength));

        // Read data
        resultCode = SQLGetData(statement, column, fieldType, buffer.data(), SQLINTEGER(dataLength + 1), &dataLength);
        field->setDataSize(dataLength);
        return resultCode;
    }

    // Fetch BLOB data until there is no more data
    constexpr size_t initialReadSize = 16384;
    field->checkSize(initialReadSize);

    size_t bufferSize = field->bufferSize();
    SQLLEN remainingSize = 0;

    SQLLEN offset = 0;
    dataLength = 0;
    const SQLLEN readSize = initialReadSize;
    resultCode = SQL_SUCCESS;
    while (resultCode != SQL_NO_DATA)
    {
        bufferSize += readSize;
        field->checkSize(bufferSize);
        resultCode = SQLGetData(statement, column, fieldType,
                                buffer.data() + offset, SQLINTEGER(readSize), &remainingSize);

        if (resultCode == SQL_NO_DATA)
        {
            return SQL_SUCCESS;
        }

        if (remainingSize >= 0 && remainingSize < readSize)
        {
            // Last chunk received
            dataLength += remainingSize;
            break;
        }

        offset += readSize - 1;
        dataLength += readSize - 1;
    }

    return resultCode;
}

SQLRETURN ODBC_readTimestampField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column,
                                  int16_t fieldType, SQLLEN& dataLength)
{
    TIMESTAMP_STRUCT timestampStruct = {};
    const SQLRETURN resultCode = SQLGetData(statement, column, fieldType, (SQLPOINTER) &timestampStruct, 0, &dataLength);
    if (dataLength > 0)
    {
        const DateTime dateTime(timestampStruct.year, (short) timestampStruct.month, (short) timestampStruct.day, (short) timestampStruct.hour, (short) timestampStruct.minute, (short) timestampStruct.second);
        field->setDateTime(dateTime, field->dataType() == VariantDataType::VAR_DATE);
    }
    return resultCode;
}
} // namespace

void ODBCConnection::queryFetch(Query* query)
{
    if (!query->active())
        THROW_QUERY_ERROR(query, "Dataset isn't open")

    auto* statement = query->statement();

    const scoped_lock lock(*m_connect);

    int resultCode = SQLFetch(statement);

    if (!successful(resultCode))
    {
        if (resultCode < 0)
        {
            THROW_QUERY_ERROR(query, queryError(query))
        }
        else
        {
            querySetEof(query, resultCode == SQL_NO_DATA);
            return;
        }
    }

    auto fieldCount = query->fieldCount();
    SQLLEN dataLength = 0;

    if (fieldCount == 0)
    {
        return;
    }

    ODBCField* field = nullptr;
    for (SQLUSMALLINT column = 0; column < (SQLUSMALLINT) fieldCount;)
    {
        try
        {
            field = (ODBCField*) &(*query)[column];
            auto fieldType = (int16_t) field->fieldType();

            ++column;

            resultCode = SQL_SUCCESS;
            switch (fieldType)
            {
                case SQL_C_SLONG:
                    resultCode = SQLGetData(statement, column, fieldType, &field->get<int64_t>(), 0, &dataLength);
                    break;

                case SQL_C_DOUBLE:
                    resultCode = SQLGetData(statement, column, fieldType, &field->get<double>(), 0, &dataLength);
                    break;

                case SQL_C_TIMESTAMP:
                    resultCode = ODBC_readTimestampField(statement, field, column, fieldType, dataLength);
                    break;

                case SQL_C_BINARY:
                case SQL_C_CHAR:
                    resultCode = ODBC_readStringOrBlobField(statement, field, column, fieldType, dataLength);
                    break;

                case SQL_BIT:
                    resultCode = SQLGetData(statement, column, fieldType, &field->get<bool>(), 1, &dataLength);
                    break;

                default:
                    dataLength = 0;
                    break;
            }

            if (resultCode != SQL_SUCCESS && resultCode != SQL_SUCCESS_WITH_INFO)
            {
                throw Exception(queryError(query));
            }

            if (fieldType == SQL_C_CHAR && dataLength > 0)
            {
                dataLength = (SQLINTEGER) trimField((char*) field->get<Buffer>().data(), (uint32_t) dataLength);
            }

            if (dataLength <= 0)
            {
                field->setNull(field->dataType());
            }
            else
            {
                field->dataSize((size_t) dataLength);
            }
        }
        catch (const Exception& e)
        {
            Query::throwError("ODBCConnection::queryFetch",
                              "Can't read field " + field->fieldName() + "\n" + string(e.what()));
        }
    }
}

String ODBCConnection::driverDescription() const
{
    if (m_connect != nullptr)
    {
        return m_connect->driverDescription();
    }
    return "";
}

void ODBCConnection::listDataSources(Strings& dsns)
{
    dsns.clear();
    array<SQLCHAR, MAX_BUF> dataSource = {0};
    array<SQLCHAR, MAX_BUF> description = {0};
    SQLSMALLINT rdsrc = 0;
    SQLSMALLINT rdesc = 0;

    SQLHENV hEnv = ODBCConnectionBase::getEnvironment().handle();
    const bool offline = hEnv == nullptr;
    if (offline)
    {
        if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HENV, &hEnv) != SQL_SUCCESS)
        {
            throw DatabaseException("ODBCConnection::SQLAllocHandle");
        }
        if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER))
        {
            throw DatabaseException("ODBCConnection::SQLSetEnvAttr");
        }
    }

    SQLUSMALLINT direction = SQL_FETCH_FIRST;
    while (true)
    {
        if (const SQLRETURN ret = SQLDataSources(hEnv, direction,
                                                 dataSource.data(), (SQLSMALLINT) dataSource.size(), &rdsrc,
                                                 description.data(), (SQLSMALLINT) description.size(), &rdesc);
            ret == SQL_NO_DATA)
        {
            break;
        }
        direction = SQL_FETCH_NEXT;
        dsns.push_back(String((char*) dataSource.data()) + " (" + String((char*) description.data()) + ")");
    }

    if (offline)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }
}

void ODBCConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    const scoped_lock lock(*m_connect);

    if (objectType == DatabaseObjectType::DATABASES)
    {
        listDataSources(objects);
        return;
    }

    SQLHSTMT stmt = nullptr;
    try
    {
        vector<SQLCHAR> objectSchema(MAX_NAME_LEN * 2);
        vector<SQLCHAR> objectName(MAX_NAME_LEN * 2);
        short procedureType = 0;
        SQLLEN objectSchemaLength = 0;
        SQLLEN objectNameLength = 0;

        stmt = makeObjectListStatement(objectType, objectSchema, objectName, procedureType, objectSchemaLength, objectNameLength);

        while (true)
        {
            objectSchema[0] = 0;
            objectName[0] = 0;
            const SQLRETURN resultCode = SQLFetch(stmt);

            if (resultCode == SQL_NO_DATA_FOUND)
            {
                break;
            }

            if (!successful(resultCode))
            {
                throw DatabaseException("SQLFetch");
            }

            objectSchema[objectSchemaLength] = 0;
            objectName[objectNameLength] = 0;

            if (objectType == DatabaseObjectType::FUNCTIONS && procedureType != SQL_PT_FUNCTION)
            {
                continue;
            }

            if (objectType == DatabaseObjectType::PROCEDURES && procedureType != SQL_PT_PROCEDURE)
            {
                continue;
            }

            objects.push_back(String((char*) objectSchema.data()) + "." + String((char*) objectName.data()));
        }

        SQLFreeStmt(stmt, SQL_DROP);
    }
    catch (const Exception& e)
    {
        String error;
        if (stmt != nullptr)
        {
            error = queryError(stmt);
            SQLFreeStmt(stmt, SQL_DROP);
        }
        logAndThrow(e.what(), error);
    }
}

SQLHSTMT ODBCConnection::makeObjectListStatement(const DatabaseObjectType& objectType, vector<SQLCHAR>& objectSchema, vector<SQLCHAR>& objectName, short& procedureType, SQLLEN& objectSchemaLength, SQLLEN& objectNameLength) const
{
    procedureType = 0;

    SQLHSTMT stmt = nullptr;

    if (SQLAllocStmt(this->handle(), &stmt) != SQL_SUCCESS)
    {
        throw DatabaseException("ODBCConnection::SQLAllocStmt");
    }

    switch (objectType)
    {
        case DatabaseObjectType::TABLES:
            if (SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, Buffer("TABLE").data(), SQL_NTS) !=
                SQL_SUCCESS)
            {
                throw DatabaseException("SQLTables");
            }
            break;

        case DatabaseObjectType::VIEWS:
            if (SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, Buffer("VIEW").data(), SQL_NTS) !=
                SQL_SUCCESS)
            {
                throw DatabaseException("SQLTables");
            }
            break;

        case DatabaseObjectType::PROCEDURES:
        case DatabaseObjectType::FUNCTIONS:
            if (SQLProcedures(stmt, nullptr, 0, Buffer("").data(), SQL_NTS,
                              Buffer("%").data(), SQL_NTS) != SQL_SUCCESS)
            {
                throw DatabaseException("SQLProcedures");
            }
            break;

        default:
            break;
    }

    if (SQLBindCol(stmt, 2, SQL_C_CHAR, objectSchema.data(), SQLLEN(objectSchema.size() - 1), &objectSchemaLength) != SQL_SUCCESS)
    {
        throw DatabaseException("SQLBindCol");
    }

    if (SQLBindCol(stmt, 3, SQL_C_CHAR, objectName.data(), SQLLEN(objectName.size() - 1), &objectNameLength) != SQL_SUCCESS)
    {
        throw DatabaseException("SQLBindCol");
    }

    if (objectType == DatabaseObjectType::FUNCTIONS || objectType == DatabaseObjectType::PROCEDURES)
    {
        constexpr SQLSMALLINT procedureTypeColumn = 8;
        if (SQLBindCol(stmt, procedureTypeColumn, SQL_C_SHORT, &procedureType, SQLLEN(sizeof(procedureType)), nullptr) != SQL_SUCCESS)
        {
            throw DatabaseException("SQLBindCol");
        }
    }

    return stmt;
}

void ODBCConnection::executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    static const RegularExpression matchStatementEnd("(;\\s*)$");
    static const RegularExpression matchRoutineStart("^CREATE\\s+FUNCTION", "i");
    static const RegularExpression matchGo("^\\s*GO\\s*$", "i");
    static const RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    string statement;
    bool routineStarted = false;
    for (String row: sqlBatch)
    {
        row = trim(row);
        if (row.empty() || matchCommentRow.matches(row))
        {
            continue;
        }

        if (!routineStarted)
        {
            row = trim(row);
            if (row.empty() || row.startsWith("--"))
            {
                continue;
            }
        }

        if (matchRoutineStart.matches(row))
        {
            routineStarted = true;
        }

        if (!routineStarted && matchStatementEnd.matches(row))
        {
            row = matchStatementEnd.s(row, "");
            statement += row;
            statements.push_back(trim(statement));
            statement = "";
            continue;
        }

        if (matchGo.matches(row))
        {
            routineStarted = false;
            statements.push_back(trim(statement));
            statement = "";
            continue;
        }

        statement += row + "\n";
    }

    if (!trim(statement).empty())
    {
        statements.push_back(statement);
    }

    for (const auto& stmt: statements)
    {
        try
        {
            Query query(this, stmt, false);
            query.exec();
        }
        catch (const Exception& e)
        {
            stringstream error;
            error << e.what() << ". Query: " << stmt;
            if (errors != nullptr)
            {
                errors->push_back(error.str());
            }
            else
            {
                throw DatabaseException(error.str());
            }
        }
    }
}

void ODBCConnection::bulkInsert(const String& tableName, const Strings& columnNames, const vector<VariantVector>& data)
{
    constexpr int recordsInBatch = 64;
    auto begin = data.begin();
    auto end = data.begin();
    for (; end != data.end(); ++end)
    {
        if (end - begin > recordsInBatch)
        {
            bulkInsertRecords(tableName, columnNames, begin, end);
            begin = end;
        }
    }

    if (begin != end)
    {
        bulkInsertRecords(tableName, columnNames, begin, end);
    }
}

map<ODBCConnection*, shared_ptr<ODBCConnection>> ODBCConnection::s_odbcConnections;

[[maybe_unused]] void* odbc_create_connection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    auto connection = make_shared<ODBCConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    ODBCConnection::s_odbcConnections[connection.get()] = connection;
    return connection.get();
}

[[maybe_unused]] void odbc_destroy_connection(void* connection)
{
    ODBCConnection::s_odbcConnections.erase((ODBCConnection*) connection);
}
