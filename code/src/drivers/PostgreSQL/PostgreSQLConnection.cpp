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
#include <cstring>
#include <limits>
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
} // namespace

class PostgreSQLStatement
{
public:
    PostgreSQLStatement(PGconn* connect, const bool int64timestamps, const bool prepared, const DateTime& epochDate,
                        std::recursive_mutex* mutex, const bool* noDeallocate)
        : m_connect(connect)
        , m_mutex(mutex)
        , m_noDeallocate(noDeallocate)
        , m_int64timestamps(int64timestamps)
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
        // DEALLOCATE is a network round trip on the connection that owns this statement, so it
        // must be serialized with every other use of that PGconn.
        unique_lock<recursive_mutex> lock;
        if (m_mutex != nullptr)
        {
            lock = unique_lock<recursive_mutex>(*m_mutex);
        }

        // A statement that was cleared after a failed execution has no result left, but it is
        // still prepared on the server and its name is still allocated. Deallocating by name is
        // what actually releases the server side; keying it off stmt() leaked every one of them.
        // On close/reconnect every statement is discarded at once and the server-side names go
        // away with the connection, so there is nothing to deallocate there.
        const bool closing = m_noDeallocate != nullptr && *m_noDeallocate;
        if (!closing && !name().empty() && m_connect != nullptr && PQstatus(m_connect) == CONNECTION_OK)
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
        finishStream();
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

    /// Takes over the first row of a query sent to the server in single-row mode.
    void startStreaming(PGresult* result, const unsigned columns = static_cast<unsigned>(-1))
    {
        m_streaming = true;
        stmt(result, 1, columns);
    }

    [[nodiscard]] bool streaming() const
    {
        return m_streaming;
    }

    /// Advances to the next row of a streaming result, fetching it from the server. The caller must
    /// hold the connection mutex: PQgetResult talks to the same PGconn as every other command.
    void fetchStream()
    {
        if (m_currentRow + 1 < m_rows)
        {
            ++m_currentRow;
            return;
        }

        PQclear(m_stmt);
        m_stmt = nullptr;
        m_rows = 0;
        m_currentRow = 0;

        for (;;)
        {
            PGresult* next = PQgetResult(m_connect);
            if (next == nullptr)
            {
                m_streaming = false;
                return;
            }

            const auto status = PQresultStatus(next);
            if (status == PGRES_SINGLE_TUPLE)
            {
                m_stmt = next;
                m_rows = 1;
                return;
            }
            if (status == PGRES_TUPLES_OK || status == PGRES_COMMAND_OK || status == PGRES_EMPTY_QUERY)
            {
                PQclear(next);
                finishStream();
                return;
            }

            // The server can fail after some rows have already been delivered. libpq hides that
            // when the whole result is buffered; in single-row mode it must be reported here.
            const String error = resultError(m_connect, next);
            PQclear(next);
            finishStream();
            throw DatabaseException("EXECUTE command failed: " + error);
        }
    }

    /// Consumes any rows still pending on the connection. Needed when a query is closed early:
    /// libpq will not accept another command until the current result stream is drained.
    void finishStream()
    {
        if (!m_streaming)
        {
            return;
        }
        m_streaming = false;
        drainResults();
    }

    /// Reads results until PQgetResult reports the end of the command.
    void drainResults()
    {
        PGresult* next = nullptr;
        while ((next = PQgetResult(m_connect)) != nullptr)
        {
            PQclear(next);
        }
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

    [[nodiscard]] bool int64Timestamps() const
    {
        return m_int64timestamps;
    }

    [[nodiscard]] PostgreSQLParamValues& paramValues()
    {
        return m_paramValues;
    }

private:
    PGconn*               m_connect {nullptr};
    std::recursive_mutex* m_mutex {nullptr};
    const bool*           m_noDeallocate {nullptr};
    PGresult*             m_stmt {nullptr};
    String                m_stmtName;
    int                   m_rows {0};
    int                   m_cols {0};
    int                   m_currentRow {0};
    bool                  m_streaming {false};
    bool                  m_binaryResults {false};
    bool                  m_int64timestamps {false};
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
    const scoped_lock lock(m_mutex);
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
    const scoped_lock lock(m_mutex);
    disconnectAllQueries();
    clearPreparedStatements();
    if (m_connect)
    {
        PQfinish(m_connect);
        m_connect = nullptr;
    }
}

std::vector<SStmtHandle> PostgreSQLConnection::cacheStatement(const String& sql, const SStmtHandle& statement)
{
    // The caller holds m_mutex. Only named statements are prepared on the server; a direct one
    // has nothing to reuse and nothing to deallocate. Dropped statements are returned rather than
    // destroyed here: their destructor issues DEALLOCATE, which must not run under the lock.
    std::vector<SStmtHandle> evicted;
    if (sql.empty() || statement == nullptr)
    {
        return evicted;
    }
    const auto* pgStatement = bit_cast<const PostgreSQLStatement*>(statement.get());
    if (pgStatement->name().empty())
    {
        return evicted;
    }

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
        const auto itor = m_preparedStatements.find(oldest);
        if (itor != m_preparedStatements.end())
        {
            evicted.push_back(itor->second);
            m_preparedStatements.erase(itor);
        }
    }
    return evicted;
}

void PostgreSQLConnection::clearPreparedStatements()
{
    const scoped_lock lock(m_mutex);
    // Every statement is about to lose its connection. Suppress DEALLOCATE: the server drops all
    // prepared statements when the connection closes, and the round trips would be wasted.
    m_noDeallocate = true;
    m_preparedStatements.clear();
    m_preparedStatementKeys.clear();
    m_noDeallocate = false;
}

void PostgreSQLConnection::drainPendingStreams()
{
    // The caller holds m_mutex. A statement is owned exclusively by its query, but a query that
    // stopped reading early still owns an unfinished libpq result stream.
    for (const auto& handle: queryStatements())
    {
        if (handle == nullptr)
        {
            continue;
        }
        auto* statement = bit_cast<PostgreSQLStatement*>(handle.get());
        if (statement->streaming())
        {
            statement->finishStream();
        }
    }
}

DBHandle PostgreSQLConnection::handle() const
{
    const scoped_lock lock(m_mutex);
    return bit_cast<DBHandle>(m_connect);
}

bool PostgreSQLConnection::active() const
{
    const scoped_lock lock(m_mutex);
    return m_connect != nullptr && PQstatus(m_connect) == CONNECTION_OK;
}

namespace {
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
    const scoped_lock lock(m_mutex);
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
    const scoped_lock lock(m_mutex);
    if (!getInTransaction())
    {
        throw DatabaseException("Transaction isn't started.");
    }

    const char* action = commit ? "COMMIT" : "ROLLBACK";

    drainPendingStreams();

    PGresult* res = PQexec(m_connect, action);
    checkError(m_connect, res, action);
    PQclear(res);

    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------

String PostgreSQLConnection::queryError(const Query*) const
{
    const scoped_lock lock(m_mutex);
    return PQerrorMessage(m_connect);
}

// Doesn't allocate a stmt if a prepared one with the same SQL is available.
void PostgreSQLConnection::queryAllocStmt(Query* query)
{
    // Releases the previous statement. Evicted statements are deallocated after the lock is gone.
    queryFreeStmt(query);

    const scoped_lock lock(m_mutex);
    if (query->autoPrepare())
    {
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
                                                       query->autoPrepare(), localEpochDate(), &m_mutex,
                                                       &m_noDeallocate);
    querySetStmt(query, reinterpret_pointer_cast<uint8_t>(stmt));
}

void PostgreSQLConnection::queryFreeStmt(Query* query)
{
    std::vector<SStmtHandle> evicted;
    {
        const scoped_lock lock(m_mutex);

        // The connection, not the Query, owns the shared statement handle. Take it before
        // querySetStmt() clears the entry.
        if (const SStmtHandle handle = queryStatement(query); handle != nullptr)
        {
            if (auto* statement = bit_cast<PostgreSQLStatement*>(handle.get()); statement->streaming())
            {
                statement->finishStream();
            }
        }
        if (query->prepared())
        {
            evicted = cacheStatement(query->sql(), queryStatement(query));
        }

        querySetStmt(query, nullptr);
        querySetPrepared(query, false);
    }
    // DEALLOCATE for the evicted statements runs here, without m_mutex held.
}

void PostgreSQLConnection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    if (auto* statement = bit_cast<PostgreSQLStatement*>(query->statement()))
    {
        // A partially read stream would block every later command on this connection.
        statement->finishStream();
        statement->clearRows();
    }
}

void PostgreSQLConnection::queryPrepare(Query* query)
{
    std::vector<SStmtHandle> evicted;
    {
        const scoped_lock lock(m_mutex);

        drainPendingStreams();

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
        evicted = cacheStatement(query->sql(), queryStatement(query));
    }
    // Evicted statements issue DEALLOCATE when this vector goes out of scope, outside the lock.
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

    // Single-row mode must be selected immediately after sending the query. It keeps only one row
    // in libpq's buffers at a time instead of materializing the whole result.
    if (PQsendQueryPrepared(m_connect, statement->name().c_str(), static_cast<int>(paramValues.size()),
                            bit_cast<const char* const*>(paramValues.values()), paramValues.lengths(),
                            paramValues.formats(), resultFormat) == 0)
    {
        THROW_QUERY_ERROR(query, "EXECUTE command failed: " + String(PQerrorMessage(m_connect)));
    }
    if (PQsetSingleRowMode(m_connect) != 1)
    {
        statement->finishStream();
        THROW_QUERY_ERROR(query, "Can't enable single-row result mode");
    }
    PGresult* stmt = PQgetResult(m_connect);

    const ExecStatusType statusType = PQresultStatus(stmt);

    string error;
    switch (statusType)
    {
        case PGRES_SINGLE_TUPLE:
            statement->startStreaming(stmt);
            break;

        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            statement->drainResults();
            break;

        case PGRES_TUPLES_OK:
            statement->stmt(stmt, static_cast<unsigned>(PQntuples(stmt)));
            statement->drainResults();
            break;

        case PGRES_EMPTY_QUERY:
            statement->stmt(stmt, 0, 0);
            statement->drainResults();
            error = "EXECUTE command failed: EMPTY QUERY";
            break;

        default:
            statement->stmt(stmt, 0, 0);
            statement->drainResults();
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

    // Direct statements stream as well: send, select single-row mode, then read the first result.
    if (PQsendQueryParams(m_connect, query->sql().c_str(), static_cast<int>(paramValues.size()), paramValues.types(),
                          reinterpret_cast<const char* const*>(paramValues.values()), paramValues.lengths(),
                          paramValues.formats(), resultFormat) == 0)
    {
        THROW_QUERY_ERROR(query, "EXECUTE command failed: " + String(PQerrorMessage(m_connect)));
    }
    if (PQsetSingleRowMode(m_connect) != 1)
    {
        statement->finishStream();
        THROW_QUERY_ERROR(query, "Can't enable single-row result mode");
    }
    PGresult* stmt = PQgetResult(m_connect);

    const ExecStatusType statusCode = PQresultStatus(stmt);

    string error;
    switch (statusCode)
    {
        case PGRES_SINGLE_TUPLE:
            statement->startStreaming(stmt, static_cast<unsigned>(PQnfields(stmt)));
            break;

        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            statement->drainResults();
            break;

        case PGRES_TUPLES_OK:
            statement->stmt(stmt, static_cast<unsigned>(PQntuples(stmt)), static_cast<unsigned>(PQnfields(stmt)));
            statement->drainResults();
            break;

        case PGRES_EMPTY_QUERY:
            // Hand the result to the statement before the error unwinds, so that clear() releases
            // it. Without this the PGresult of a failed statement was simply leaked.
            statement->stmt(stmt, 0, 0);
            statement->drainResults();
            error = "EXECUTE command failed: EMPTY QUERY";
            break;

        default:
            statement->stmt(stmt, 0, 0);
            statement->drainResults();
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
            // MoneyData is an exact scaled integer. Send it as NUMERIC, not FLOAT8, or the decimal
            // digits are lost before the value ever reaches the server.
            postgreType = NUMERIC;
            break;

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
    const scoped_lock lock(m_mutex);
    if (!active())
    {
        open();
    }

    if (query->active())
    {
        return;
    }

    drainPendingStreams();

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
        // These servers send fractional seconds as a double; truncating to whole seconds lost
        // the microseconds of every timestamp.
        epochOffset = chrono::duration_cast<chrono::microseconds>(chrono::duration<double>(seconds));
    }
    return epochDate + epochOffset;
}

// Converts internal NUMERIC Postgresql binary to long double
/// Reads a network-order value without modifying the PostgreSQL result buffer.
uint32_t readUint32(const char* data)
{
    uint32_t value = 0;
    memcpy(&value, data, sizeof(value));
    return ntohl(value);
}

uint16_t readUint16(const char* data)
{
    uint16_t value = 0;
    memcpy(&value, data, sizeof(value));
    return ntohs(value);
}

MoneyData readNumericToScaledInteger(const char* data, const size_t dataLength)
{
    constexpr uint16_t numericNegative = 0x4000;
    constexpr uint16_t numericSpecial = 0xC000;
    constexpr uint16_t numericNaN = 0xC000;
    constexpr uint16_t numericPInf = 0xD000;
    constexpr uint16_t numericNInf = 0xF000;
    // MoneyData::divider() supports scales 0..15; a larger PostgreSQL dscale is truncated.
    constexpr int maxScale = 15;

    if (dataLength < 8)
    {
        throw DatabaseException("Truncated PostgreSQL NUMERIC value");
    }
    const auto ndigits = static_cast<int16_t>(readUint16(data));
    const auto weight = static_cast<int16_t>(readUint16(data + 2));
    const auto sign = readUint16(data + 4);
    const auto dscale = readUint16(data + 6);

    if (ndigits < 0 || dataLength < 8 + 2ull * static_cast<size_t>(ndigits))
    {
        throw DatabaseException("Invalid PostgreSQL NUMERIC value");
    }

    // NaN and the infinities are signalled by a special sign and a code in dscale. The old code
    // treated any non-zero sign as "negative", which turned NaN into an arbitrary money value.
    if (sign == numericSpecial)
    {
        switch (dscale)
        {
            case numericNaN:
                throw DatabaseException("PostgreSQL NUMERIC NaN cannot be represented as money");
            case numericPInf:
                throw DatabaseException("PostgreSQL NUMERIC infinity cannot be represented as money");
            case numericNInf:
                throw DatabaseException("PostgreSQL NUMERIC -infinity cannot be represented as money");
            default:
                throw DatabaseException("Unsupported PostgreSQL NUMERIC special value");
        }
    }
    if (sign != 0 && sign != numericNegative)
    {
        throw DatabaseException("Unsupported PostgreSQL NUMERIC sign");
    }

    const int targetScale = min<int>(dscale, maxScale);

    // A digit at position i represents value * 10^(4*(weight-i)). Multiplying by 10^targetScale
    // turns it into the scaled integer. Accumulate in int64 with explicit overflow checks: this
    // stays portable (no __int128) and never wraps into a wrong amount.
    int64_t     quantity = 0;
    const char* digit = data + 8;
    for (int i = 0; i < ndigits; ++i, digit += 2)
    {
        const auto value = readUint16(digit);
        if (value > 9999)
        {
            throw DatabaseException("Invalid PostgreSQL NUMERIC digit");
        }

        const int exponent = 4 * (weight - i) + targetScale;
        int64_t   part = 0;
        if (exponent >= 0)
        {
            if (exponent > 18)
            {
                throw DatabaseException("PostgreSQL NUMERIC value is out of range");
            }
            int64_t multiplier = 1;
            for (int e = 0; e < exponent; ++e)
            {
                multiplier *= 10;
            }
            if (value > numeric_limits<int64_t>::max() / multiplier)
            {
                throw DatabaseException("PostgreSQL NUMERIC value is out of range");
            }
            part = static_cast<int64_t>(value) * multiplier;
        }
        else if (exponent >= -18)
        {
            int64_t divisor = 1;
            for (int e = 0; e < -exponent; ++e)
            {
                divisor *= 10;
            }
            part = value / divisor;
        }
        // A digit further beyond the target scale contributes less than one unit.

        if (part > numeric_limits<int64_t>::max() - quantity)
        {
            throw DatabaseException("PostgreSQL NUMERIC value is out of range");
        }
        quantity += part;
    }

    if (sign == numericNegative)
    {
        quantity = -quantity;
    }

    return MoneyData(static_cast<int64_t>(quantity), static_cast<uint8_t>(targetScale));
}

void decodeArray(const char* data, const size_t dataLength, DatabaseField* field,
                 const PostgreSQLConnection::TimestampFormat timestampFormat)
{
    if (dataLength < 12)
    {
        throw DatabaseException("Truncated PostgreSQL array header");
    }

    const auto dimensionNumber = readUint32(data);
    const auto elementType = readUint32(data + 8);
    data += 12;
    auto remaining = dataLength - 12;

    // The header is: dimension count, has-null flag, element OID, then one count/lower-bound pair
    // per dimension. The old decoder wrote converted values back into the result buffer.
    if (dimensionNumber == 0)
    {
        field->setString("{}");
        return;
    }
    if (dimensionNumber > 16 || remaining < dimensionNumber * 8)
    {
        throw DatabaseException("Invalid PostgreSQL array dimensions");
    }

    vector<uint32_t> counts(dimensionNumber);
    vector<uint32_t> lowerBounds(dimensionNumber);
    for (uint32_t dim = 0; dim < dimensionNumber; ++dim)
    {
        counts[dim] = readUint32(data);
        lowerBounds[dim] = readUint32(data + 4);
        data += 8;
        remaining -= 8;
    }

    // Elements are flattened in row-major order; the total is the product of all dimension sizes.
    size_t total = 1;
    for (const auto count: counts)
    {
        if (count != 0 && total > numeric_limits<size_t>::max() / count)
        {
            throw DatabaseException("PostgreSQL array is too large");
        }
        total *= count;
    }
    if (total == 0)
    {
        field->setString("{}");
        return;
    }

    stringstream output;
    // PostgreSQL prints subscripts only when they are not the default 1-based bounds.
    if (any_of(lowerBounds.begin(), lowerBounds.end(), [](const uint32_t bound) { return bound != 1; }))
    {
        for (uint32_t dim = 0; dim < dimensionNumber; ++dim)
        {
            output << "[" << lowerBounds[dim] << ":" << (lowerBounds[dim] + counts[dim] - 1) << "]";
        }
        output << "=";
    }

    const auto readElement = [&]() {
        if (remaining < 4)
        {
            throw DatabaseException("Truncated PostgreSQL array element length");
        }
        const auto dataSize = static_cast<int32_t>(readUint32(data));
        data += 4;
        remaining -= 4;

        if (dataSize == -1)
        {
            output << "NULL";
            return;
        }
        if (dataSize < 0 || static_cast<size_t>(dataSize) > remaining)
        {
            throw DatabaseException("Invalid PostgreSQL array element length");
        }

        switch (static_cast<PostgreSQLDataType>(elementType))
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
                output << string(data, static_cast<size_t>(dataSize));
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
        remaining -= static_cast<size_t>(dataSize);
    };

    // Elements are flattened; walking the dimensions recursively emits the braces and separators
    // at the right places, including a new inner slice for every outer element.
    const auto emitDimension = [&](auto&& self, const uint32_t dim) -> void {
        output << "{";
        for (uint32_t i = 0; i < counts[dim]; ++i)
        {
            if (dim + 1 == dimensionNumber)
            {
                readElement();
            }
            else
            {
                self(self, dim + 1);
            }
            if (i + 1 < counts[dim])
            {
                output << ",";
            }
        }
        output << "}";
    };
    emitDimension(emitDimension, 0);

    field->setString(output.str());
}

} // namespace

void PostgreSQLConnection::queryFetch(Query* query)
{
    if (!query->active())
    {
        THROW_QUERY_ERROR(query, "Dataset isn't open");
    }
    // The statement is owned exclusively by this query until it is closed, so decoding its own
    // result needs no connection lock. The timestamp format is copied into the statement at
    // allocation time instead of being read from the connection here.
    auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Statement isn't open");
    }

    if (statement->streaming())
    {
        try
        {
            // PQgetResult talks to the connection, but decoding the row below does not: only the
            // fetch is serialized, not the potentially expensive field conversion.
            const scoped_lock lock(m_mutex);
            statement->fetchStream();
        }
        catch (const Exception& e)
        {
            THROW_QUERY_ERROR(query, e.what());
        }
    }
    else
    {
        statement->fetch();
    }

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
    const auto            integerTimestamps = statement->int64Timestamps();
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
                    field->setMoney(readNumericToScaledInteger(data, static_cast<size_t>(dataLength)));
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
                    decodeArray(data, static_cast<size_t>(dataLength), field,
                                statement->int64Timestamps() ? PostgreSQLConnection::TimestampFormat::INT64
                                                             : PostgreSQLConnection::TimestampFormat::DOUBLE);
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

    // A catalog query is one-shot: going through Query would need a shared_ptr to this connection
    // and would allocate a statement just to throw it away. libpq does the same in one round trip.
    const scoped_lock lock(m_mutex);
    if (!active())
    {
        open();
    }

    drainPendingStreams();

    PGresult* res = PQexec(m_connect, objectsSQL.c_str());
    if (res == nullptr)
    {
        throw DatabaseException("objectList command failed: Out of memory");
    }
    if (const auto status = PQresultStatus(res); status != PGRES_TUPLES_OK)
    {
        const String error = "objectList command failed: " + resultError(m_connect, res);
        PQclear(res);
        throw DatabaseException(error);
    }

    for (int row = 0; row < PQntuples(res); ++row)
    {
        if (PQgetisnull(res, row, 0) == 0)
        {
            objects.emplace_back(PQgetvalue(res, row, 0));
        }
    }
    PQclear(res);
}

String PostgreSQLConnection::driverDescription() const
{
    return "PostgreSQL";
}

void PostgreSQLConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    const Strings statements = extractStatements(batchSQL);

    // Each batch statement is executed once; a prepared statement would cost four round trips and
    // would also require a shared_ptr to this connection for the temporary Query.
    const scoped_lock lock(m_mutex);
    if (!active())
    {
        open();
    }

    drainPendingStreams();

    for (const auto& stmt: statements)
    {
        PGresult* res = PQexec(m_connect, stmt.c_str());
        if (res == nullptr || PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            const String error = "Batch command failed: " + resultError(m_connect, res);
            if (res != nullptr)
            {
                PQclear(res);
            }
            if (errors != nullptr)
            {
                errors->push_back(error);
            }
            else
            {
                throw DatabaseException(error);
            }
        }
        else
        {
            PQclear(res);
        }
    }
}

Strings PostgreSQLConnection::extractStatements(const Strings& sqlBatch)
{
    static const RegularExpression matchRoutine("^(CREATE|REPLACE) .*(FUNCTION|PROCEDURE)", "i");
    static const RegularExpression matchDo("^\\s*DO\\s", "i");
    static const RegularExpression matchFunctionBodyStart(R"(AS\s+(\S+)\s*$)", "i");
    static const RegularExpression matchDoBodyStart(R"(^\s*DO\s+(\S+)\s*$)", "i");
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

        auto appended = false;
        if (!functionHeader && matchRoutine.matches(row))
        {
            functionHeader = true;
            statement += row;
            statement += '\n';
            continue;
        }
        if (!functionHeader && matchDo.matches(row))
        {
            // A DO block has the same dollar-quoted body as a function, but the delimiter is on
            // the DO line itself rather than behind an AS.
            functionHeader = true;
            statement += row;
            statement += '\n';
            appended = true;
        }

        if (functionHeader && !functionBody)
        {
            if (matchFunctionBodyStart.matches(row) || matchDoBodyStart.matches(row))
            {
                const auto matches = matchFunctionBodyStart.matches(row)
                                         ? matchFunctionBodyStart.m(row)
                                         : matchDoBodyStart.m(row);
                functionBody = true;
                functionHeader = false;
                delimiter = matches[0].value;
                if (!appended)
                {
                    statement += row;
                    statement += '\n';
                }
                continue;
            }
        }

        if (functionBody && row.find(delimiter) != string::npos)
        {
            delimiter = "";
            functionBody = false;
        }

        if (!functionBody && matchStatementEnd.matches(row))
        {
            if (!appended)
            {
                statement += row;
            }
            statements.push_back(statement);
            statement.clear();
            continue;
        }

        if (!appended)
        {
            statement += row;
            statement += '\n';
        }
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
