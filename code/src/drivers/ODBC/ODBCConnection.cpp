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

#include <array>
#include <iomanip>
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
    : public DatabaseField {
    friend class ODBCConnection;

public:
    ODBCField(const string& fieldName, int fieldColumn, int fieldType, VariantDataType dataType, int fieldLength,
              int fieldScale)
        : DatabaseField(fieldName, fieldColumn, fieldType, dataType, fieldLength, fieldScale)
    {
    }
};

static SQLRETURN ODBC_readStringOrBlobField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column, int16_t fieldType, SQLLEN& dataLength);
static SQLRETURN ODBC_readTimestampField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column, int16_t fieldType, SQLLEN& dataLength);
static void ODBC_queryBindParameter(const Query* query, QueryParameter* parameter);

} // namespace sptk

ODBCConnection::ODBCConnection(const String& connectionString)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::GENERIC_ODBC)
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
static inline bool successful(int ret)
{
    return ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO;
}

String ODBCConnection::queryError(SQLHSTMT stmt) const
{
    array<SQLCHAR, SQL_MAX_MESSAGE_LENGTH> errorDescription = {};
    array<SQLCHAR, SQL_MAX_MESSAGE_LENGTH> errorState = {};

    SWORD pcnmsg = 0;
    SQLINTEGER nativeError = 0;

    String error;
    int rc = SQLError(SQL_NULL_HENV, handle(), stmt, errorState.data(), &nativeError, errorDescription.data(),
                      (SQLSMALLINT) errorDescription.size(), &pcnmsg);

    if (rc == SQL_SUCCESS)
    {
        error = (const char*) errorDescription.data();
    }
    else
    {
        rc = SQLError(SQL_NULL_HENV, handle(), nullptr, errorState.data(), &nativeError, errorDescription.data(),
                      (SQLSMALLINT) errorDescription.size(), &pcnmsg);
        if (rc == SQL_SUCCESS)
        {
            error = (const char*) errorDescription.data();
        }
    }

    if (rc != SQL_SUCCESS)
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
    scoped_lock lock(*m_connect);

    auto* hStmt = (SQLHSTMT) query->statement();
    if (hStmt != SQL_NULL_HSTMT)
    {
        SQLFreeStmt(hStmt, SQL_DROP);
    }

    auto* hdb = handle();

    if (int rc = SQLAllocStmt(hdb, &hStmt); rc != SQL_SUCCESS)
    {
        String error = queryError(query);
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
    scoped_lock lock(*m_connect);

    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void ODBCConnection::queryCloseStmt(Query* query)
{
    scoped_lock lock(*m_connect);

    SQLFreeStmt(query->statement(), SQL_CLOSE);
}

void ODBCConnection::queryPrepare(Query* query)
{
    scoped_lock lock(*m_connect);

    query->fields().clear();

    char* sql = query->sql().empty() ? nullptr : query->sql().data();
    if (!successful(SQLPrepare(query->statement(), (SQLCHAR*) sql, SQL_NTS)))
    {
        THROW_QUERY_ERROR(query, queryError(query))
    }
}

void ODBCConnection::queryExecute(Query* query)
{
    scoped_lock lock(*m_connect);

    int rc = 0;
    if (query->prepared())
    {
        rc = SQLExecute(query->statement());
    }
    else
    {
        if (query->sql().empty())
        {
            return;
        }
        rc = SQLExecDirect(query->statement(), (SQLCHAR*) query->sql().data(), SQL_NTS);
    }

    if (successful(rc))
    {
        return;
    }

    if (rc == SQL_NEED_DATA)
    {
        THROW_QUERY_ERROR(query, "Invalid data size")
    }

    constexpr int diagRecordSize = 16;
    array<SQLCHAR, diagRecordSize> state = {};
    array<SQLCHAR, MAX_ERROR_LEN> text = {};
    SQLINTEGER nativeError = 0;
    SQLSMALLINT recordCount = 0;
    SQLSMALLINT textLength = 0;

    rc = SQLGetDiagField(SQL_HANDLE_STMT, query->statement(), 1, SQL_DIAG_NUMBER, &recordCount, sizeof(recordCount),
                         &textLength);

    Strings errors;
    SQLSMALLINT recordNumber = 1;
    while (successful(rc))
    {
        rc = SQLGetDiagRec(SQL_HANDLE_STMT, query->statement(), recordNumber, state.data(), &nativeError,
                           text.data(), (SQLSMALLINT) text.size(), &textLength);
        if (successful(rc))
        {
            errors.push_back(removeDriverIdentification((const char*) text.data()));
        }
    }

    if (!errors.empty())
        THROW_QUERY_ERROR(query, errors.join("; "))

    if (!successful(rc))
        THROW_QUERY_ERROR(query, queryError(query))
}

int ODBCConnection::queryColCount(Query* query)
{
    scoped_lock lock(*m_connect);

    int16_t count = 0;
    if (!successful(SQLNumResultCols(query->statement(), &count)))
        THROW_QUERY_ERROR(query, queryError(query))

    return count;
}

void ODBCConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
    scoped_lock lock(*m_connect);
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

    scoped_lock lock(*m_connect);

    if (!successful(
        SQLColAttributes(query->statement(), (SQLUSMALLINT) column, (SQLUSMALLINT) descType, buff, (int16_t) len,
                         &available, nullptr)))
        THROW_QUERY_ERROR(query, queryError(query))
}

static bool dateTimeToTimestamp(TIMESTAMP_STRUCT* t, DateTime dt, bool dateOnly)
{
    if (!dt.zero())
    {
        short wday = 0;
        short yday = 0;
        short ms = 0;
        dt.decodeDate(&t->year, (int16_t*) &t->month, (int16_t*) &t->day, &wday, &yday);
        if (dateOnly)
        {
            t->hour = t->minute = t->second = 0;
        }
        else
        {
            dt.decodeTime((int16_t*) &t->hour, (int16_t*) &t->minute, (int16_t*) &t->second, &ms);
        }
        t->fraction = ms * 1000;
        return true;
    }
    return false;
}

void sptk::ODBC_queryBindParameter(const Query* query, QueryParameter* param)
{
    static SQLLEN cbNullValue = SQL_NULL_DATA;

    VariantDataType ptype = param->dataType();
    for (unsigned j = 0; j < param->bindCount(); ++j)
    {
        int16_t paramType = 0;
        int16_t sqlType = 0;
        int16_t scale = 0;
        void* buff;
        SQLLEN len = 0;
        auto paramNumber = int16_t(param->bindIndex(j) + 1);

        SQLLEN* cbValue = nullptr;
        if (param->isNull())
        {
            cbValue = &cbNullValue;
            len = 0;
        }

        int16_t parameterMode = SQL_PARAM_INPUT;
        switch (ptype)
        {
            case VariantDataType::VAR_BOOL:
                paramType = SQL_C_BIT;
                sqlType = SQL_BIT;
                buff = (void*) &param->get<bool>();
                break;

            case VariantDataType::VAR_INT:
                paramType = SQL_C_SLONG;
                sqlType = SQL_INTEGER;
                buff = (void*) &param->get<int>();
                break;

            case VariantDataType::VAR_INT64:
                paramType = SQL_C_SBIGINT;
                sqlType = SQL_BIGINT;
                buff = (void*) &param->get<int64_t>();
                break;

            case VariantDataType::VAR_FLOAT:
                paramType = SQL_C_DOUBLE;
                sqlType = SQL_DOUBLE;
                buff = (void*) &param->get<double>();
                break;

            case VariantDataType::VAR_STRING:
                len = (long) param->dataSize();
                paramType = SQL_C_CHAR;
                sqlType = SQL_WVARCHAR;
                buff = (void*) param->getText();
                break;

            case VariantDataType::VAR_TEXT:
                len = (long) param->dataSize();
                paramType = SQL_C_CHAR;
                sqlType = SQL_WLONGVARCHAR;
                buff = (void*) param->getText();
                break;

            case VariantDataType::VAR_DATE:
                paramType = SQL_C_TIMESTAMP;
                sqlType = SQL_TIMESTAMP;
                len = sizeof(TIMESTAMP_STRUCT);
                buff = param->conversionBuffer();
                if (!dateTimeToTimestamp(reinterpret_cast<TIMESTAMP_STRUCT*>(param->conversionBuffer()), 
                    param->get<DateTime>(), true))
                {
                    paramType = SQL_C_CHAR;
                    sqlType = SQL_CHAR;
                    *(char*) buff = 0;
                }
                break;

            case VariantDataType::VAR_DATE_TIME:
                paramType = SQL_C_TIMESTAMP;
                sqlType = SQL_TIMESTAMP;
                //len = sizeof(TIMESTAMP_STRUCT);
                len = 19;
                buff = param->conversionBuffer();
                if (!dateTimeToTimestamp(reinterpret_cast<TIMESTAMP_STRUCT*>(param->conversionBuffer()), 
                    param->get<DateTime>(), false))
                {
                    paramType = SQL_C_CHAR;
                    sqlType = SQL_CHAR;
                    *static_cast<char*>(buff) = 0;
                }
                break;

            case VariantDataType::VAR_BUFFER:
                paramType = SQL_C_BINARY;
                sqlType = SQL_LONGVARBINARY;
                len = static_cast<SQLLEN>(param->dataSize());
                buff = (void*) param->getText();
                param->callbackLength() = len;
#ifdef _WIN32
                cbValue = (SQLLEN*) &param->callbackLength();
#else
                cbValue = &param->callbackLength();
#endif
                break;

            case VariantDataType::VAR_IMAGE_NDX:
            case VariantDataType::VAR_IMAGE_PTR:
            case VariantDataType::VAR_MONEY:
            case VariantDataType::VAR_NONE:
                throw DatabaseException(
                    "Unsupported parameter type(" + to_string((int) param->dataType()) + ") for parameter '" +
                    param->name() + "'");
        }

        const auto rc = SQLBindParameter(query->statement(), (SQLUSMALLINT) paramNumber, parameterMode, paramType, sqlType,
                                         len, scale, buff, len, cbValue);
        if (rc != SQL_SUCCESS)
        {
            param->binding().reset(false);
            THROW_QUERY_ERROR(query, "Can't bind parameter " << paramNumber)
        }
    }
}

void ODBCConnection::queryBindParameters(Query* query)
{
    scoped_lock lock(*m_connect);

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

    int count = queryColCount(query);

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

static uint32_t trimField(char* s, uint32_t sz)
{
    char* p = s + sz;
    char ch = s[0];
    s[0] = '!';

    while (*p == ' ')
    {
        --p;
    }
    *(++p) = 0;

    if (ch == ' ' && s[1] == 0)
    {
        s[0] = 0;
        return 0;
    }

    s[0] = ch;
    return uint32_t(p - s);
}

SQLRETURN sptk::ODBC_readStringOrBlobField(SQLHSTMT statement, DatabaseField* dbField, SQLUSMALLINT column,
                                           int16_t fieldType, SQLLEN& dataLength)
{
    auto* field = dynamic_cast<ODBCField*>(dbField);

    auto& buffer = field->get<Buffer>();

    // Get data length
    auto rc = SQLGetData(statement, column, fieldType, buffer.data(), SQLINTEGER(0), &dataLength);
    if (!successful(rc))
    {
        return rc;
    }

    if (dataLength == 0 || (dataLength < 0 && dataLength != SQL_NO_TOTAL))
    {
        return SQL_SUCCESS;
    }

    if (dataLength != SQL_NO_TOTAL)
    {
        field->checkSize(uint32_t(dataLength + 1));

        // Read data
        rc = SQLGetData(statement, column, fieldType, buffer.data(), SQLINTEGER(dataLength + 1), &dataLength);
        return rc;
    }

    // Fetch BLOB data until there is no more data
    constexpr size_t initialReadSize = 16384;
    field->checkSize(initialReadSize);

    size_t bufferSize = field->bufferSize();
    SQLLEN remainingSize = 0;

    SQLLEN offset = 0;
    dataLength = 0;
    SQLLEN readSize = initialReadSize;
    while (successful(rc))
    {
        bufferSize += readSize;
        field->checkSize(bufferSize);
        rc = SQLGetData(statement, column, fieldType, buffer.data() + offset, SQLINTEGER(readSize), &remainingSize);
        if (remainingSize > 0)
        {
            readSize = remainingSize;
        } // Last chunk received
        offset += readSize - 1;
        dataLength += readSize - 1;
    }

    return rc == SQL_NO_DATA ? SQL_SUCCESS : rc;
}

SQLRETURN sptk::ODBC_readTimestampField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column,
                                        int16_t fieldType, SQLLEN& dataLength)
{
    TIMESTAMP_STRUCT t = {};
    SQLRETURN rc = SQLGetData(statement, column, fieldType, (SQLPOINTER) &t, 0, &dataLength);
    if (dataLength > 0)
    {
        DateTime dt(t.year, (short) t.month, (short) t.day, (short) t.hour, (short) t.minute, (short) t.second);
        field->setDateTime(dt, field->dataType() == VariantDataType::VAR_DATE);
    }
    return rc;
}

void ODBCConnection::queryFetch(Query* query)
{
    if (!query->active())
        THROW_QUERY_ERROR(query, "Dataset isn't open")

    auto* statement = query->statement();

    scoped_lock lock(*m_connect);

    int rc = SQLFetch(statement);

    if (!successful(rc))
    {
        if (rc < 0)
        {
            THROW_QUERY_ERROR(query, queryError(query))
        }
        else
        {
            querySetEof(query, rc == SQL_NO_DATA);
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

            rc = SQL_SUCCESS;
            switch (fieldType)
            {
                case SQL_C_SLONG:
                    rc = SQLGetData(statement, column, fieldType, &field->get<int64_t>(), 0, &dataLength);
                    break;

                case SQL_C_DOUBLE:
                    rc = SQLGetData(statement, column, fieldType, &field->get<double>(), 0, &dataLength);
                    break;

                case SQL_C_TIMESTAMP:
                    rc = ODBC_readTimestampField(statement, field, column, fieldType, dataLength);
                    break;

                case SQL_C_BINARY:
                case SQL_C_CHAR:
                    rc = ODBC_readStringOrBlobField(statement, field, column, fieldType, dataLength);
                    break;

                case SQL_BIT:
                    rc = SQLGetData(statement, column, fieldType, &field->get<bool>(), 1, &dataLength);
                    break;

                default:
                    dataLength = 0;
                    break;
            }

            if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
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
    array<SQLCHAR, MAX_BUF> datasrc = {0};
    array<SQLCHAR, MAX_BUF> descrip = {0};
    SQLSMALLINT rdsrc = 0;
    SQLSMALLINT rdesc = 0;

    SQLHENV hEnv = ODBCConnectionBase::getEnvironment().handle();
    bool offline = hEnv == nullptr;
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
        const SQLRETURN ret = SQLDataSources(
            hEnv, direction,
            datasrc.data(), (SQLSMALLINT) datasrc.size(), &rdsrc,
            descrip.data(), (SQLSMALLINT) descrip.size(), &rdesc);
        if (ret == SQL_NO_DATA)
        {
            break;
        }
        direction = SQL_FETCH_NEXT;
        dsns.push_back(String((char*) datasrc.data()) + " (" + String((char*) descrip.data()) + ")");
    }

    if (offline)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }
}

void ODBCConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    scoped_lock lock(*m_connect);

    if (objectType == DatabaseObjectType::DATABASES)
    {
        listDataSources(objects);
        return;
    }

    SQLHSTMT stmt = nullptr;
    try
    {
        Buffer objectSchema(MAX_NAME_LEN);
        Buffer objectName(MAX_NAME_LEN);
        short procedureType = 0;

        stmt = makeObjectListStatement(objectType, objectSchema, objectName, procedureType);

        while (true)
        {
            objectSchema[0] = 0;
            objectName[0] = 0;
            SQLRETURN rc = SQLFetch(stmt);

            if (rc == SQL_NO_DATA_FOUND)
            {
                break;
            }

            if (!successful(rc))
            {
                throw DatabaseException("SQLFetch");
            }

            String objectNameStr = String((char*) objectName.data()).replace(";0$", "");

            if (objectType == DatabaseObjectType::FUNCTIONS && procedureType != SQL_PT_FUNCTION)
            {
                continue;
            }

            if (objectType == DatabaseObjectType::PROCEDURES && procedureType != SQL_PT_PROCEDURE)
            {
                continue;
            }

            objects.push_back(String((char*) objectSchema.data()) + "." + objectNameStr);
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

SQLHSTMT ODBCConnection::makeObjectListStatement(const DatabaseObjectType& objectType, Buffer& objectSchema, Buffer& objectName, short& procedureType) const
{
    procedureType = 0;
    SQLRETURN rc;

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
            rc = SQLProcedures(stmt, nullptr, 0, Buffer("").data(), SQL_NTS,
                               Buffer("%").data(), SQL_NTS);
            if (rc != SQL_SUCCESS)
            {
                throw DatabaseException("SQLProcedures");
            }
            break;

        case DatabaseObjectType::DATABASES:
        case DatabaseObjectType::UNDEFINED:
            break;
    }

    SQLLEN cbObjectSchema = 0;
    SQLLEN cbObjectName = 0;

    if (SQLBindCol(stmt, 2, SQL_C_CHAR, objectSchema.data(), (SQLLEN) objectSchema.capacity() / 2, &cbObjectSchema) != SQL_SUCCESS)
    {
        throw DatabaseException("SQLBindCol");
    }

    if (SQLBindCol(stmt, 3, SQL_C_CHAR, objectName.data(), (SQLLEN) objectName.capacity() / 2, &cbObjectName) != SQL_SUCCESS)
    {
        throw DatabaseException("SQLBindCol");
    }

    if (objectType == DatabaseObjectType::FUNCTIONS || objectType == DatabaseObjectType::PROCEDURES)
    {
        constexpr SQLSMALLINT procedureTypeColumn = 8;
        rc = SQLBindCol(stmt, procedureTypeColumn, SQL_C_SHORT, &procedureType, sizeof(procedureType), nullptr);
        if (rc != SQL_SUCCESS)
        {
            throw DatabaseException("SQLBindCol");
        }
    }

    return stmt;
}

void ODBCConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    RegularExpression matchStatementEnd("(;\\s*)$");
    RegularExpression matchRoutineStart("^CREATE\\s+FUNCTION", "i");
    RegularExpression matchGo("^\\s*GO\\s*$", "i");
    RegularExpression matchCommentRow("^\\s*--");

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

void ODBCConnection::_bulkInsert(const String& tableName, const Strings& columnNames, const vector<VariantVector>& data)
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

void* odbc_create_connection(const char* connectionString)
{
    auto connection = make_shared<ODBCConnection>(connectionString);
    ODBCConnection::s_odbcConnections[connection.get()] = connection;
    return connection.get();
}

void odbc_destroy_connection(void* connection)
{
    ODBCConnection::s_odbcConnections.erase((ODBCConnection*) connection);
}
