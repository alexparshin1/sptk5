/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>
#include <sptk5/db/BulkQuery.h>
#include <sptk5/db/OracleConnection.h>

using namespace std;
using namespace sptk;
using namespace oracle::occi;

namespace {
void Oracle_readTimestamp(oracle::occi::ResultSet* resultSet, sptk::DatabaseField* field, unsigned int columnIndex);
void Oracle_readDate(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);
void Oracle_readBLOB(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);
void Oracle_readCLOB(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);
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
        CERR(e.what());
    }
}

void OracleConnection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
        setInTransaction(false);
        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        Statement* createLobTable = nullptr;
        try
        {
            m_connection = shared_ptr<Connection>(m_environment.createConnection(connectionString()),
                                                  [this](Connection* ptr)
                                                  {
                                                      disconnectAllQueries();
                                                      m_environment.terminateConnection(ptr);
                                                  });
            createLobTable = m_connection->createStatement();
            createLobTable->setSQL(
                "CREATE GLOBAL TEMPORARY TABLE sptk_lobs (sptk_clob CLOB, sptk_blob BLOB) ON COMMIT DELETE ROWS");
            createLobTable->executeUpdate();
        }
        catch (const SQLException& e)
        {
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
    try
    {
        m_connection.reset();
    }
    catch (const SQLException& e)
    {
        throw DatabaseException(string("Can't close connection: ") + e.what());
    }
}

DBHandle OracleConnection::handle() const
{
    return (DBHandle) m_connection.get();
}

bool OracleConnection::active() const
{
    return m_connection != nullptr;
}

String OracleConnection::nativeConnectionString() const
{
    // Connection string in format: host[:port][/instance]
    stringstream                    connectionString;
    const DatabaseConnectionString& connString = PoolDatabaseConnection::connectionString();
    connectionString << connString.hostName();
    if (connString.portNumber())
    {
        connectionString << ":" << connString.portNumber();
    }
    if (!connString.databaseName().empty())
    {
        connectionString << "/" << connString.databaseName();
    }
    return connectionString.str();
}

void OracleConnection::driverBeginTransaction()
{
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
        throw DatabaseException((action + " failed: ") + e.what());
    }

    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------
String OracleConnection::queryError(const Query*) const
{
    return m_lastError;
}

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void OracleConnection::queryAllocStmt(Query* query)
{
    auto stmt = make_shared<OracleStatement>(this, query->sql());
    querySetStmt(query, reinterpret_pointer_cast<uint8_t>(stmt));
}

void OracleConnection::queryFreeStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
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

void OracleConnection::setMaxParamSizes(const CParamVector& enumeratedParams, Statement* stmt,
                                        const QueryColumnTypeSizeMap& columnTypeSizes)
{
    unsigned paramIndex = 1;
    for (const auto& param: enumeratedParams)
    {
        if (auto sizesIterator = columnTypeSizes.find(upperCase(param->name()));
            sizesIterator != columnTypeSizes.end())
        {
            if (sizesIterator->second.length)
            {
                stmt->setMaxParamSize(paramIndex, (unsigned) sizesIterator->second.length);
            }
            else
            {
                const auto maxParamDataSize = 32;
                stmt->setMaxParamSize(paramIndex, maxParamDataSize);
            }
        }
        ++paramIndex;
    }
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
        statement->setParameterValues();
    }
    catch (const SQLException& e)
    {
        throw DatabaseException(e.what());
    }
}

VariantDataType sptk::OracleTypeToVariantType(Type oracleType, int scale)
{
    switch (oracleType)
    {
        using enum sptk::VariantDataType;
        case Type(SQLT_NUM):
            return scale == 0 ? VAR_INT : VAR_FLOAT;
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

    // bind parameters also executes a query
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
        const int columnDataSize = metaData.getInt(MetaData::ATTR_DATA_SIZE);
        if (columnName.empty())
        {
            columnName = format("column_{}", columnIndex + 1);
        }

        if (columnType == OCCI_SQLT_LNG && columnDataSize == 0)
        {
            const auto maxColumnSize = 16384;
            resultSet->setMaxColumnSize(columnIndex + 1, maxColumnSize);
        }

        const VariantDataType dataType = OracleTypeToVariantType(columnType, columnScale);
        auto                  field = make_shared<DatabaseField>(columnName, columnType, dataType, columnDataSize,
                                                                 columnScale);
        query->fields().push_back(field);

        ++columnIndex;
    }
}

namespace {
void Oracle_readTimestamp(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex)
{
    int             year = 0;
    unsigned        month = 0;
    unsigned        day = 0;
    unsigned        hour = 0;
    unsigned        min = 0;
    unsigned        sec = 0;
    unsigned        ms = 0;
    const Timestamp timestamp = resultSet->getTimestamp(columnIndex);
    timestamp.getDate(year, month, day);
    timestamp.getTime(hour, min, sec, ms);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(hour), short(min), short(sec)));
}

void Oracle_readDate(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex)
{
    int      year = 0;
    unsigned month = 0;
    unsigned day = 0;
    unsigned hour = 0;
    unsigned min = 0;
    unsigned sec = 0;
    resultSet->getDate(columnIndex).getDate(year, month, day, hour, min, sec);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(0), short(0), short(0)), true);
}
} // namespace

void OracleConnection::queryFetch(Query* query)
{
    if (!query->active())
    {
        THROW_QUERY_ERROR(query, "Dataset isn't open");
    }

    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<OracleStatement*>(query->statement());

    try
    {
        statement->fetch();
    }
    catch (const oracle::occi::SQLException& e)
    {
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
    for (unsigned fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
    {
        auto* field = bit_cast<DatabaseField*>(&(*query)[fieldIndex]);

        try
        {
            // Result set column index starts from 1
            auto columnIndex = fieldIndex + 1;

            if (resultSet->isNull(columnIndex))
            {
                field->setNull(VariantDataType::VAR_NONE);
                continue;
            }

            switch ((Type) field->fieldType())
            {
                case Type(SQLT_INT):
                case Type(SQLT_UIN):
                    field->setInteger(resultSet->getInt(columnIndex));
                    break;

                case Type(SQLT_NUM):
                    if (field->dataType() == VariantDataType::VAR_INT)
                    {
                        field->setInteger(resultSet->getInt(columnIndex));
                    }
                    else
                    {
                        field->setFloat(resultSet->getFloat(columnIndex));
                    }
                    break;

                case Type(SQLT_FLT):
                case Type(SQLT_BFLOAT):
                    field->setFloat(resultSet->getFloat(columnIndex));
                    break;

                case Type(SQLT_BDOUBLE):
                    field->setFloat(resultSet->getDouble(columnIndex));
                    break;

                case Type(SQLT_DAT):
                case Type(SQLT_DATE):
                    Oracle_readDate(resultSet, field, columnIndex);
                    break;

                case Type(SQLT_TIME):
                case Type(SQLT_TIME_TZ):
                case Type(SQLT_TIMESTAMP):
                case Type(SQLT_TIMESTAMP_TZ):
                    Oracle_readTimestamp(resultSet, field, columnIndex);
                    break;

                case Type(SQLT_BLOB):
                    Oracle_readBLOB(resultSet, field, columnIndex);
                    break;

                case Type(SQLT_CLOB):
                    Oracle_readCLOB(resultSet, field, columnIndex);
                    break;

                default:
                    field->setString(resultSet->getString(columnIndex));
                    break;
            }
        }
        catch (const Exception& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
        catch (const SQLException& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
    }
}

namespace {
void Oracle_readCLOB(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto& buffer = field->get<Buffer>();
    Clob  clob = resultSet->getClob(columnIndex);
    clob.open(OCCI_LOB_READONLY);
    // Attention: clob stored as wide char
    const unsigned clobChars = clob.length();
    const unsigned clobBytes = clobChars * 4;
    field->checkSize(clobBytes);
    const unsigned bytes = clob.read(clobChars, buffer.data(), clobBytes, 1);
    clob.close();
    field->setDataSize(bytes);
    buffer.data()[bytes] = 0;
}

void Oracle_readBLOB(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto& buffer = field->get<Buffer>();
    Blob  blob = resultSet->getBlob(columnIndex);
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
    Query query(this, objectsSQL);
    query.open();
    while (!query.eof())
    {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }
    query.close();
}

void OracleConnection::bulkInsert(const String& fullTableName, const Strings& columnNames,
                                  const vector<VariantVector>& data)
{
    const bool wasInTransaction = inTransaction();
    if (!wasInTransaction)
    {
        beginTransaction();
    }

    BulkQuery groupInsert(this, "gtest_temp_table", columnNames, 100);
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

String OracleConnection::paramMark(unsigned paramIndex)
{
    return format(":{}", paramIndex + 1);
}

void OracleConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    static const RegularExpression matchStatementEnd("(;\\s*)$");
    static const RegularExpression matchRoutineStart("^CREATE (OR REPLACE )?(FUNCTION|TRIGGER)", "i");
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
    for (const auto& stmt: statements)
    {
        try
        {
            Query query(this, stmt, false);
            query.exec();
        }
        catch (const Exception& e)
        {
            if (errors)
            {
                errors->push_back(e.what() + String(": ") + stmt);
            }
            else
            {
                throw;
            }
        }
        catch (const SQLException& e)
        {
            if (errors)
            {
                errors->push_back(e.what() + String(": ") + stmt);
            }
            else
            {
                throw;
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

map<OracleConnection*, shared_ptr<OracleConnection>> OracleConnection::s_oracleConnections;

[[maybe_unused]] void* oracleCreateConnection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    auto connection = make_shared<OracleConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    OracleConnection::s_oracleConnections[connection.get()] = connection;
    return connection.get();
}

[[maybe_unused]] void oracleDestroyConnection(void* connection)
{
    OracleConnection::s_oracleConnections.erase(bit_cast<OracleConnection*>(connection));
}
