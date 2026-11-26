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

#include "PostgreSQLParamValues.h"
#include "htonq.h"
#include "sptk5/db/BulkQuery.h"
#include <sptk5/cutils>
#include <sptk5/db/DatabaseField.h>
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream> // libc++

using namespace std;
using namespace sptk;

namespace sptk {
constexpr auto hoursPerDay = 24;

/// Local midnight of 2000-01-01. PostgreSQL DATE and TIMESTAMP (without time zone) are naive
/// wall-clock values, so they are anchored here and read back as that wall clock, locally.
/// Built on first use rather than at namespace scope: converting a local date needs the timezone
/// offset resolved by sputil5's static initialisation, which is not ordered against the static
/// initialisation of this driver.
const DateTime& localEpochDate()
{
    static const DateTime epochDate(2000, 1, 1);
    return epochDate;
}

/// The instant 2000-01-01T00:00:00Z. TIMESTAMPTZ is transmitted as microseconds since that
/// absolute instant, so it must be anchored independently of the client and the server session
/// time zone - anchoring it locally shifts every value by the difference between the two.
const DateTime& utcEpochDate()
{
    static const DateTime epochDate(DateTime::time_point(
        std::chrono::sys_days(std::chrono::year(2000) / std::chrono::January / 1)));
    return epochDate;
}

namespace {
/// Types whose binary representation queryFetch decodes. Text-like types are listed because their
/// binary form is the same bytes as their text form. A type that is not listed makes the whole
/// result set fall back to text: libpq selects one result format for all columns, and the server
/// can render any type as text, whereas it cannot render an unknown type as binary on our behalf.
bool supportsBinaryResult(const Oid type)
{
    if (type == 114 /* JSON */ || type == 142 /* XML */)
    {
        return true;
    }

    switch (static_cast<PostgreSQLDataType>(type))
    {
        using enum PostgreSQLDataType;
        case BOOLEAN:
        case BYTE:
        case NAME:
        case INT8:
        case INT2:
        case INT4:
        case TEXT:
        case OID:
        case FLOAT4:
        case FLOAT8:
        case CHAR_ARRAY:
        case INT2_VECTOR:
        case INT2_ARRAY:
        case INT4_ARRAY:
        case TEXT_ARRAY:
        case VARCHAR_ARRAY:
        case INT8_ARRAY:
        case FLOAT4_ARRAY:
        case FLOAT8_ARRAY:
        case CHAR:
        case VARCHAR:
        case DATE:
        case TIMESTAMP:
        case TIMESTAMP_ARRAY:
        case TIMESTAMPTZ:
        case TIMESTAMPTZ_ARRAY:
        case NUMERIC:
        case BYTEA:
            return true;

        default:
            return false;
    }
}
} // namespace

class PostgreSQLStatement
{
public:
    PostgreSQLStatement(PGconn* connect, const bool int64timestamps, const bool prepared, const DateTime& epochDate)
        : m_connect(connect)
        , m_paramValues(int64timestamps, epochDate)
    {
        if (prepared)
        {
            m_stmtName = format("S{}", nextIndex());
        }
    }

    PostgreSQLStatement(const PostgreSQLStatement&) = delete;

    ~PostgreSQLStatement()
    {
        // A statement that was cleared after a failed execution has no result left, but it is
        // still prepared on the server and its name is still allocated. Deallocating by name is
        // what actually releases the server side; keying it off stmt() leaked every one of them.
        if (!name().empty() && m_connect != nullptr && PQstatus(m_connect) == CONNECTION_OK)
        {
            const String deallocateCommand = "DEALLOCATE \"" + name() + "\"";
            PGresult*    res = PQexec(m_connect, deallocateCommand.c_str());
            if (res != nullptr)
            {
                PQclear(res);
            }
        }

        clear();
    }

    PostgreSQLStatement& operator=(const PostgreSQLStatement&) = delete;

    void clear()
    {
        if (m_stmt)
        {
            PQclear(m_stmt);
            m_stmt = nullptr;
        }
        clearRows();
        m_cols = 0;
    }

    void clearRows()
    {
        m_rows = 0;
        m_currentRow = -1;
    }

    void stmt(PGresult* result, const unsigned rows, const unsigned cols = static_cast<unsigned>(-1))
    {
        if (m_stmt)
        {
            PQclear(m_stmt);
        }
        m_stmt = result;

        m_rows = static_cast<int>(rows);

        if (cols != static_cast<unsigned>(-1))
        {
            m_cols = static_cast<int>(cols);
        }

        m_currentRow = -1;
    }

    [[nodiscard]] const PGresult* stmt() const
    {
        return m_stmt;
    }

    [[nodiscard]] const String& name() const
    {
        return m_stmtName;
    }

    void fetch()
    {
        ++m_currentRow;
    }

    [[nodiscard]] bool eof() const
    {
        return m_currentRow >= m_rows;
    }

    [[nodiscard]] unsigned currentRow() const
    {
        return static_cast<unsigned>(m_currentRow);
    }

    [[nodiscard]] unsigned colCount() const
    {
        return static_cast<unsigned>(m_cols);
    }

    /// True when every column of the result can be read from PostgreSQL's binary representation.
    /// libpq selects one format for the whole result, so a single unknown column forces the text
    /// format, which the server can render for any type.
    void binaryResults(const bool binary)
    {
        m_binaryResults = binary;
    }

    [[nodiscard]] bool binaryResults() const
    {
        return m_binaryResults;
    }

    [[nodiscard]] PostgreSQLParamValues& paramValues()
    {
        return m_paramValues;
    }

private:
    PGconn*               m_connect {nullptr};
    PGresult*             m_stmt {nullptr};
    String                m_stmtName;
    int                   m_rows {0};
    int                   m_cols {0};
    int                   m_currentRow {0};
    bool                  m_binaryResults {false};
    PostgreSQLParamValues m_paramValues;

    static unsigned nextIndex();
};

unsigned PostgreSQLStatement::nextIndex()
{
    static std::atomic<unsigned> index = 0;
    return index.fetch_add(1, std::memory_order_relaxed);
}

} // namespace sptk

PostgreSQLConnection::PostgreSQLConnection(const String& connectionString, const std::chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::POSTGRES, connectTimeout)
{
}

PostgreSQLConnection::~PostgreSQLConnection()
{
    try
    {
        if (PostgreSQLConnection::active())
        {
            if (getInTransaction())
            {
                rollbackTransaction();
            }
            close();
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

namespace {
/// Quotes a value for a libpq connection string. libpq splits the string on whitespace unless the
/// value is single-quoted, and inside a quoted value a backslash and a single quote are escaped by
/// a backslash. Bare values made of safe characters are passed through unchanged.
string csQuote(const string& value)
{
    bool needsQuotes = value.empty();
    if (!needsQuotes)
    {
        for (const char c: value)
        {
            if (isalnum(static_cast<unsigned char>(c)) == 0 && c != '_' && c != '-' && c != '.' && c != '/' && c != ':')
            {
                needsQuotes = true;
                break;
            }
        }
    }

    if (!needsQuotes)
    {
        return value;
    }

    string result;
    result.reserve(value.size() + 2);
    result.push_back('\'');
    for (const char c: value)
    {
        if (c == '\\' || c == '\'')
        {
            result.push_back('\\');
        }
        result.push_back(c);
    }
    result.push_back('\'');
    return result;
}

string csParam(const string& name, const string& value)
{
    return value.empty() ? "" : name + "=" + csQuote(value) + " ";
}
} // namespace

String PostgreSQLConnection::nativeConnectionString() const
{
    const DatabaseConnectionString& connString = connectionString();

    // A value may be given either in the URL itself or as a query parameter. The URL form wins,
    // so that a connection string written in the driver's usual shape keeps working, and a
    // parameter is the fallback for connectors that build the whole string as a query.
    const auto either = [](const String& primary, const String& fallback) {
        return primary.empty() ? fallback : primary;
    };

    String port;
    if (connString.portNumber() != 0)
    {
        port = to_string(connString.portNumber());
    }
    else
    {
        port = connString.parameter("port");
    }

    // The connect timeout, in seconds, which is what libpq wants. A value in the URL comes first,
    // since that is the one a configuration sets; otherwise the timeout the connection was created
    // with is used, which was being passed to the connector and then never applied.
    auto connectTimeoutSeconds = connString.parameter("connect_timeout");
    if (connectTimeoutSeconds.empty() && connectTimeout().count() > 0)
    {
        connectTimeoutSeconds = to_string(connectTimeout().count());
    }

    const String dbName = either(connString.databaseName(), connString.parameter("dbname"));
    const String hostName = either(connString.hostName(), connString.parameter("host"));
    const String userName = either(connString.userName(), connString.parameter("user"));
    const String password = either(connString.password(), connString.parameter("password"));

    string result =
        csParam("dbname", dbName) +
        csParam("host", hostName) +
        csParam("user", userName) +
        csParam("password", password) +
        csParam("port", port) +
        csParam("connect_timeout", connectTimeoutSeconds);

    // Every other query parameter is a libpq keyword in its own right (sslmode, options,
    // application_name, sslcert, target_session_attrs, keepalives, ...). Passing them through
    // keeps a connection from silently losing settings the configuration asked for.
    static const set<string, less<>> handledKeys{
        "dbname", "host", "user", "password", "port", "connect_timeout"};

    // A libpq keyword is an identifier. A query parameter whose name is not one cannot be a
    // keyword, and pasting it in would corrupt the whole string rather than be ignored.
    const auto isKeyword = [](const string& name) {
        return !name.empty() && all_of(name.begin(), name.end(), [](const unsigned char c) {
            return isalnum(c) != 0 || c == '_';
        });
    };

    for (const auto& [name, value]: connString.parameters())
    {
        if (!isKeyword(name) || handledKeys.contains(name))
        {
            continue;
        }
        result += name + "=" + csQuote(value) + " ";
    }

    return result;
}

void PostgreSQLConnection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
        setInTransaction(false);

        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        // active() is false here, but the handle may still exist: a dropped connection leaves
        // m_connect non-null with a bad status. Overwriting it with a fresh PQconnectdb leaked
        // the old handle, its socket and its buffers on every reconnect.
        if (m_connect != nullptr)
        {
            // Release the statements first: they hold the old PGconn and would otherwise use it
            // after PQfinish. Closing them while the handle is still alive is safe, and any query
            // that is reopening itself allocates a new statement below.
            disconnectAllQueries();
            clearPreparedStatements();
            PQfinish(m_connect);
            m_connect = nullptr;
        }

        m_connect = PQconnectdb(nativeConnectionString().c_str());

        if (PQstatus(m_connect) != CONNECTION_OK)
        {
            const String error = PQerrorMessage(m_connect);
            PQfinish(m_connect);
            m_connect = nullptr;
            throw DatabaseException(error);
        }

        if (m_timestampsFormat == TimestampFormat::UNKNOWN)
        {
            if (const char* val = PQparameterStatus(m_connect, "integer_datetimes");
                val == nullptr || upperCase(val) == "ON")
            {
                m_timestampsFormat = TimestampFormat::INT64;
            }
            else
            {
                m_timestampsFormat = TimestampFormat::DOUBLE;
            }
        }
    }
}

void PostgreSQLConnection::closeDatabase()
{
    disconnectAllQueries();
    clearPreparedStatements();
    if (m_connect)
    {
        PQfinish(m_connect);
        m_connect = nullptr;
    }
}

void PostgreSQLConnection::cacheStatement(const String& sql, const SStmtHandle& statement)
{
    // The caller holds m_mutex. Only named statements are prepared on the server; a direct one
    // has nothing to reuse and nothing to deallocate.
    if (sql.empty() || statement == nullptr)
    {
        return;
    }
    const auto* pgStatement = bit_cast<const PostgreSQLStatement*>(statement.get());
    if (pgStatement->name().empty())
    {
        return;
    }

    if (const auto existing = m_preparedStatements.find(sql); existing != m_preparedStatements.end())
    {
        m_preparedStatements.erase(existing);
        m_preparedStatementKeys.erase(find(m_preparedStatementKeys.begin(), m_preparedStatementKeys.end(), sql));
    }

    m_preparedStatements.emplace(sql, statement);
    m_preparedStatementKeys.push_back(sql);

    while (m_preparedStatementKeys.size() > maxCachedStatements)
    {
        const String oldest = m_preparedStatementKeys.front();
        m_preparedStatementKeys.pop_front();
        // Dropping the last reference destroys the statement and deallocates it on the server.
        m_preparedStatements.erase(oldest);
    }
}

void PostgreSQLConnection::clearPreparedStatements()
{
    const scoped_lock lock(m_mutex);
    m_preparedStatements.clear();
    m_preparedStatementKeys.clear();
}

DBHandle PostgreSQLConnection::handle() const
{
    return bit_cast<DBHandle>(m_connect);
}

bool PostgreSQLConnection::active() const
{
    return m_connect != nullptr && PQstatus(m_connect) == CONNECTION_OK;
}

namespace {
/// The message attached to a result belongs to the command that produced it. PQerrorMessage(conn)
/// reports the last error on the connection and may be unrelated or already cleared.
String resultError(const PGconn* conn, const PGresult* res)
{
    if (res != nullptr)
    {
        if (const char* message = PQresultErrorMessage(res); message != nullptr && *message != '\0')
        {
            return message;
        }
    }
    return PQerrorMessage(conn);
}

void checkError(const PGconn* conn, PGresult* res, const String& command)
{
    if (res == nullptr)
    {
        throw DatabaseException(command + " command failed: Out of memory");
    }

    if (const auto statusCode = PQresultStatus(res);
        statusCode != PGRES_COMMAND_OK)
    {
        const auto error = command + " command failed: " + resultError(conn, res);
        PQclear(res);
        throw DatabaseException(error);
    }
}
} // namespace

void PostgreSQLConnection::driverBeginTransaction()
{
    if (m_connect == nullptr)
    {
        open();
    }

    if (getInTransaction())
    {
        throw DatabaseException("Transaction already started.");
    }

    PGresult* res = PQexec(m_connect, "BEGIN");
    checkError(m_connect, res, "BEGIN");
    PQclear(res);

    setInTransaction(true);
}

void PostgreSQLConnection::driverEndTransaction(const bool commit)
{
    if (!getInTransaction())
    {
        throw DatabaseException("Transaction isn't started.");
    }

    const char* action = commit ? "COMMIT" : "ROLLBACK";

    PGresult* res = PQexec(m_connect, action);
    checkError(m_connect, res, action);
    PQclear(res);

    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------

String PostgreSQLConnection::queryError(const Query*) const
{
    return PQerrorMessage(m_connect);
}

// Doesn't allocate a stmt if a prepared one with the same SQL is available.
void PostgreSQLConnection::queryAllocStmt(Query* query)
{
    queryFreeStmt(query); // releases the previous statement, keeping it in the cache if prepared

    if (query->autoPrepare())
    {
        const scoped_lock lock(m_mutex);
        if (const auto cached = m_preparedStatements.find(query->sql()); cached != m_preparedStatements.end())
        {
            // The entry is handed out exclusively: two open queries must not share one statement
            // and its result buffer. It goes back into the cache when this query frees it.
            const SStmtHandle handle = cached->second;
            m_preparedStatements.erase(cached);
            m_preparedStatementKeys.erase(find(m_preparedStatementKeys.begin(), m_preparedStatementKeys.end(),
                                                query->sql()));
            querySetStmt(query, handle);
            querySetPrepared(query, true);
            bit_cast<PostgreSQLStatement*>(handle.get())->clearRows();
            return;
        }
    }

    const auto stmt = make_shared<PostgreSQLStatement>(m_connect, m_timestampsFormat == TimestampFormat::INT64,
                                                       query->autoPrepare(), localEpochDate());
    querySetStmt(query, reinterpret_pointer_cast<uint8_t>(stmt));
}

void PostgreSQLConnection::queryFreeStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    // The connection, not the Query, owns the shared statement handle. Take it before
    // querySetStmt() clears the entry.
    if (query->prepared())
    {
        cacheStatement(query->sql(), queryStatement(query));
    }

    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void PostgreSQLConnection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    if (auto* statement = bit_cast<PostgreSQLStatement*>(query->statement()))
    {
        statement->clearRows();
    }
}

void PostgreSQLConnection::queryPrepare(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());

    PostgreSQLParamValues& params = statement->paramValues();
    params.setParameters(query->params());

    const Oid* paramTypes = params.types();
    const auto paramCount = params.size();

    auto* stmt = PQprepare(m_connect, statement->name().c_str(), query->sql().c_str(), static_cast<int>(paramCount),
                           paramTypes);

    if (stmt == nullptr)
    {
        throw DatabaseException(format("PREPARE command failed: {}", PQerrorMessage(m_connect)));
    }

    checkError(m_connect, stmt, "PREPARE");

    PGresult* stmt2 = PQdescribePrepared(m_connect, statement->name().c_str());
    if (stmt2 == nullptr)
    {
        PQclear(stmt);
        throw DatabaseException(format("DESCRIBE command failed: {}", PQerrorMessage(m_connect)));
    }

    auto fieldCount = static_cast<unsigned>(PQnfields(stmt2));

    if (fieldCount != 0 && PQftype(stmt2, 0) == VOIDOID)
    {
        fieldCount = 0;
    } // VOID result considered as no result

    // The format applies to the whole result, so it is binary only if every column can be decoded
    // from its binary form. Otherwise the whole result is read as text, which the server renders
    // correctly for any type - including the ones the driver has no decoder for.
    auto binaryResults = fieldCount != 0;
    for (unsigned column = 0; binaryResults && column < fieldCount; ++column)
    {
        binaryResults = supportsBinaryResult(PQftype(stmt2, static_cast<int>(column)));
    }

    PQclear(stmt2);

    statement->stmt(stmt, 0, fieldCount);
    statement->binaryResults(binaryResults);

    querySetPrepared(query, true);
    cacheStatement(query->sql(), queryStatement(query));
}

size_t PostgreSQLConnection::queryColCount(Query* query)
{
    const auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());

    return statement->colCount();
}

void PostgreSQLConnection::queryBindParameters(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto*       statement = bit_cast<PostgreSQLStatement*>(query->statement());
    auto&       paramValues = statement->paramValues();
    // A cached statement was prepared for an earlier Query, whose parameter objects are gone.
    // Refresh the pointers before reading their values, whether or not this statement is new.
    paramValues.setParameters(query->params());
    const auto& params = paramValues.params();

    uint32_t paramNumber = 0;
    for (const auto& param: params)
    {
        paramValues.setParameterValue(paramNumber, param);
        ++paramNumber;
    }

    // Binary only when queryPrepare established that every result column has a binary decoder.
    const auto resultFormat = statement->binaryResults() ? 1 : 0;

    PGresult* stmt = PQexecPrepared(m_connect, statement->name().c_str(), static_cast<int>(paramValues.size()),
                                    bit_cast<const char* const*>(paramValues.values()),
                                    paramValues.lengths(), paramValues.formats(), resultFormat);

    const ExecStatusType statusType = PQresultStatus(stmt);

    string error;
    switch (statusType)
    {
        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            break;

        case PGRES_TUPLES_OK:
            statement->stmt(stmt, static_cast<unsigned>(PQntuples(stmt)));
            break;

        case PGRES_EMPTY_QUERY:
            statement->stmt(stmt, 0, 0);
            error = "EXECUTE command failed: EMPTY QUERY";
            break;

        default:
            statement->stmt(stmt, 0, 0);
            error = "EXECUTE command failed: ";
            error += resultError(m_connect, stmt);
            break;
    }

    if (!error.empty())
    {
        statement->clear();
        THROW_QUERY_ERROR(query, error);
    }
}

void PostgreSQLConnection::queryExecDirect(const Query* query) const
{
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());
    auto& paramValues = statement->paramValues();
    paramValues.setParameters(query->params());
    const auto& params = paramValues.params();
    uint32_t    paramNumber = 0;

    for (const auto& param: params)
    {
        paramValues.setParameterValue(paramNumber, param);
        ++paramNumber;
    }

    // A direct statement is not described first, so its column types are unknown until it runs.
    // Text is the only format that is correct for every type; binary would turn a uuid or a json
    // value into raw bytes. Prepared statements keep binary format where it is known to be safe.
    constexpr auto resultFormat = 0;
    statement->binaryResults(false);

    PGresult* stmt = PQexecParams(m_connect, query->sql().c_str(), static_cast<int>(paramValues.size()), paramValues.types(),
                                  reinterpret_cast<const char* const*>(paramValues.values()),
                                  paramValues.lengths(), paramValues.formats(), resultFormat);

    const ExecStatusType statusCode = PQresultStatus(stmt);

    string error;
    switch (statusCode)
    {
        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            break;

        case PGRES_TUPLES_OK:
            statement->stmt(stmt, static_cast<unsigned>(PQntuples(stmt)), static_cast<unsigned>(PQnfields(stmt)));
            break;

        case PGRES_EMPTY_QUERY:
            // Hand the result to the statement before the error unwinds, so that clear() releases
            // it. Without this the PGresult of a failed statement was simply leaked.
            statement->stmt(stmt, 0, 0);
            error = "EXECUTE command failed: EMPTY QUERY";
            break;

        default:
            statement->stmt(stmt, 0, 0);
            error = "EXECUTE command failed: ";
            error += resultError(m_connect, stmt);
            break;
    }

    if (!error.empty())
    {
        statement->clear();
        THROW_QUERY_ERROR(query, error);
    }
}

void PostgreSQLConnection::postgreTypeToVariantType(const PostgreSQLDataType postgreType, VariantDataType& dataType)
{
    switch (postgreType)
    {
        using enum sptk::VariantDataType;
        using enum PostgreSQLDataType;
        case BOOLEAN:
            dataType = VAR_BOOL;
            break;

        case OID:
        case INT2:
        case INT4:
            dataType = VAR_INT;
            break;

        case INT8:
            dataType = VAR_INT64;
            break;

        case NUMERIC:
        case FLOAT4:
        case FLOAT8:
            dataType = VAR_FLOAT;
            break;

        case BYTEA:
            dataType = VAR_BUFFER;
            break;

        case DATE:
            dataType = VAR_DATE;
            break;

        case TIME:
        case TIMESTAMP:
            dataType = VAR_DATE_TIME;
            break;

        default:
            dataType = VAR_STRING;
            break;
    }
}

void PostgreSQLConnection::variantTypeToPostgreType(VariantDataType dataType, PostgreSQLDataType& postgreType,
                                                    const String& paramName)
{
    switch (dataType)
    {
        using enum PostgreSQLDataType;
        using enum VariantDataType;
        case VAR_INT:
            postgreType = INT4;
            break; ///< Integer 4 bytes

        case VAR_MONEY:
        case VAR_FLOAT:
            postgreType = FLOAT8;
            break; ///< Floating-point (double)

        case VAR_STRING:
        case VAR_TEXT:
            postgreType = VARCHAR;
            break; ///< Varchar

        case VAR_BUFFER:
            postgreType = BYTEA;
            break; ///< Bytea

        case VAR_DATE:
        case VAR_DATE_TIME:
            postgreType = TIMESTAMP;
            break; ///< Timestamp

        case VAR_INT64:
            postgreType = INT8;
            break; ///< Integer 8 bytes

        case VAR_BOOL:
            postgreType = BOOLEAN;
            break; ///< Boolean

        default:
            throw DatabaseException(
                format("Unsupported parameter type {} for parameter '{}'", static_cast<int>(dataType), paramName.c_str()));
    }
}

void PostgreSQLConnection::queryOpen(Query* query)
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

    if (query->autoPrepare())
    {
        if (!query->prepared())
        {
            queryPrepare(query);
        }
        queryBindParameters(query);
    }
    else
    {
        queryExecDirect(query);
    }

    const auto* statement = bit_cast<const PostgreSQLStatement*>(query->statement());

    const auto columnCount = static_cast<short>(queryColCount(query));
    if (columnCount < 1)
    {
        return;
    }

    querySetActive(query, true);

    if (query->fieldCount() == 0)
    {
        const scoped_lock lock(m_mutex);
        // Reading the column attributes
        const PGresult* stmt = statement->stmt();
        auto&           fields = query->fields();

        for (short column = 0; column < columnCount; ++column)
        {
            String columnName = PQfname(stmt, column);

            if (columnName.empty())
            {
                columnName = format("column_{}", column + 1);
            }

            auto dataType = static_cast<PostgreSQLDataType>(PQftype(stmt, column));
            auto fieldType = VariantDataType::VAR_NONE;
            postgreTypeToVariantType(dataType, fieldType);
            auto fieldLength = PQfsize(stmt, column);
            fieldLength = fieldLength < 0 ? 0 : fieldLength;
            auto field = make_shared<DatabaseField>(columnName, static_cast<int>(dataType), fieldType, fieldLength);
            fields.push_back(field);
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

namespace {
[[nodiscard]] bool readBool(const char* data)
{
    return *data != static_cast<char>(0);
}

int16_t readInt2(const char* data)
{
    return static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(data)));
}

int32_t readInt4(const char* data)
{
    return static_cast<int32_t>(ntohl(*bit_cast<const uint32_t*>(data)));
}

int64_t readInt8(const char* data)
{
    return bit_cast<int64_t>(ntohq(*bit_cast<const uint64_t*>(data)));
}

float readFloat4(const char* data)
{
    return bit_cast<float>(ntohl(*bit_cast<const uint32_t*>(data)));
}

double readFloat8(const char* data)
{
    return bit_cast<double>(ntohq(*bit_cast<const uint64_t*>(data)));
}

DateTime readDate(const char* data, const DateTime& epochDate)
{
    // The number of days since 2000-01-01 is a 32-bit value, but multiplying it by 24 in 32 bits
    // overflows around year 245000, while PostgreSQL's DATE range goes to 5874897.
    const auto dateTime = static_cast<int32_t>(ntohl(*bit_cast<const uint32_t*>(data)));
    return epochDate + chrono::hours(static_cast<int64_t>(dateTime) * hoursPerDay);
}

DateTime readTimestamp(const char* data, const bool integerTimestamps, const DateTime& epochDate)
{
    const auto           value = ntohq(*bit_cast<const uint64_t*>(data));
    chrono::microseconds epochOffset;

    if (integerTimestamps)
    {
        // time is in usecs
        epochOffset = chrono::microseconds(value);
    }
    else
    {
        const void* ptr = &value;
        const auto  seconds = *bit_cast<const double*>(ptr);
        epochOffset = chrono::seconds(static_cast<int>(seconds));
    }
    return epochDate + epochOffset;
}

// Converts internal NUMERIC Postgresql binary to long double
MoneyData readNumericToScaledInteger(const char* numeric)
{
    const auto ndigits = static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(numeric)));
    const auto weight = static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(numeric + 2)));
    const auto sign = static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(numeric + 4)));
    auto       dscale = ntohs(*bit_cast<const uint16_t*>(numeric + 6));

    if (constexpr auto maxDscale = 16;
        dscale > maxDscale)
    {
        dscale = maxDscale;
    }

    constexpr auto digitsOffset = 8;
    numeric += digitsOffset;
    int64_t value = 0;

    constexpr auto scaleShift = 4;
    auto           scale = 0;
    if (weight < 0)
    {
        for (auto i = 0; i < -(weight + 1); ++i)
        {
            scale += scaleShift;
        }
    }

    constexpr auto digitMultiplier = 10000;
    auto           digitWeight = weight;
    for (auto i = 0; i < ndigits; ++i)
    {
        const auto digit = static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(numeric)));

        value = value * digitMultiplier + digit;
        if (digitWeight < 0)
        {
            scale += scaleShift;
        }

        --digitWeight;
        numeric += sizeof(int16_t);
    }

    while (scale < dscale - scaleShift)
    {
        value *= digitMultiplier;
        scale += scaleShift;
    }

    constexpr auto multiplier1M = 1000000;
    constexpr auto multiplier100K = 100000;
    constexpr auto multiplier10K = 10000;
    constexpr auto multiplier1K = 1000;
    constexpr auto multiplier100 = 100;
    constexpr auto multiplier10 = 10;

    constexpr auto scaleMinus6 = -6;
    constexpr auto scaleMinus5 = -5;
    constexpr auto scaleMinus4 = -4;
    constexpr auto scaleMinus3 = -3;
    constexpr auto scaleMinus2 = -2;
    constexpr auto scaleMinus1 = -1;
    constexpr auto scalePlus1 = 1;
    constexpr auto scalePlus2 = 2;
    constexpr auto scalePlus3 = 3;

    switch (scale - dscale)
    {
        case scaleMinus6:
            value *= multiplier1M;
            break;
        case scaleMinus5:
            value *= multiplier100K;
            break;
        case scaleMinus4:
            value *= multiplier10K;
            break;
        case scaleMinus3:
            value *= multiplier1K;
            break;
        case scaleMinus2:
            value *= multiplier100;
            break;
        case scaleMinus1:
            value *= multiplier10;
            break;
        case scalePlus1:
            value /= multiplier10;
            break;
        case scalePlus2:
            value /= multiplier100;
            break;
        case scalePlus3:
            value /= multiplier1K;
            break;
        default:
            break;
    }

    if (sign != 0)
    {
        value = -value;
    }

    MoneyData moneyData(value, static_cast<uint8_t>(dscale));

    return moneyData;
}

void decodeArray(char* data, DatabaseField* field, const PostgreSQLConnection::TimestampFormat timestampFormat)
{
    struct PGArrayHeader
    {
        uint32_t dimensionNumber;
        uint32_t hasNull;
        uint32_t elementType;
    };

    struct PGArrayDimension
    {
        uint32_t elementCount;
        uint32_t lowerBound;
    };

    auto* arrayHeader = bit_cast<PGArrayHeader*>(data);
    arrayHeader->dimensionNumber = ntohl(arrayHeader->dimensionNumber);
    arrayHeader->hasNull = ntohl(arrayHeader->hasNull);
    arrayHeader->elementType = ntohl(arrayHeader->elementType);
    data += sizeof(PGArrayHeader);

    auto* dimensions = bit_cast<PGArrayDimension*>(data);
    data += arrayHeader->dimensionNumber * sizeof(PGArrayDimension);

    stringstream output;
    for (size_t dim = 0; dim < arrayHeader->dimensionNumber; ++dim)
    {
        PGArrayDimension* dimension = dimensions + dim;
        dimension->elementCount = ntohl(dimension->elementCount);
        dimension->lowerBound = ntohl(dimension->lowerBound);
        output << "{";
        for (size_t element = 0; element < dimension->elementCount; ++element)
        {
            if (element != 0)
            {
                output << ",";
            }

            const auto dataSize = ntohl(*bit_cast<const int32_t*>(data));
            data += sizeof(uint32_t);

            if (dataSize <= 0)
            {
                output << "NULL";
                continue;
            }

            switch (static_cast<PostgreSQLDataType>(arrayHeader->elementType))
            {
                using enum PostgreSQLDataType;
                case INT2:
                    output << readInt2(data);
                    break;

                case INT4:
                    output << readInt4(data);
                    break;

                case INT8:
                    output << readInt8(data);
                    break;

                case FLOAT4:
                    output << readFloat4(data);
                    break;

                case FLOAT8:
                    output << readFloat8(data);
                    break;

                case TEXT:
                case CHAR:
                case VARCHAR:
                    output << string(data, dataSize);
                    break;

                case DATE:
                    output << readDate(data, localEpochDate()).dateString();
                    break;

                case TIMESTAMPTZ:
                    output << readTimestamp(data, timestampFormat == PostgreSQLConnection::TimestampFormat::INT64, utcEpochDate())
                                  .isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS);
                    break;

                case TIMESTAMP:
                    output << readTimestamp(data, timestampFormat == PostgreSQLConnection::TimestampFormat::INT64, localEpochDate())
                                  .isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS);
                    break;

                default:
                    throw DatabaseException("Unsupported array element type");
            }
            data += dataSize;
        }
        output << "}";
    }
    field->setString(output.str());
}
} // namespace

void PostgreSQLConnection::queryFetch(Query* query)
{
    if (!query->active())
    {
        THROW_QUERY_ERROR(query, "Dataset isn't open");
    }
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Statement isn't open");
    }

    statement->fetch();

    if (statement->eof())
    {
        querySetEof(query, true);
        return;
    }

    const auto fieldCount = static_cast<int>(query->fieldCount());

    if (fieldCount == 0)
    {
        return;
    }

    const PGresult*       stmt = statement->stmt();
    const auto            currentRow = static_cast<int>(statement->currentRow());
    const auto            integerTimestamps = m_timestampsFormat == TimestampFormat::INT64;
    static array<char, 1> emptyString {};

    for (auto column = 0; column < fieldCount; ++column)
    {
        auto* field = bit_cast<DatabaseField*>(&(*query)[static_cast<size_t>(column)]);
        try
        {
            const auto fieldType = static_cast<PostgreSQLDataType>(field->fieldType());
            const auto dataType = field->dataType();
            const auto dataLength = PQgetlength(stmt, currentRow, column);

            if (PQgetisnull(stmt, currentRow, column) == 1)
            {
                field->setNull(dataType);
                continue;
            }

            if (dataLength == 0)
            {
                using enum sptk::VariantDataType;
                switch (dataType)
                {
                    case VAR_STRING:
                    case VAR_TEXT:
                    case VAR_BUFFER:
                        field->setExternalBuffer(bit_cast<uint8_t*>(emptyString.data()), 0, dataType);
                        break;
                    default:
                        field->setNull(dataType);
                        break;
                }
                continue;
            }

            char* data = PQgetvalue(stmt, currentRow, column);

            if (!statement->binaryResults())
            {
                // Text is used when a column has no binary decoder, or for a direct statement whose
                // column types were not described before execution. The server renders every type
                // as text, so the value can be used as a string as-is. bytea is the exception: its
                // text form is an escaped literal, which libpq converts back to the original bytes.
                if (fieldType == PostgreSQLDataType::BYTEA)
                {
                    size_t         decodedLength = 0;
                    unsigned char* decoded = PQunescapeBytea(bit_cast<const unsigned char*>(data), &decodedLength);
                    if (decoded == nullptr)
                    {
                        throw DatabaseException("Can't decode bytea value");
                    }
                    field->setBuffer(decoded, decodedLength, VariantDataType::VAR_BUFFER);
                    PQfreemem(decoded);
                }
                else
                {
                    field->setExternalBuffer(bit_cast<uint8_t*>(data), static_cast<size_t>(dataLength),
                                             VariantDataType::VAR_STRING);
                }
                continue;
            }

            switch (fieldType)
            {
                using enum PostgreSQLDataType;
                case BOOLEAN:
                    field->setBool(readBool(data));
                    break;

                case INT2:
                    field->setInteger(readInt2(data));
                    break;

                case OID:
                case INT4:
                    field->setInteger(readInt4(data));
                    break;

                case INT8:
                    field->setInt64(readInt8(data));
                    break;

                case FLOAT4:
                    field->setFloat(readFloat4(data));
                    break;

                case FLOAT8:
                    field->setFloat(readFloat8(data));
                    break;

                case NUMERIC:
                    field->setMoney(readNumericToScaledInteger(data));
                    break;

                case BYTEA:
                    field->setExternalBuffer(bit_cast<uint8_t*>(data), static_cast<size_t>(dataLength),
                                             VariantDataType::VAR_BUFFER); // External buffer
                    break;

                case DATE:
                    field->setDateTime(readDate(data, localEpochDate()));
                    break;

                case TIMESTAMPTZ:
                    field->setDateTime(readTimestamp(data, integerTimestamps, utcEpochDate()));
                    break;
                case TIMESTAMP:
                    field->setDateTime(readTimestamp(data, integerTimestamps, localEpochDate()));
                    break;

                case CHAR_ARRAY:
                case INT2_VECTOR:
                case INT2_ARRAY:
                case INT4_ARRAY:
                case TEXT_ARRAY:
                case VARCHAR_ARRAY:
                case INT8_ARRAY:
                case FLOAT4_ARRAY:
                case FLOAT8_ARRAY:
                case TIMESTAMP_ARRAY:
                case TIMESTAMPTZ_ARRAY:
                    decodeArray(data, field, m_timestampsFormat);
                    break;

                default:
                    field->setExternalBuffer(bit_cast<uint8_t*>(data), static_cast<size_t>(dataLength),
                                             VariantDataType::VAR_STRING); // External string
                    break;
            }
        }
        catch (const Exception& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
    }
}

void PostgreSQLConnection::objectList(const DatabaseObjectType objectType, Strings& objects)
{
    const string tablesSQL("SELECT table_schema || '.' || table_name "
                           "FROM information_schema.tables "
                           "WHERE table_schema NOT IN ('information_schema','pg_catalog') ");
    string       objectsSQL;
    objects.clear();

    switch (objectType)
    {
        case DatabaseObjectType::FUNCTIONS:
            objectsSQL =
                "SELECT DISTINCT routine_schema || '.' || routine_name "
                "FROM information_schema.routines "
                "WHERE routine_schema NOT IN ('information_schema','pg_catalog') "
                "AND routine_type = 'FUNCTION'";
            break;

        case DatabaseObjectType::PROCEDURES:
            objectsSQL =
                "SELECT DISTINCT routine_schema || '.' || routine_name "
                "FROM information_schema.routines "
                "WHERE routine_schema NOT IN ('information_schema','pg_catalog') "
                "AND routine_type = 'PROCEDURE'";
            break;

        case DatabaseObjectType::VIEWS:
            objectsSQL = tablesSQL + "AND table_type = 'VIEW'";
            break;

        case DatabaseObjectType::DATABASES:
            objectsSQL =
                "SELECT datname FROM pg_database WHERE datname NOT IN ('postgres','template0','template1')";
            break;

        default:
            objectsSQL = tablesSQL + "AND table_type = 'BASE TABLE'";
            break;
    }

    auto self = shared_from_this();
    if (self == nullptr)
    {
        throw DatabaseException("PoolDatabaseConnection is not created as shared_ptr");
    }

    // A one-shot catalog query: a prepared statement would cost PREPARE + DESCRIBE + EXECUTE +
    // DEALLOCATE instead of one PQexecParams call, and would be thrown away immediately.
    Query query(self, objectsSQL, false);
    query.open();

    while (!query.eof())
    {
        objects.push_back(query[0].asString());
        query.next();
    }

    query.close();
}

String PostgreSQLConnection::driverDescription() const
{
    return "PostgreSQL";
}

void PostgreSQLConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    const Strings statements = extractStatements(batchSQL);

    auto self = shared_from_this();
    if (self == nullptr)
    {
        throw DatabaseException("PoolDatabaseConnection is not created as shared_ptr");
    }

    for (const auto& stmt: statements)
    {
        try
        {
            // Each batch statement is executed once; preparing it is four round trips for nothing.
            Query query(self, stmt, false);
            query.exec();
        }
        catch (const Exception& e)
        {
            if (errors != nullptr)
            {
                errors->push_back(e.what());
            }
            else
            {
                throw;
            }
        }
    }
}

Strings PostgreSQLConnection::extractStatements(const Strings& sqlBatch)
{
    static const RegularExpression matchFunction("^(CREATE|REPLACE) .*FUNCTION", "i");
    static const RegularExpression matchFunctionBodyStart(R"(AS\s+(\S+)\s*$)", "i");
    static const RegularExpression matchStatementEnd(R"(;(\s*|\s*--.*)$)");
    static const RegularExpression matchCommentRow(R"(^\s*--)");

    Strings statements;
    String  delimiter;
    String  statement;

    auto functionHeader = false;
    auto functionBody = false;
    for (auto row: sqlBatch)
    {
        if (!functionHeader && !functionBody)
        {
            row = row.trim();
            if (row.empty() || matchCommentRow.matches(row))
            {
                continue;
            }
        }

        if (!functionHeader && matchFunction.matches(row))
        {
            functionHeader = true;
            statement += row;
            statement += '\n';
            continue;
        }

        if (functionHeader && !functionBody && matchFunctionBodyStart.matches(row))
        {
            auto matches = matchFunctionBodyStart.m(row);
            functionBody = true;
            functionHeader = false;
            delimiter = matches[0].value;
            statement += row;
            statement += '\n';
            continue;
        }

        if (functionBody && row.find(delimiter) != string::npos)
        {
            delimiter = "";
            functionBody = false;
        }

        if (!functionBody && matchStatementEnd.matches(row))
        {
            statement += row;
            statements.push_back(statement);
            statement.clear();
            continue;
        }

        statement += row;
        statement += '\n';
    }

    if (!statement.trim().empty())
    {
        statements.push_back(statement);
    }
    return statements;
}

void PostgreSQLConnection::queryColAttributes(Query*, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void PostgreSQLConnection::queryColAttributes(Query*, int16_t, int16_t, char*, int)
{
    notImplemented("queryColAttributes");
}

SynchronizedMap<PostgreSQLConnection*, shared_ptr<PostgreSQLConnection>> PostgreSQLConnection::s_postgresqlConnections;

[[maybe_unused]] void* postgresqlCreateConnection(const char* connectionString, const size_t connectionTimeoutSeconds)
{
    const auto connection = make_shared<PostgreSQLConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    PostgreSQLConnection::s_postgresqlConnections.insert(connection.get(), connection);
    return connection.get();
}

[[maybe_unused]] void postgresqlDestroyConnection(void* connection)
{
    PostgreSQLConnection::s_postgresqlConnections.erase(bit_cast<PostgreSQLConnection*>(connection));
}
