/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-06                                             ║
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

#include "sptk5/db/BulkQuery.h"
#include <array>
#include <sptk5/RegularExpression.h>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/ODBCConnection.h>
#include <sql.h>
#include <string>
#include <sstream> // libc++

constexpr size_t MaxBufferSize = 1024;
constexpr size_t MaxErrorLen = 1024;

using namespace std;
using namespace sptk;

namespace sptk
{
    class ODBCField
        : public DatabaseField
    {
        friend class ODBCConnection;

    public:
        ODBCField(const String& fieldName, const int fieldType, int dateTimeVariant, const VariantDataType dataType,
                  const size_t fieldLength, const int fieldScale)
            : DatabaseField(fieldName, fieldType, dataType, fieldLength, fieldScale)
              , dateTimeVariant(dateTimeVariant)
        {
        }

        using DatabaseField::operator=;
        int dateTimeVariant{0}; ///< 0 for datetime, 1 for datetime2 and timestamp
        bool exactNumeric{false}; ///< NUMERIC/DECIMAL read as exact text rather than double
    };
} // namespace sptk

namespace
{
    [[nodiscard]] bool successful(const int ret)
    {
        return ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO;
    }

    SQLRETURN odbcReadExactNumericField(const SQLHSTMT statement, DatabaseField* field, const SQLUSMALLINT column,
                                        SQLLEN& dataLength, std::mutex& mutex)
    {
        auto* odbcField = dynamic_cast<ODBCField*>(field);
        auto& buffer = odbcField->get<Buffer>();

        // DECIMAL/NUMERIC are fixed-length on the server. SQLGetData cannot return fixed-length data
        // in parts: the length probe followed by a read returns SQL_NO_DATA ("no more data") on some
        // drivers. Read the exact decimal text in a single call instead.
        const size_t bufferSize = max<size_t>(64, field->fieldSize() + 1);
        odbcField->checkSize(bufferSize);
        SQLRETURN resultCode;
        {
            const scoped_lock lock(mutex);
            resultCode = SQLGetData(statement, column, SQL_C_CHAR, buffer.data(),
                                    static_cast<SQLINTEGER>(bufferSize), &dataLength);
        }
        if (successful(resultCode) && dataLength > 0)
        {
            odbcField->setDataSize(min<size_t>(static_cast<size_t>(dataLength), bufferSize - 1));
        }
        return resultCode;
    }

    SQLRETURN odbcReadTimestampField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column, SQLLEN& dataLength,
                                     chrono::minutes sessionTimezoneOffset);
    void odbcQueryBindParameter(const Query* query, QueryParameter* parameter);
} // namespace

ODBCConnection::ODBCConnection(const String& connectionString, const std::chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::GENERIC_ODBC, connectTimeout)
{
}

namespace
{
    /// ODBC connection-string values containing delimiters or whitespace must be brace-quoted;
    /// a brace inside such a value is doubled.
    String odbcQuote(const String& value)
    {
        if (value.empty() || value.find_first_of(";={}, \t\r\n") == String::npos)
        {
            return value;
        }

        String escaped;
        escaped.reserve(value.size() + 2);
        escaped.push_back('{');
        for (const char c : value)
        {
            if (c == '{' || c == '}')
            {
                escaped.push_back(c);
            }
            escaped.push_back(c);
        }
        escaped.push_back('}');
        return escaped;
    }
} // namespace

String ODBCConnection::nativeConnectionString() const
{
    const DatabaseConnectionString& connString = connectionString();
    stringstream result;
    result << "DSN=" << odbcQuote(connString.hostName());
    if (!connString.userName().empty())
    {
        result << ";UID=" << odbcQuote(connString.userName());
    }
    if (!connString.password().empty())
    {
        result << ";PWD=" << odbcQuote(connString.password());
    }
    if (!connString.databaseName().empty())
    {
        result << ";DATABASE=" << odbcQuote(connString.databaseName());
    }

    // Every connection-string parameter must reach the driver, not just the four named ones.
    // The named fields above are skipped to avoid duplicates in the case-insensitive map.
    for (const auto& [name, value] : connString.parameters())
    {
        if (name == "DSN" || name == "UID" || name == "PWD" || name == "DATABASE")
        {
            continue;
        }
        result << ';' << name << '=' << odbcQuote(value);
    }
    return result.str();
}

chrono::minutes ODBCConnection::getSessionTimezoneOffset()
{
    // Ask the server directly instead of going through Query: that needs a shared_ptr to this
    // connection, so a stack-allocated ODBCConnection could not be opened at all.
    const scoped_lock lock(*m_connect);
    if (m_timezoneKnown)
    {
        return m_sessionTimezoneOffset;
    }

    SQLHSTMT stmt = SQL_NULL_HSTMT;
    if (!successful(SQLAllocStmt(handle(), &stmt)))
    {
        throw DatabaseException(
            String("Can't allocate timezone statement: ") + queryError(static_cast<SQLHSTMT>(nullptr)));
    }

    String sql("SELECT CURRENT_TIMESTAMP");
    const auto localBefore = DateTime::Now();
    const auto resultCode = SQLExecDirect(stmt, bit_cast<SQLCHAR*>(sql.data()), SQL_NTS);
    const auto localAfter = DateTime::Now();

    TIMESTAMP_STRUCT timestampStruct = {};
    SQLLEN dataLength = 0;
    if (successful(resultCode) && successful(SQLFetch(stmt)))
    {
        SQLGetData(stmt, 1, SQL_C_TIMESTAMP, &timestampStruct, sizeof(timestampStruct), &dataLength);
    }
    SQLFreeStmt(stmt, SQL_DROP);

    if (!successful(resultCode) || dataLength <= 0)
    {
        throw DatabaseException("Can't read server timestamp for timezone offset");
    }

    const DateTime serverTime(timestampStruct.year, static_cast<short>(timestampStruct.month),
                              static_cast<short>(timestampStruct.day), static_cast<short>(timestampStruct.hour),
                              static_cast<short>(timestampStruct.minute), static_cast<short>(timestampStruct.second));

    // Sampling local time on both sides of the query removes most of the round-trip bias. The
    // remaining sub-second part cannot tip a whole-minute offset because we keep full precision
    // until the final conversion.
    const DateTime midpoint = localBefore + (localAfter - localBefore) / 2;
    m_sessionTimezoneOffset = chrono::duration_cast<chrono::minutes>(serverTime - midpoint);
    m_timezoneKnown = true;
    return m_sessionTimezoneOffset;
}

void ODBCConnection::_openDatabase(const String& newConnectionString)
{
    // Serialize connection state: two threads must not both connect or both read the timezone.
    const scoped_lock stateLock(m_stateMutex);
    if (!active())
    {
        {
            const scoped_lock lock(*m_connect);
            setInTransaction(false);
        }
        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        // Cached statements belong to the old physical connection.
        clearPreparedStatements();

        // A new physical connection may live in a different timezone.
        m_timezoneKnown = false;

        String finalConnectionString;
        m_connect->connect(nativeConnectionString(), finalConnectionString, false);
        if (m_connect->driverDescription().find("Microsoft SQL Server") != string::npos)
        {
            connectionType(DatabaseConnectionType::MSSQL_ODBC);
        }

        getSessionTimezoneOffset();
    }
}

void ODBCConnection::closeDatabase()
{
    disconnectAllQueries();
    clearPreparedStatements();
    const scoped_lock stateLock(m_stateMutex);
    m_timezoneKnown = false;
    m_connect->freeConnect();
}

DBHandle ODBCConnection::handle() const
{
    return static_cast<DBHandle>(m_connect->handle());
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

    {
        const scoped_lock stateLock(m_stateMutex);
        if (getInTransaction())
        {
            logAndThrow("ODBCConnection::driverBeginTransaction", "Transaction already started.");
        }
    }

    m_connect->beginTransaction();
    {
        const scoped_lock stateLock(m_stateMutex);
        setInTransaction(true);
    }
}

void ODBCConnection::driverEndTransaction(const bool commit)
{
    {
        const scoped_lock stateLock(m_stateMutex);
        if (!getInTransaction())
        {
            if (commit)
            {
                logAndThrow("ODBCConnection::driverEndTransaction", "Can't commit - transaction isn't started.");
            }
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

    m_connect->setConnectOption(SQL_ATTR_AUTOCOMMIT, true);
    {
        const scoped_lock stateLock(m_stateMutex);
        setInTransaction(false);
    }
}

//-----------------------------------------------------------------------------------------------

String ODBCConnection::queryError(const SQLHSTMT stmt) const
{
    array<SQLCHAR, SQL_MAX_MESSAGE_LENGTH> message = {};
    array<SQLCHAR, 8> state = {};
    SQLINTEGER nativeError = 0;
    SQLSMALLINT messageLength = 0;

    // Diagnostics belong to the handle that failed. SQLError() reports the last error on the
    // connection, which may be unrelated; SQLGetDiagRec() keeps the two separate.
    const auto read = [&](const SQLSMALLINT handleType, const SQLHANDLE handle)
    {
        return successful(SQLGetDiagRec(handleType, handle, 1, state.data(), &nativeError, message.data(),
                                        static_cast<SQLSMALLINT>(message.size()), &messageLength));
    };

    String error;
    if (stmt != SQL_NULL_HSTMT && read(SQL_HANDLE_STMT, stmt))
    {
        error = bit_cast<const char*>(message.data());
    }
    else if (read(SQL_HANDLE_DBC, handle()))
    {
        error = bit_cast<const char*>(message.data());
    }
    else
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
    queryFreeStmt(query); // caches a prepared statement for reuse instead of dropping it

    const scoped_lock lock(*m_connect);
    if (query->autoPrepare())
    {
        if (const auto cached = m_preparedStatements.find(query->sql()); cached != m_preparedStatements.end())
        {
            // Handed out exclusively: the statement is removed from the cache until this query
            // frees it, so two open queries never share one result set.
            const SStmtHandle handle = cached->second;
            m_preparedStatements.erase(cached);
            m_preparedStatementKeys.erase(find(m_preparedStatementKeys.begin(), m_preparedStatementKeys.end(),
                                               query->sql()));
            SQLFreeStmt(static_cast<SQLHSTMT>(handle.get()), SQL_CLOSE);
            querySetStmt(query, handle);
            querySetPrepared(query, true);
            return;
        }
    }

    // The previous statement is owned by the connection's shared_ptr. Assigning the new handle
    // below releases it exactly once; freeing it here as well would drop the same HSTMT twice.
    SQLHSTMT hStmt = SQL_NULL_HSTMT;

    if (auto* hdb = handle();
        !successful(SQLAllocStmt(hdb, &hStmt)))
    {
        const String error = queryError(query);
        querySetStmt(query, SQL_NULL_HSTMT);
        logAndThrow("ODBCConnection::queryAllocStmt", error);
    }

    const auto statement = shared_ptr<uint8_t>(static_cast<StmtHandle>(hStmt),
                                               [](const StmtHandle ptr)
                                               {
                                                   SQLFreeStmt((SQLHSTMT)ptr, SQL_DROP);
                                               });
    querySetStmt(query, statement);
}

void ODBCConnection::queryFreeStmt(Query* query)
{
    std::vector<SStmtHandle> evicted;
    SStmtHandle discarded;
    {
        const scoped_lock lock(*m_connect);
        const SStmtHandle handle = queryStatement(query);
        if (query->prepared())
        {
            evicted = cacheStatement(query->sql(), handle);
        }
        else
        {
            // Release a direct statement after unlocking: SQLFreeStmt is a driver call.
            discarded = handle;
        }
        querySetStmt(query, nullptr);
        querySetPrepared(query, false);
    }
}

std::vector<SStmtHandle> ODBCConnection::cacheStatement(const String& sql, const SStmtHandle& statement)
{
    std::vector<SStmtHandle> evicted;
    if (sql.empty() || statement == nullptr)
    {
        return evicted;
    }

    // The caller holds the connection mutex. Evicted statements are returned, not destroyed here:
    // their deleter calls SQLFreeStmt, which must not run under the lock.
    SQLFreeStmt(static_cast<SQLHSTMT>(statement.get()), SQL_CLOSE);

    if (const auto existing = m_preparedStatements.find(sql); existing != m_preparedStatements.end())
    {
        evicted.push_back(existing->second);
        m_preparedStatements.erase(existing);
        m_preparedStatementKeys.erase(find(m_preparedStatementKeys.begin(), m_preparedStatementKeys.end(), sql));
    }

    m_preparedStatements.emplace(sql, statement);
    m_preparedStatementKeys.push_back(sql);

    while (m_preparedStatementKeys.size() > maxCachedStatements)
    {
        const String oldest = m_preparedStatementKeys.front();
        m_preparedStatementKeys.pop_front();
        if (const auto itor = m_preparedStatements.find(oldest); itor != m_preparedStatements.end())
        {
            evicted.push_back(itor->second);
            m_preparedStatements.erase(itor);
        }
    }
    return evicted;
}

void ODBCConnection::clearPreparedStatements()
{
    const scoped_lock lock(*m_connect);
    m_preparedStatements.clear();
    m_preparedStatementKeys.clear();
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

    const auto buffer(query->sql());
    // For some reason, SQLPrepare doesn't work correctly without this copy when using cxx11 ABI
    const auto* sql = buffer.c_str();
    if (buffer.empty())
    {
        THROW_QUERY_ERROR(query, "PREPARE command failed: EMPTY QUERY");
    }
    if (!successful(SQLPrepare(query->statement(), bit_cast<SQLCHAR*>(sql), SQL_NTS)))
    {
        THROW_QUERY_ERROR(query, queryError(query));
    }
}

void ODBCConnection::queryExecute(Query* query)
{
    const scoped_lock lock(*m_connect);

    int result;
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
        result = SQLExecDirect(query->statement(), bit_cast<SQLCHAR*>(query->sql().data()), SQL_NTS);
    }

    if (successful(result))
    {
        return;
    }

    QueryParameter* parameter;
    while (result == SQL_NEED_DATA)
    {
        result = SQLParamData(query->statement(), bit_cast<SQLPOINTER*>(&parameter));
        if (result == SQL_NEED_DATA)
        {
            const auto len = static_cast<SQLLEN>(parameter->dataSize());
            if (const auto buff = bit_cast<SQLPOINTER>(parameter->getText());
                SQLPutData(query->statement(), buff, len) != SQL_SUCCESS)
            {
                break;
            }
            continue;
        }
        if (result == SQL_SUCCESS)
        {
            return;
        }
        break;
    }

    constexpr auto diagRecordSize = 16;
    array<SQLCHAR, diagRecordSize> state = {};
    array<SQLCHAR, MaxErrorLen> text = {};
    SQLINTEGER nativeError = 0;
    SQLSMALLINT recordCount = 0;
    SQLSMALLINT textLength = 0;

    result = SQLGetDiagField(SQL_HANDLE_STMT, query->statement(), 1, SQL_DIAG_NUMBER, &recordCount, sizeof(recordCount),
                             &textLength);

    Strings errors;
    SQLSMALLINT recordNumber = 1;
    while (successful(result))
    {
        result = SQLGetDiagRec(SQL_HANDLE_STMT, query->statement(), recordNumber, state.data(), &nativeError,
                               text.data(), static_cast<SQLSMALLINT>(text.size()), &textLength);
        if (successful(result))
        {
            errors.push_back(removeDriverIdentification(bit_cast<const char*>(text.data())));
        }
        ++recordNumber;
    }

    if (!errors.empty())
    {
        THROW_QUERY_ERROR(query, errors.join("; "));
    }

    if (!successful(result))
    {
        THROW_QUERY_ERROR(query, queryError(query));
    }
}

size_t ODBCConnection::queryColCount(Query* query)
{
    const scoped_lock lock(*m_connect);

    SQLSMALLINT count = 0;
    if (!successful(SQLNumResultCols(query->statement(), &count)))
    {
        THROW_QUERY_ERROR(query, queryError(query));
    }

    return static_cast<size_t>(count);
}

void ODBCConnection::queryColAttributesUnlocked(Query* query, const int16_t column, const int16_t attribute,
                                                int32_t& value)
{
    SQLLEN result = 0;

    if (!successful(
        SQLColAttributes(query->statement(), static_cast<SQLUSMALLINT>(column), static_cast<SQLUSMALLINT>(attribute),
                         nullptr, 0, nullptr, &result)))
    {
        THROW_QUERY_ERROR(query, queryError(query));
    }

    value = static_cast<int32_t>(result);
}

void ODBCConnection::queryColAttributes(Query* query, const int16_t column, const int16_t attribute, int32_t& value)
{
    const scoped_lock lock(*m_connect);
    queryColAttributesUnlocked(query, column, attribute, value);
}

void ODBCConnection::queryColAttributesUnlocked(Query* query, const int16_t column, const int16_t attribute,
                                                const LPSTR buff, const int len)
{
    int16_t available = 0;
    if (buff == nullptr || len <= 0)
    {
        THROW_QUERY_ERROR(query, "Invalid buffer or buffer len");
    }

    if (!successful(
        SQLColAttributes(query->statement(), static_cast<SQLUSMALLINT>(column), static_cast<SQLUSMALLINT>(attribute),
                         buff, static_cast<int16_t>(len), &available, nullptr)))
    {
        THROW_QUERY_ERROR(query, queryError(query));
    }
}

void ODBCConnection::queryColAttributes(Query* query, const int16_t column, const int16_t attribute, const LPSTR buff,
                                        const int len)
{
    const scoped_lock lock(*m_connect);
    queryColAttributesUnlocked(query, column, attribute, buff, len);
}

namespace
{
    bool dateTimeToTimestamp(TIMESTAMP_STRUCT* timestampStruct, const DateTime& dateTime, const bool dateOnly)
    {
        if (!dateTime.zero())
        {
            short weekDay = 0;
            short yearDay = 0;
            short milliseconds = 0;
            dateTime.decodeDate(&timestampStruct->year, bit_cast<int16_t*>(&timestampStruct->month),
                                bit_cast<int16_t*>(&timestampStruct->day),
                                &weekDay, &yearDay);
            if (dateOnly)
            {
                timestampStruct->hour = timestampStruct->minute = timestampStruct->second = 0;
            }
            else
            {
                dateTime.decodeTime(bit_cast<int16_t*>(&timestampStruct->hour),
                                    bit_cast<int16_t*>(&timestampStruct->minute),
                                    bit_cast<int16_t*>(&timestampStruct->second), &milliseconds);
            }
            constexpr auto msInSecond = 1000;
            timestampStruct->fraction = static_cast<SQLUINTEGER>(milliseconds * msInSecond);
            return true;
        }
        return false;
    }

    namespace
    {
        void bindLOB(QueryParameter* parameter, SQLPOINTER& buff, SQLLEN& len, SQLLEN*& cbValue)
        {
            len = static_cast<SQLLEN>(parameter->dataSize());
            buff = bit_cast<void*>(parameter->getText());
#ifdef _WIN32
            cbValue = bit_cast<SQLLEN*>(&parameter->callbackLength());
#else
            cbValue = &parameter->callbackLength();
#endif
            if (constexpr auto minBlobSize = 256;
                len >= minBlobSize)
            {
                *cbValue = SQL_LEN_DATA_AT_EXEC(static_cast<SQLLEN>(parameter->dataSize()));
                buff = static_cast<SQLPOINTER>(parameter);
            }
            else
            {
                // The indicator is read by SQLBindParameter. Leaving it at its initial zero sends an
                // empty value for every short text or binary parameter.
                *cbValue = len;
            }
        }
    } // namespace

    void odbcQueryBindParameter(const Query* query, QueryParameter* parameter)
    {
        static constexpr auto dateAccuracy = 19;
        static SQLLEN cbNullValue = SQL_NULL_DATA;

        const VariantDataType variantDataType = parameter->dataType();
        for (unsigned j = 0; j < parameter->bindCount(); ++j)
        {
            int16_t paramType;
            int16_t valueType;
            constexpr int16_t scale = 0;
            void* buff;
            SQLLEN len = 0;
            const auto paramNumber = static_cast<int16_t>(parameter->bindIndex(j) + 1);

            SQLLEN* cbValue = nullptr;
            if (parameter->isNull())
            {
                cbValue = &cbNullValue;
                len = 0;
            }

            constexpr int16_t inputOutputMode = SQL_PARAM_INPUT;
            switch (variantDataType)
            {
            case VariantDataType::VAR_BOOL:
                paramType = SQL_C_BIT;
                valueType = SQL_BIT;
                buff = bit_cast<void*>(&parameter->get<bool>());
                break;

            case VariantDataType::VAR_INT:
                paramType = SQL_C_SLONG;
                valueType = SQL_INTEGER;
                buff = bit_cast<void*>(&parameter->get<int>());
                break;

            case VariantDataType::VAR_INT64:
                paramType = SQL_C_SBIGINT;
                valueType = SQL_BIGINT;
                buff = bit_cast<void*>(&parameter->get<int64_t>());
                break;

            case VariantDataType::VAR_FLOAT:
                paramType = SQL_C_DOUBLE;
                valueType = SQL_DOUBLE;
                buff = bit_cast<void*>(&parameter->get<double>());
                break;

            case VariantDataType::VAR_STRING:
                len = static_cast<long>(parameter->dataSize());
                paramType = SQL_C_CHAR;
                // SQL_C_CHAR is a narrow buffer: pairing it with SQL_WVARCHAR makes the driver
                // interpret the bytes as UTF-16.
                valueType = SQL_VARCHAR;
                buff = bit_cast<void*>(parameter->getText());
                break;

            case VariantDataType::VAR_TEXT:
                paramType = SQL_C_CHAR;
                valueType = SQL_LONGVARCHAR;
                bindLOB(parameter, buff, len, cbValue);
                break;

            case VariantDataType::VAR_DATE:
                paramType = SQL_C_TIMESTAMP;
                valueType = SQL_TIMESTAMP;
                len = sizeof(TIMESTAMP_STRUCT);
                buff = parameter->conversionBuffer();
                if (!dateTimeToTimestamp(bit_cast<TIMESTAMP_STRUCT*>(parameter->conversionBuffer()),
                                         parameter->get<DateTime>(), true))
                {
                    paramType = SQL_C_CHAR;
                    valueType = SQL_CHAR;
                    *bit_cast<char*>(buff) = 0;
                }
                break;

            case VariantDataType::VAR_DATE_TIME:
                paramType = SQL_C_TIMESTAMP;
                valueType = SQL_TIMESTAMP;
                len = dateAccuracy;
                buff = parameter->conversionBuffer();
                if (!dateTimeToTimestamp(bit_cast<TIMESTAMP_STRUCT*>(parameter->conversionBuffer()),
                                         parameter->get<DateTime>(), false))
                {
                    paramType = SQL_C_CHAR;
                    valueType = SQL_CHAR;
                    *static_cast<char*>(buff) = 0;
                }
                break;

            case VariantDataType::VAR_BUFFER:
                paramType = SQL_C_BINARY;
                valueType = SQL_LONGVARBINARY;
                bindLOB(parameter, buff, len, cbValue);
                break;

            default:
                throw DatabaseException(
                    format("Unsupported parameter type {} for parameter '{}'", static_cast<int>(parameter->dataType()),
                           parameter->name()));
            }

            // For character data BufferLength must include room for the terminating zero.
            const SQLLEN bufferLength = paramType == SQL_C_CHAR ? len + 1 : len;
            const auto resultCode = SQLBindParameter(query->statement(), static_cast<SQLUSMALLINT>(paramNumber),
                                                     inputOutputMode,
                                                     paramType, valueType, len, scale, buff, bufferLength, cbValue);
            if (!successful(resultCode))
            {
                parameter->binding().reset(false);
                THROW_QUERY_ERROR(query, format("Can't bind parameter {}", paramNumber));
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
        odbcQueryBindParameter(query, param);
    }
}

void ODBCConnection::odbcTypeToCType(const int32_t odbcType, int32_t& ctype, VariantDataType& dataType)
{
    switch (odbcType)
    {
    case SQL_TINYINT:
    case SQL_SMALLINT:
        ctype = SQL_C_SSHORT;
        dataType = VariantDataType::VAR_INT;
        break;

    case SQL_INTEGER:
        ctype = SQL_C_SLONG;
        dataType = VariantDataType::VAR_INT;
        break;

    case SQL_BIGINT:
        // SQL_C_SLONG writes only four bytes into the 64-bit field, corrupting negative values
        // and anything above INT32_MAX.
        ctype = SQL_C_SBIGINT;
        dataType = VariantDataType::VAR_INT64;
        break;

    case SQL_NUMERIC:
    case SQL_DECIMAL:
        // An exact decimal cannot survive a round trip through double. Keep the driver's
        // textual representation; Variant still converts it to a number on request.
        ctype = SQL_C_CHAR;
        dataType = VariantDataType::VAR_STRING;
        break;

    case SQL_REAL:
    case SQL_DOUBLE:
    case SQL_FLOAT:
        ctype = SQL_C_DOUBLE;
        dataType = VariantDataType::VAR_FLOAT;
        break;

    case SQL_DATE: // ODBC 2.0 only
    case SQL_TYPE_DATE: // ODBC 3.0 only
        ctype = SQL_C_TIMESTAMP;
        dataType = VariantDataType::VAR_DATE;
        break;

    case SQL_TIME:
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIME:
    case SQL_TYPE_TIMESTAMP:
        ctype = SQL_C_TIMESTAMP;
        dataType = VariantDataType::VAR_DATE_TIME;
        break;

    case SQL_BINARY:
    case SQL_LONGVARBINARY:
    case SQL_VARBINARY:
        ctype = SQL_C_BINARY;
        dataType = VariantDataType::VAR_BUFFER;
        break;

    case SQL_BIT:
        ctype = SQL_C_BIT;
        dataType = VariantDataType::VAR_BOOL;
        break;

    default:
        ctype = SQL_C_CHAR;
        dataType = VariantDataType::VAR_STRING;
        break;
    }
}

void ODBCConnection::parseColumns(Query* query, const size_t count)
{
    // One lock for the whole column loop instead of four or five acquire/release pairs per column.
    const scoped_lock lock(*m_connect);

    // Reading the column attributes
    array<char, MAX_NAME_LEN> columnName = {};
    auto columnType = 0;
    auto columnLength = 0;
    auto columnScale = 0;
    auto cType = 0;
    auto dataType = VariantDataType::VAR_NONE;

    constexpr auto largeTextSize = 65536;
    constexpr auto maxColumnScale = 20;
    array<char, 64> columnTypeName = {};

    for (int16_t column = 1; column <= static_cast<int16_t>(count); ++column)
    {
        queryColAttributesUnlocked(query, column, SQL_COLUMN_NAME, columnName.data(), MAX_NAME_LEN - 1);
        queryColAttributesUnlocked(query, column, SQL_COLUMN_TYPE, columnType);
        queryColAttributesUnlocked(query, column, SQL_COLUMN_LENGTH, columnLength);
        queryColAttributesUnlocked(query, column, SQL_COLUMN_SCALE, columnScale);

        // Only the legacy SQL Server "datetime" type needs the session offset applied on read.
        // Everything else (datetime2, date, time, SQL_TYPE_TIMESTAMP) is already wall-clock.
        int dateTimeVariant = 1;
        if (columnType == SQL_TIMESTAMP || columnType == SQL_TYPE_TIMESTAMP || columnType == SQL_TYPE_DATE ||
            columnType == SQL_DATE || columnType == SQL_TIME || columnType == SQL_TYPE_TIME)
        {
            queryColAttributesUnlocked(query, column, SQL_COLUMN_TYPE_NAME, columnTypeName.data(),
                                       static_cast<int>(columnTypeName.size()) - 1);
            std::string typeName(columnTypeName.data());
            dateTimeVariant = typeName == "datetime" ? 0 : 1;
        }

        odbcTypeToCType(columnType, cType, dataType);

        if (dataType == VariantDataType::VAR_STRING && columnLength >= largeTextSize)
        {
            dataType = VariantDataType::VAR_TEXT;
        }

        columnLength = min(columnLength, FETCH_BUFFER_SIZE);

        if (dataType == VariantDataType::VAR_FLOAT && (columnScale < 0 || columnScale > maxColumnScale))
        {
            columnScale = 0;
        }

        auto field = make_shared<ODBCField>(columnName.data(), cType, dateTimeVariant, dataType, columnLength,
                                            columnScale);
        field->exactNumeric = columnType == SQL_NUMERIC || columnType == SQL_DECIMAL;
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

    queryBindParameters(query);

    if (query->autoPrepare() && !query->prepared())
    {
        queryPrepare(query);
        querySetPrepared(query, true);
    }

    queryExecute(query);

    const auto count = queryColCount(query);
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

namespace
{
    uint32_t trimField(char* str, const uint32_t size)
    {
        char* ptr = str + size - 1;
        const auto chr = str[0];
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
        return static_cast<uint32_t>(ptr - str);
    }

    SQLRETURN odbcReadStringOrBlobField(const SQLHSTMT statement, DatabaseField* field, const SQLUSMALLINT column,
                                        const int16_t fieldType, SQLLEN& dataLength, std::mutex& mutex)
    {
        auto* odbcField = dynamic_cast<ODBCField*>(field);
        auto& buffer = odbcField->get<Buffer>();

        // With BufferLength 0 the driver only reports the length (or SQL_NO_TOTAL for long data).
        SQLRETURN resultCode;
        {
            const scoped_lock lock(mutex);
            resultCode = SQLGetData(statement, column, fieldType, buffer.data(), 0, &dataLength);
        }
        if (!successful(resultCode))
        {
            return resultCode;
        }

        if (dataLength == SQL_NULL_DATA)
        {
            field->setNull(field->dataType());
            return SQL_SUCCESS;
        }

        if (dataLength == 0)
        {
            // An empty string or an empty blob is a value, not NULL.
            odbcField->setDataSize(0);
            return SQL_SUCCESS;
        }
        if (dataLength < 0 && dataLength != SQL_NO_TOTAL)
        {
            return resultCode;
        }

        if (dataLength != SQL_NO_TOTAL)
        {
            odbcField->checkSize(static_cast<size_t>(dataLength) + 1);
            {
                const scoped_lock lock(mutex);
                resultCode = SQLGetData(statement, column, fieldType, buffer.data(),
                                        static_cast<SQLINTEGER>(dataLength + 1), &dataLength);
            }
            if (successful(resultCode))
            {
                odbcField->setDataSize(static_cast<size_t>(dataLength));
            }
            return resultCode;
        }

        // Unknown length: read in chunks. Character data uses one byte of the buffer for the
        // terminator; binary data uses all of it. The next chunk starts exactly where the useful
        // bytes of the previous one ended.
        constexpr SQLLEN readSize = 16384;
        const SQLLEN capacity = fieldType == SQL_C_CHAR ? readSize - 1 : readSize;
        SQLLEN offset = 0;
        SQLLEN chunkLength = 0;
        for (;;)
        {
            odbcField->checkSize(static_cast<size_t>(offset + readSize));
            {
                const scoped_lock lock(mutex);
                resultCode = SQLGetData(statement, column, fieldType, buffer.data() + offset,
                                        static_cast<SQLINTEGER>(readSize), &chunkLength);
            }
            if (resultCode == SQL_NO_DATA)
            {
                break;
            }
            if (!successful(resultCode))
            {
                return resultCode;
            }

            const SQLLEN written = (chunkLength == SQL_NO_TOTAL || chunkLength > capacity) ? capacity : chunkLength;
            offset += written;
            // Buffer::reserve() only copies m_size bytes when it grows. Keep the size in sync with the
            // bytes already received, or the next chunk reallocation discards the earlier chunks.
            buffer.bytes(static_cast<size_t>(offset));

            // SQL_SUCCESS marks the last chunk; SQL_SUCCESS_WITH_INFO means there is more data.
            if (resultCode == SQL_SUCCESS || written == 0)
            {
                break;
            }
        }

        dataLength = offset;
        odbcField->setDataSize(static_cast<size_t>(dataLength));
        return SQL_SUCCESS;
    }

    SQLRETURN odbcReadTimestampField(const SQLHSTMT statement, DatabaseField* field, const SQLUSMALLINT column,
                                     SQLLEN& dataLength, const chrono::minutes sessionTimezoneOffset, std::mutex& mutex)
    {
        TIMESTAMP_STRUCT timestampStruct = {};
        SQLRETURN resultCode;
        {
            const scoped_lock lock(mutex);
            resultCode = SQLGetData(statement, column, SQL_C_TIMESTAMP,
                                    (SQLPOINTER)&timestampStruct, sizeof(timestampStruct), &dataLength);
        }
        if (!successful(resultCode) || dataLength <= 0)
        {
            return resultCode;
        }

        // TIMESTAMP_STRUCT.fraction is nanoseconds; DateTime stores milliseconds. The caller passes a
        // non-zero offset only for the old SQL Server 'datetime' type that carries one.
        const auto milliseconds = static_cast<short>(timestampStruct.fraction / 1000000);
        const DateTime dateTime(timestampStruct.year, static_cast<short>(timestampStruct.month),
                                static_cast<short>(timestampStruct.day), static_cast<short>(timestampStruct.hour),
                                static_cast<short>(timestampStruct.minute), static_cast<short>(timestampStruct.second),
                                milliseconds);
        field->setDateTime(dateTime - sessionTimezoneOffset, field->dataType() == VariantDataType::VAR_DATE);
        return resultCode;
    }
} // namespace


void ODBCConnection::queryFetch(Query* query)
{
    if (!query->active())
    {
        THROW_QUERY_ERROR(query, "Query isn't open");
    }

    auto* statement = query->statement();

    // Take the state lock before any connection lock: _openDatabase takes them in the opposite
    // order, and holding them in both orders would deadlock two concurrent threads.
    chrono::minutes sessionOffset;
    {
        const scoped_lock stateLock(m_stateMutex);
        sessionOffset = m_sessionTimezoneOffset;
    }

    SQLRETURN fetchResult;
    {
        // Only the ODBC call itself needs the connection lock. Decoding the row below is local
        // to this query and must not serialize other users of the connection.
        const scoped_lock lock(*m_connect);
        fetchResult = SQLFetch(statement);
    }

    if (!successful(fetchResult))
    {
        if (fetchResult < 0)
        {
            String error;
            {
                const scoped_lock lock(*m_connect);
                error = queryError(query);
            }
            THROW_QUERY_ERROR(query, error);
        }

        querySetEof(query, fetchResult == SQL_NO_DATA);
        return;
    }

    const auto fieldCount = query->fieldCount();
    SQLLEN dataLength = 0;

    if (fieldCount == 0)
    {
        return;
    }

    ODBCField* field = nullptr;
    for (SQLUSMALLINT column = 0; column < static_cast<SQLUSMALLINT>(fieldCount);)
    {
        try
        {
            field = dynamic_cast<ODBCField*>(&(*query)[column]);
            const auto fieldType = static_cast<int16_t>(field->fieldType());

            ++column;

            const auto getData = [&](const int16_t cType, SQLPOINTER buffer, const SQLINTEGER bufferLength)
            {
                const scoped_lock lock(*m_connect);
                return SQLGetData(statement, column, cType, buffer, bufferLength, &dataLength);
            };

            int resultCode = SQL_SUCCESS;
            switch (fieldType)
            {
            case SQL_C_SSHORT:
                {
                    // The driver writes two bytes. Read into a correctly sized value so that
                    // negative SMALLINT/TINYINT values are sign-extended, not zero-extended.
                    SQLSMALLINT value = 0;
                    resultCode = getData(fieldType, &value, 0);
                    if (successful(resultCode) && dataLength > 0)
                    {
                        field->setInteger(value);
                    }
                    break;
                }

            case SQL_C_SLONG:
                resultCode = getData(fieldType, &field->get<int>(), 0);
                break;

            case SQL_C_SBIGINT:
                resultCode = getData(fieldType, &field->get<int64_t>(), 0);
                break;

            case SQL_C_DOUBLE:
                resultCode = getData(fieldType, &field->get<double>(), 0);
                break;

            case SQL_C_TIMESTAMP:
                if (field->dateTimeVariant == 0)
                {
                    // The old datetime type requires TZ offset.
                    resultCode =
                        odbcReadTimestampField(statement, field, column, dataLength, sessionOffset, *m_connect);
                }
                else
                {
                    // The new datetime2 type doesn't require TZ offset.
                    resultCode = odbcReadTimestampField(statement, field, column, dataLength, chrono::minutes(0),
                                                        *m_connect);
                }
                break;

            case SQL_C_BINARY:
                resultCode = odbcReadStringOrBlobField(statement, field, column, fieldType, dataLength, *m_connect);
                break;

            case SQL_C_CHAR:
                if (field->exactNumeric)
                {
                    resultCode = odbcReadExactNumericField(statement, field, column, dataLength, *m_connect);
                }
                else
                {
                    resultCode = odbcReadStringOrBlobField(statement, field, column, fieldType, dataLength, *m_connect);
                }
                break;

            case SQL_BIT:
                resultCode = getData(fieldType, &field->get<bool>(), 1);
                break;

            default:
                dataLength = 0;
                break;
            }

            if (resultCode != SQL_SUCCESS && resultCode != SQL_SUCCESS_WITH_INFO)
            {
                String error;
                {
                    const scoped_lock lock(*m_connect);
                    error = queryError(query);
                }
                throw Exception(error);
            }

            if (fieldType == SQL_C_CHAR && dataLength > 0)
            {
                dataLength = bit_cast<SQLINTEGER>(trimField(bit_cast<char*>(field->get<Buffer>().data()),
                                                            static_cast<uint32_t>(dataLength)));
            }

            if (dataLength < 0)
            {
                field->setNull(field->dataType());
            }
            else
            {
                // Zero is a valid length: an empty string or empty blob, not NULL.
                field->dataSize(static_cast<size_t>(dataLength));
            }
        }
        catch (const Exception& e)
        {
            const auto fieldName = field != nullptr ? field->fieldName() : "";
            Query::throwError("ODBCConnection::queryFetch",
                              "Can't read field " + fieldName + "\n" + string(e.what()));
        }
    }
}

String ODBCConnection::driverDescription() const
{
    return m_connect != nullptr ? m_connect->driverDescription() : "";
}

void ODBCConnection::listDataSources(Strings& dsns)
{
    dsns.clear();
    array<SQLCHAR, MaxBufferSize> dataSource{};
    array<SQLCHAR, MaxBufferSize> description{};
    SQLSMALLINT rdsrc = 0;
    SQLSMALLINT rdesc = 0;

    SQLHENV hEnv = ODBCConnectionBase::getEnvironment().handle();
    const auto offline = hEnv == nullptr;
    if (offline)
    {
        if (!successful(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HENV, &hEnv)))
        {
            throw DatabaseException("ODBCConnection::SQLAllocHandle");
        }
        if (!successful(SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION,
                                      reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), SQL_IS_INTEGER)))
        {
            SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
            throw DatabaseException("ODBCConnection::SQLSetEnvAttr");
        }
    }

    SQLUSMALLINT direction = SQL_FETCH_FIRST;
    while (true)
    {
        const auto ret = SQLDataSources(hEnv, direction, dataSource.data(), static_cast<SQLSMALLINT>(dataSource.size()),
                                        &rdsrc, description.data(), static_cast<SQLSMALLINT>(description.size()),
                                        &rdesc);
        if (ret == SQL_NO_DATA)
        {
            break;
        }
        if (!successful(ret))
        {
            if (offline)
            {
                SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
            }
            throw DatabaseException("ODBCConnection::SQLDataSources failed");
        }
        direction = SQL_FETCH_NEXT;
        dsns.push_back(String(bit_cast<char*>(dataSource.data()), static_cast<size_t>(rdsrc)) + " (" +
            String(bit_cast<char*>(description.data()), static_cast<size_t>(rdesc)) + ")");
    }

    if (offline)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }
}

void ODBCConnection::objectList(const DatabaseObjectType objectType, Strings& objects)
{
    objects.clear();

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

        stmt = makeObjectListStatement(objectType, objectSchema, objectName, procedureType, objectSchemaLength,
                                       objectNameLength);

        while (true)
        {
            objectSchema[0] = 0;
            objectName[0] = 0;
            const auto resultCode = SQLFetch(stmt);

            if (resultCode == SQL_NO_DATA_FOUND)
            {
                break;
            }

            if (!successful(resultCode))
            {
                throw DatabaseException("SQLFetch");
            }

            // A driver may report a length larger than the bound buffer (SQL_SUCCESS_WITH_INFO).
            // Clamp it instead of writing past the end of the vector.
            objectSchema[min<SQLLEN>(objectSchemaLength, static_cast<SQLLEN>(objectSchema.size() - 1))] = 0;
            objectName[min<SQLLEN>(objectNameLength, static_cast<SQLLEN>(objectName.size() - 1))] = 0;

            if (objectType == DatabaseObjectType::FUNCTIONS && procedureType != SQL_PT_FUNCTION)
            {
                continue;
            }

            if (objectType == DatabaseObjectType::PROCEDURES && procedureType != SQL_PT_PROCEDURE)
            {
                continue;
            }

            objects.push_back(
                String(bit_cast<char*>(objectSchema.data())) + "." + String(bit_cast<char*>(objectName.data())));
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

SQLHSTMT ODBCConnection::makeObjectListStatement(const DatabaseObjectType& objectType, vector<SQLCHAR>& objectSchema,
                                                 vector<SQLCHAR>& objectName, short& procedureType,
                                                 SQLLEN& objectSchemaLength, SQLLEN& objectNameLength) const
{
    procedureType = 0;

    SQLHSTMT stmt = nullptr;

    if (SQLAllocStmt(this->handle(), &stmt) != SQL_SUCCESS)
    {
        const auto error = queryError(static_cast<SQLHSTMT>(nullptr));
        throw DatabaseException("ODBCConnection::SQLAllocStmt: " + error);
    }

    switch (objectType)
    {
    case DatabaseObjectType::TABLES:
        if (!successful(SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, Buffer("TABLE").data(), SQL_NTS)))
        {
            SQLFreeStmt(stmt, SQL_DROP);
            throw DatabaseException("SQLTables");
        }
        break;

    case DatabaseObjectType::VIEWS:
        if (!successful(SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, Buffer("VIEW").data(), SQL_NTS)))
        {
            SQLFreeStmt(stmt, SQL_DROP);
            throw DatabaseException("SQLTables");
        }
        break;

    case DatabaseObjectType::PROCEDURES:
    case DatabaseObjectType::FUNCTIONS:
        if (!successful(SQLProcedures(stmt, nullptr, 0, Buffer("").data(), SQL_NTS,
                                      Buffer("%").data(), SQL_NTS)))
        {
            SQLFreeStmt(stmt, SQL_DROP);
            throw DatabaseException("SQLProcedures");
        }
        break;

    default:
        break;
    }

    if (constexpr SQLSMALLINT schemaColumnNumber = 2;
        SQLBindCol(stmt, schemaColumnNumber, SQL_C_CHAR, objectSchema.data(),
                   static_cast<SQLLEN>(objectSchema.size() - 1), &objectSchemaLength) != SQL_SUCCESS)
    {
        SQLFreeStmt(stmt, SQL_DROP);
        throw DatabaseException("SQLBindCol");
    }

    if (constexpr SQLSMALLINT objectNameColumnNumber = 3;
        SQLBindCol(stmt, objectNameColumnNumber, SQL_C_CHAR, objectName.data(),
                   static_cast<SQLLEN>(objectName.size() - 1), &objectNameLength) != SQL_SUCCESS)
    {
        SQLFreeStmt(stmt, SQL_DROP);
        throw DatabaseException("SQLBindCol");
    }

    if (objectType == DatabaseObjectType::FUNCTIONS || objectType == DatabaseObjectType::PROCEDURES)
    {
        constexpr SQLSMALLINT procedureTypeColumn = 8;
        if (SQLBindCol(stmt, procedureTypeColumn, SQL_C_SHORT, &procedureType,
                       static_cast<SQLLEN>(sizeof(procedureType)), nullptr) != SQL_SUCCESS)
        {
            SQLFreeStmt(stmt, SQL_DROP);
            throw DatabaseException("SQLBindCol");
        }
    }

    return stmt;
}

void ODBCConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    static const RegularExpression matchRoutine("^(CREATE|REPLACE) .*(FUNCTION|PROCEDURE)", "i");
    static const RegularExpression matchDo("^\\s*DO\\s", "i");
    static const RegularExpression matchBodyStart(R"(AS\s+(\S+)\s*$)", "i");
    static const RegularExpression matchDoBodyStart(R"(^\s*DO\s+(\S+)\s*$)", "i");
    static const RegularExpression matchStatementEnd(R"(;(\s*|\s*--.*)$)");
    static const RegularExpression matchGo("^\\s*GO\\s*$", "i");
    static const RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    String delimiter;
    String statement;
    bool functionHeader = false;
    bool functionBody = false;

    for (auto row : batchSQL)
    {
        if (!functionHeader && !functionBody)
        {
            row = row.trim();
            if (row.empty() || matchCommentRow.matches(row))
            {
                continue;
            }
        }

        auto appended = false;
        if (!functionHeader && (matchRoutine.matches(row) || matchDo.matches(row)))
        {
            functionHeader = true;
            statement += row;
            statement += '\n';
            appended = true;
        }

        // A semicolon inside the dollar-quoted body of a routine or DO block is not a separator.
        if (functionBody && !delimiter.empty() && !appended && row.find(delimiter) != String::npos)
        {
            delimiter.clear();
            functionBody = false;
        }

        if (functionHeader && !functionBody)
        {
            if (const auto quotePos = row.find("$$"); quotePos != String::npos)
            {
                delimiter = "$$";
                functionHeader = false;
                functionBody = true;
                if (row.find("$$", quotePos + 2) != String::npos)
                {
                    delimiter.clear();
                    functionBody = false;
                }
            }
            else if (matchBodyStart.matches(row) || matchDoBodyStart.matches(row))
            {
                const auto matches = matchBodyStart.matches(row) ? matchBodyStart.m(row) : matchDoBodyStart.m(row);
                delimiter = matches[0].value;
                functionHeader = false;
                functionBody = true;
                if (!appended)
                {
                    statement += row;
                    statement += '\n';
                }
            }
        }

        if (!functionBody && matchGo.matches(row))
        {
            if (!trim(statement).empty())
            {
                statements.push_back(trim(statement));
            }
            statement.clear();
            continue;
        }

        if (!functionBody && matchStatementEnd.matches(row))
        {
            if (!appended)
            {
                statement += row;
            }
            statements.push_back(trim(statement));
            statement.clear();
            continue;
        }

        if (!appended)
        {
            statement += row;
            statement += '\n';
        }
    }

    if (!trim(statement).empty())
    {
        statements.push_back(trim(statement));
    }

    // Execute directly. This needs neither a shared_ptr to the connection (which Query would) nor
    // a per-statement SQLAllocStmt/SQLFreeStmt pair through Query's statement management.
    if (!active())
    {
        open();
    }

    const scoped_lock lock(*m_connect);
    for (const auto& sql : statements)
    {
        SQLHSTMT hStmt = SQL_NULL_HSTMT;
        try
        {
            if (!successful(SQLAllocStmt(handle(), &hStmt)))
            {
                throw DatabaseException("SQLAllocStmt failed: " + queryError(static_cast<SQLHSTMT>(nullptr)));
            }
            String text(sql);
            if (!successful(SQLExecDirect(hStmt, bit_cast<SQLCHAR*>(text.data()), SQL_NTS)))
            {
                throw DatabaseException(queryError(hStmt));
            }
            SQLFreeStmt(hStmt, SQL_DROP);
            hStmt = SQL_NULL_HSTMT;
        }
        catch (const Exception& e)
        {
            if (hStmt != SQL_NULL_HSTMT)
            {
                SQLFreeStmt(hStmt, SQL_DROP);
            }
            stringstream error;
            error << e.what() << ". Query: " << sql;
            if (errors == nullptr)
            {
                throw DatabaseException(error.str());
            }
            errors->push_back(error.str());
        }
    }
}

SynchronizedMap<ODBCConnection*, shared_ptr<ODBCConnection>> ODBCConnection::s_odbcConnections;

[[maybe_unused]] void* odbcCreateConnection(const char* connectionString, const size_t connectionTimeoutSeconds)
{
    const auto connection = make_shared<ODBCConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    ODBCConnection::s_odbcConnections.insert(connection.get(), connection);
    return connection.get();
}

[[maybe_unused]] void odbcDestroyConnection(void* connection)
{
    ODBCConnection::s_odbcConnections.erase(bit_cast<ODBCConnection*>(connection));
}
