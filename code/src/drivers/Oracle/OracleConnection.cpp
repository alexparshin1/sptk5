/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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

#include <sptk5/cutils>
#include <sptk5/db/BulkQuery.h>
#include <sptk5/db/OracleConnection.h>
#include <sstream> // libc++

using namespace std;
using namespace sptk;
using namespace oracle::occi;

namespace {
int64_t numberToInt64(const oracle::occi::Number& number, const oracle::occi::Environment* environment)
{
    return std::stoll(number.toText(environment, String(38, '9')));
}

void Oracle_readTimestamp(oracle::occi::ResultSet* resultSet, sptk::DatabaseField* field, unsigned int columnIndex,
                          std::recursive_mutex& mutex);
void Oracle_readDate(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex,
                     std::recursive_mutex& mutex);
void Oracle_readBLOB(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex,
                     std::recursive_mutex& mutex);
void Oracle_readCLOB(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex,
                     std::recursive_mutex& mutex);
} // namespace

OracleConnection::OracleConnection(const String& connectionString, chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::ORACLE, connectTimeout)
{
}

OracleConnection::~OracleConnection()
{
    try
    {
        if (getInTransaction() && OracleConnection::active())
        {
            rollbackTransaction();
        }
        disconnectAllQueries();
        close();
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
    catch (const SQLException& e)
    {
        setLastError(e.what());
        CERR(e.what());
    }
}

void OracleConnection::_openDatabase(const String& newConnectionString)
{
    const scoped_lock lock(m_mutex);
    if (!active())
    {
        clearPreparedStatements();
        setInTransaction(false);
        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        Statement* createLobTable = nullptr;
        try
        {
            // The deleter must not capture `this` or call back into this object: it can run while
            // the connection is being destroyed. The environment outlives m_connection (member
            // destruction order), so capture its handle directly.
            auto* environment = m_environment.handle();
            m_connection = shared_ptr<Connection>(m_environment.createConnection(connectionString()),
                                                  [environment](Connection* ptr)
                                                  {
                                                      environment->terminateConnection(ptr);
                                                  });
            createLobTable = m_connection->createStatement();
            createLobTable->setSQL(
                "CREATE GLOBAL TEMPORARY TABLE sptk_lobs (sptk_clob CLOB, sptk_blob BLOB) ON COMMIT DELETE ROWS");
            createLobTable->executeUpdate();
        }
        catch (const SQLException& e)
        {
            setLastError(e.what());
            if (strstr(e.what(), "already used") == nullptr)
            {
                if (m_connection)
                {
                    m_connection->terminateStatement(createLobTable);
                    m_connection.reset();
                }
                throw DatabaseException(string("Can't create connection: ") + e.what());
            }
        }
        m_connection->terminateStatement(createLobTable);
    }
}

void OracleConnection::closeDatabase()
{
    const scoped_lock lock(m_mutex);
    try
    {
        disconnectAllQueries();
        clearPreparedStatements();
        m_connection.reset();
    }
    catch (const SQLException& e)
    {
        setLastError(e.what());
        throw DatabaseException(string("Can't close connection: ") + e.what());
    }
}

DBHandle OracleConnection::handle() const
{
    const scoped_lock lock(m_mutex);
    return (DBHandle) m_connection.get();
}

bool OracleConnection::active() const
{
    const scoped_lock lock(m_mutex);
    return m_connection != nullptr;
}

String OracleConnection::nativeConnectionString() const
{
    const DatabaseConnectionString& connString = PoolDatabaseConnection::connectionString();
    stringstream                    result;
    if (!connString.userName().empty())
    {
        result << connString.userName();
        if (!connString.password().empty())
        {
            result << '/' << connString.password();
        }
        result << '@';
    }
    // Oracle EZCONNECT: user[/password]@host[:port][/service]
    result << connString.hostName();
    if (connString.portNumber())
    {
        result << ':' << connString.portNumber();
    }
    if (!connString.databaseName().empty())
    {
        result << '/' << connString.databaseName();
    }
    return result.str();
}

void OracleConnection::driverBeginTransaction()
{
    const scoped_lock lock(m_mutex);
    if (!m_connection)
    {
        open();
    }

    if (getInTransaction())
    {
        throw DatabaseException("Transaction already started.");
    }

    setInTransaction(true);
}

void OracleConnection::driverEndTransaction(bool commit)
{
    const scoped_lock lock(m_mutex);
    if (!getInTransaction())
    {
        throw DatabaseException("Transaction isn't started.");
    }

    string action;
    try
    {
        if (commit)
        {
            action = "COMMIT";
            m_connection->commit();
        }
        else
        {
            action = "ROLLBACK";
            m_connection->rollback();
        }
    }
    catch (const SQLException& e)
    {
        setLastError(e.what());
        throw DatabaseException((action + " failed: ") + e.what());
    }

    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------
void OracleConnection::setLastError(const String& error)
{
    const scoped_lock lock(m_mutex);
    m_lastError = error;
}

String OracleConnection::queryError(const Query*) const
{
    const scoped_lock lock(m_mutex);
    return m_lastError;
}

void OracleConnection::queryAllocStmt(Query* query)
{
    queryFreeStmt(query);

    const scoped_lock lock(m_mutex);
    if (query->autoPrepare())
    {
        if (const auto cached = m_preparedStatements.find(query->sql()); cached != m_preparedStatements.end())
        {
            const SStmtHandle handle = cached->second;
            m_preparedStatements.erase(cached);
            m_preparedStatementKeys.erase(find(m_preparedStatementKeys.begin(), m_preparedStatementKeys.end(),
                                                query->sql()));
            bit_cast<OracleStatement*>(handle.get())->close();
            querySetStmt(query, handle);
            querySetPrepared(query, true);
            return;
        }
    }

    auto stmt = make_shared<OracleStatement>(this, query->sql());
    querySetStmt(query, reinterpret_pointer_cast<uint8_t>(stmt));
}

void OracleConnection::queryFreeStmt(Query* query)
{
    std::vector<SStmtHandle> evicted;
    SStmtHandle              discarded;
    {
        const scoped_lock lock(m_mutex);
        const SStmtHandle  handle = queryStatement(query);
        if (query->prepared())
        {
            evicted = cacheStatement(query->sql(), handle);
        }
        else
        {
            discarded = handle;
        }
        querySetStmt(query, nullptr);
        querySetPrepared(query, false);
    }
}

std::vector<SStmtHandle> OracleConnection::cacheStatement(const String& sql, const SStmtHandle& statement)
{
    std::vector<SStmtHandle> evicted;
    if (sql.empty() || statement == nullptr)
    {
        return evicted;
    }

    bit_cast<OracleStatement*>(statement.get())->close();

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

void OracleConnection::clearPreparedStatements()
{
    const scoped_lock lock(m_mutex);
    m_preparedStatements.clear();
    m_preparedStatementKeys.clear();
}


void OracleConnection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    auto*             statement = bit_cast<OracleStatement*>(query->statement());
    if (statement)
    {
        statement->close();
    }
}

void OracleConnection::queryPrepare(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<OracleStatement*>(query->statement());
    statement->enumerateParams(query->params());
    querySetPrepared(query, true);
}

size_t OracleConnection::queryColCount(Query* query)
{
    const auto* statement = bit_cast<OracleStatement*>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Query not opened");
    }
    else
    {
        return statement->colCount();
    }
}

void OracleConnection::queryBindParameters(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<OracleStatement*>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Query not prepared");
    }

    try
    {
        // A cached statement was prepared for a previous Query whose parameter objects are gone.
        statement->enumerateParams(query->params());
        statement->setParameterValues();
    }
    catch (const SQLException& e)
    {
        setLastError(e.what());
        throw DatabaseException(e.what());
    }
}

VariantDataType sptk::OracleTypeToVariantType(Type oracleType, int scale)
{
    switch (oracleType)
    {
        using enum sptk::VariantDataType;
        case Type(SQLT_NUM):
            // NUMBER has up to 38 digits. int32 truncates integral values and double loses
            // significant digits; keep the decimal text and convert on demand.
            return scale == 0 ? VAR_INT64 : VAR_STRING;
        case Type(SQLT_INT):
            return VAR_INT;
        case Type(SQLT_UIN):
            return VAR_INT64;
        case Type(SQLT_DAT):
        case Type(SQLT_DATE):
            return VAR_DATE;
        case Type(SQLT_FLT):
        case Type(SQLT_BFLOAT):
        case Type(SQLT_BDOUBLE):
            return VAR_FLOAT;
        case Type(SQLT_BLOB):
            return VAR_BUFFER;
        case Type(SQLT_CLOB):
            return VAR_TEXT;
        case Type(SQLT_TIME):
        case Type(SQLT_TIME_TZ):
        case Type(SQLT_TIMESTAMP):
        case Type(SQLT_TIMESTAMP_TZ):
            return VAR_DATE_TIME;
        default:
            return VAR_STRING;
    }
}

Type sptk::VariantTypeToOracleType(VariantDataType dataType)
{
    switch (dataType)
    {
        using enum sptk::VariantDataType;
        case VAR_NONE:
            throw DatabaseException("Data type is not defined");
        case VAR_INT:
            return (Type) SQLT_INT;
        case VAR_FLOAT:
            return OCCIBDOUBLE;
        case VAR_STRING:
            return OCCICHAR;
        case VAR_TEXT:
            return OCCICLOB;
        case VAR_BUFFER:
            return OCCIBLOB;
        case VAR_DATE:
            return OCCIDATE;
        case VAR_DATE_TIME:
            return OCCITIMESTAMP;
        case VAR_INT64:
        case VAR_BOOL:
            return OCCIINT;
        default:
            throw DatabaseException(format("Unsupported SPTK data type: {}", (int) dataType));
    }
}

void OracleConnection::queryExecute(Query* query)
{
    try
    {
        auto* statement = bit_cast<OracleStatement*>(query->statement());
        if (statement == nullptr)
        {
            throw Exception("Query is not prepared");
        }
        statement->execute(getInTransaction());
    }
    catch (const SQLException& e)
    {
        setLastError(e.what());
        throw DatabaseException(e.what());
    }
}

void OracleConnection::queryOpen(Query* query)
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

    if (!query->prepared())
    {
        queryPrepare(query);
    }

    queryBindParameters(query);

    auto* statement = bit_cast<OracleStatement*>(query->statement());

    queryExecute(query);
    if (queryColCount(query) < 1)
    {
        statement->getOutputParameters(query->fields());
        return;
    }
    else
    {
        querySetActive(query, true);
        if (query->fieldCount() == 0)
        {
            const scoped_lock lock(m_mutex);
            ResultSet*        resultSet = statement->resultSet();
            createQueryFieldsFromMetadata(query, resultSet);
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

void OracleConnection::createQueryFieldsFromMetadata(Query* query, ResultSet* resultSet)
{
    const vector<MetaData> resultSetMetaData = resultSet->getColumnListMetaData();
    unsigned               columnIndex = 0;
    for (const MetaData& metaData: resultSetMetaData)
    {
        auto      columnType = (Type) metaData.getInt(MetaData::ATTR_DATA_TYPE);
        const int columnScale = metaData.getInt(MetaData::ATTR_SCALE);
        String    columnName(metaData.getString(MetaData::ATTR_NAME));
        int columnDataSize = metaData.getInt(MetaData::ATTR_DATA_SIZE);
        if (columnName.empty())
        {
            columnName = format("column_{}", columnIndex + 1);
        }

        if (columnType == OCCI_SQLT_LNG && columnDataSize == 0)
        {
            const auto maxColumnSize = 16384;
            resultSet->setMaxColumnSize(columnIndex + 1, maxColumnSize);
            // The field must be created with the same size as the result set, or it will allocate
            // the default 256 bytes and truncate LONG values.
            columnDataSize = maxColumnSize;
        }

        const VariantDataType dataType = OracleTypeToVariantType(columnType, columnScale);
        auto                  field = make_shared<DatabaseField>(columnName, columnType, dataType, columnDataSize,
                                                                 columnScale);
        query->fields().push_back(field);

        ++columnIndex;
    }
}

namespace {
void Oracle_readTimestamp(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex, std::recursive_mutex& mutex)
{
    int             year = 0;
    unsigned        month = 0;
    unsigned        day = 0;
    unsigned        hour = 0;
    unsigned        min = 0;
    unsigned        sec = 0;
    unsigned        ms = 0;
    Timestamp timestamp;
    {
        const scoped_lock lock(mutex);
        timestamp = resultSet->getTimestamp(columnIndex);
    }
    timestamp.getDate(year, month, day);
    timestamp.getTime(hour, min, sec, ms);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(hour), short(min), short(sec),
                                static_cast<short>(ms / 1000000)));
}

void Oracle_readDate(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex, std::recursive_mutex& mutex)
{
    int      year = 0;
    unsigned month = 0;
    unsigned day = 0;
    unsigned hour = 0;
    unsigned min = 0;
    unsigned sec = 0;
    Date date;
    {
        const scoped_lock lock(mutex);
        date = resultSet->getDate(columnIndex);
    }
    date.getDate(year, month, day, hour, min, sec);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(0), short(0), short(0)), true);
}
} // namespace

void OracleConnection::queryFetch(Query* query)
{
    if (!query->active())
    {
        THROW_QUERY_ERROR(query, "Dataset isn't open");
    }

    auto* statement = bit_cast<OracleStatement*>(query->statement());
    if (statement == nullptr)
    {
        THROW_QUERY_ERROR(query, "Statement isn't open");
    }

    try
    {
        const scoped_lock lock(m_mutex);
        statement->fetch();
    }
    catch (const oracle::occi::SQLException& e)
    {
        setLastError(e.what());
        THROW_QUERY_ERROR(query, e.what());
    }

    if (statement->eof())
    {
        querySetEof(query, true);
        return;
    }

    auto fieldCount = query->fieldCount();
    if (fieldCount == 0)
    {
        return;
    }

    ResultSet* resultSet = statement->resultSet();

    // OCCI calls need the connection lock; converting the values into fields does not.
    const auto locked = [&](auto&& fn) {
        const scoped_lock lock(m_mutex);
        return fn();
    };

    for (unsigned fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
    {
        auto* field = bit_cast<DatabaseField*>(&(*query)[fieldIndex]);

        try
        {
            // Result set column index starts from 1
            const auto columnIndex = static_cast<unsigned>(fieldIndex) + 1;

            if (locked([&] { return resultSet->isNull(columnIndex); }))
            {
                field->setNull(field->dataType());
                continue;
            }

            switch ((Type) field->fieldType())
            {
                case Type(SQLT_INT):
                    field->setInteger(locked([&] { return resultSet->getInt(columnIndex); }));
                    break;

                case Type(SQLT_UIN):
                    field->setInt64(numberToInt64(locked([&] { return resultSet->getNumber(columnIndex); }),
                                                  environment()));
                    break;

                case Type(SQLT_NUM):
                    if (field->dataType() == VariantDataType::VAR_INT64)
                    {
                        field->setInt64(numberToInt64(locked([&] { return resultSet->getNumber(columnIndex); }),
                                                      environment()));
                    }
                    else
                    {
                        field->setString(locked([&] { return resultSet->getString(columnIndex); }));
                    }
                    break;

                case Type(SQLT_FLT):
                case Type(SQLT_BFLOAT):
                    field->setFloat(locked([&] { return resultSet->getFloat(columnIndex); }));
                    break;

                case Type(SQLT_BDOUBLE):
                    field->setFloat(locked([&] { return resultSet->getDouble(columnIndex); }));
                    break;

                case Type(SQLT_DAT):
                case Type(SQLT_DATE):
                    Oracle_readDate(resultSet, field, columnIndex, m_mutex);
                    break;

                case Type(SQLT_TIME):
                case Type(SQLT_TIME_TZ):
                case Type(SQLT_TIMESTAMP):
                case Type(SQLT_TIMESTAMP_TZ):
                    Oracle_readTimestamp(resultSet, field, columnIndex, m_mutex);
                    break;

                case Type(SQLT_BLOB):
                    Oracle_readBLOB(resultSet, field, columnIndex, m_mutex);
                    break;

                case Type(SQLT_CLOB):
                    Oracle_readCLOB(resultSet, field, columnIndex, m_mutex);
                    break;

                default:
                    field->setString(locked([&] { return resultSet->getString(columnIndex); }));
                    break;
            }
        }
        catch (const Exception& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
        catch (const SQLException& e)
        {
            setLastError(e.what());
            THROW_QUERY_ERROR(query, "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
    }
}

namespace {
void Oracle_readCLOB(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex, std::recursive_mutex& mutex)
{
    auto& buffer = field->get<Buffer>();
    Clob  clob;
    {
        const scoped_lock lock(mutex);
        clob = resultSet->getClob(columnIndex);
    }
    clob.open(OCCI_LOB_READONLY);
    // Attention: clob stored as wide char
    const unsigned clobChars = clob.length();
    const unsigned clobBytes = clobChars * 4;
    field->checkSize(clobBytes + 1);
    const unsigned bytes = clob.read(clobChars, buffer.data(), clobBytes, 1);
    clob.close();
    // OCCI reports the number of characters read, which may be smaller than the byte count for
    // multibyte character sets. Never write the terminator past the allocated buffer.
    const unsigned dataSize = min(bytes, clobBytes);
    field->setDataSize(dataSize);
    buffer.data()[dataSize] = 0;
}

void Oracle_readBLOB(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex, std::recursive_mutex& mutex)
{
    auto& buffer = field->get<Buffer>();
    Blob  blob;
    {
        const scoped_lock lock(mutex);
        blob = resultSet->getBlob(columnIndex);
    }
    blob.open(OCCI_LOB_READONLY);
    const unsigned bytes = blob.length();
    field->checkSize(bytes);
    blob.read(bytes, buffer.data(), bytes, 1);
    blob.close();
    field->setDataSize(bytes);
}
} // namespace

void OracleConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
        using enum sptk::DatabaseObjectType;
        case PROCEDURES:
            objectsSQL = "SELECT object_name FROM user_procedures WHERE object_type = 'PROCEDURE'";
            break;
        case FUNCTIONS:
            objectsSQL = "SELECT object_name FROM user_procedures WHERE object_type = 'FUNCTION'";
            break;
        case TABLES:
            objectsSQL = "SELECT table_name FROM user_tables";
            break;
        case VIEWS:
            objectsSQL = "SELECT view_name FROM sys.all_views";
            break;
        case DATABASES:
            objectsSQL = "SELECT username FROM all_users";
            break;
        default:
            throw Exception("Not implemented yet");
    }
    // A one-shot catalog query: prepare/execute/describe through Query is wasted work here, and a
    // raw OCCI statement does not need a shared_ptr to this connection.
    const scoped_lock lock(m_mutex);
    if (!active())
    {
        open();
    }

    Statement* stmt = m_connection->createStatement(objectsSQL);
    try
    {
        ResultSet* resultSet = stmt->executeQuery();
        while (resultSet->next() != ResultSet::END_OF_FETCH)
        {
            objects.push_back(resultSet->getString(1));
        }
        stmt->closeResultSet(resultSet);
    }
    catch (...)
    {
        m_connection->terminateStatement(stmt);
        throw;
    }
    m_connection->terminateStatement(stmt);
}

void OracleConnection::bulkInsert(const String& fullTableName, const Strings& columnNames,
                                  vector<VariantVector>& data, const size_t groupSize)
{
    const bool wasInTransaction = inTransaction();
    if (!wasInTransaction)
    {
        beginTransaction();
    }

    BulkQuery groupInsert(shared_from_this(), fullTableName, "", columnNames, groupSize);
    groupInsert.insertRows(data);

    if (!wasInTransaction)
    {
        commitTransaction();
    }
}

String OracleConnection::driverDescription() const
{
    return OracleEnvironment::clientVersion();
}

std::string OracleConnection::paramMark(unsigned paramIndex)
{
    return ":" + to_string(paramIndex + 1);
}

void OracleConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    static const RegularExpression matchStatementEnd("(;\\s*)$");
    static const RegularExpression matchRoutineStart(
        "^(CREATE|REPLACE)( OR REPLACE)? (FUNCTION|PROCEDURE|PACKAGE|TYPE|TRIGGER)", "i");
    static const RegularExpression matchGo("^/\\s*$");
    static const RegularExpression matchShowErrors("^SHOW\\s+ERRORS", "i");
    static const RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    string  statement;
    bool    routineStarted = false;
    for (const auto& aRow: batchSQL)
    {
        String row = trim(aRow);
        if (row.empty() || matchCommentRow.matches(row))
        {
            continue;
        }

        if (!routineStarted)
        {
            row = trim(row);
            if (row.empty())
            {
                continue;
            }
            if (matchShowErrors.matches(row))
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

    executeMultipleStatements(statements, errors);
}

void OracleConnection::executeMultipleStatements(const Strings& statements, Strings* errors)
{
    const scoped_lock lock(m_mutex);
    if (!active())
    {
        open();
    }

    for (const auto& sql: statements)
    {
        try
        {
            Statement* stmt = m_connection->createStatement(sql);
            stmt->executeUpdate();
            m_connection->terminateStatement(stmt);
        }
        catch (const Exception& e)
        {
            if (errors)
            {
                errors->push_back(e.what() + String(": ") + sql);
            }
            else
            {
                throw;
            }
        }
        catch (const SQLException& e)
        {
            setLastError(e.what());
            if (errors)
            {
                errors->push_back(String(e.what()) + ": " + sql);
            }
            else
            {
                throw DatabaseException(string(e.what()) + ": " + sql);
            }
        }
    }
}

void OracleConnection::queryColAttributes(Query*, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void OracleConnection::queryColAttributes(Query*, int16_t, int16_t, char*, int)
{
    notImplemented("queryColAttributes");
}

SynchronizedMap<OracleConnection*, shared_ptr<OracleConnection>> OracleConnection::s_oracleConnections;

[[maybe_unused]] void* oracleCreateConnection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    auto connection = make_shared<OracleConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    OracleConnection::s_oracleConnections.insert(connection.get(), connection);
    return connection.get();
}

[[maybe_unused]] void oracleDestroyConnection(void* connection)
{
    OracleConnection::s_oracleConnections.erase(bit_cast<OracleConnection*>(connection));
}
