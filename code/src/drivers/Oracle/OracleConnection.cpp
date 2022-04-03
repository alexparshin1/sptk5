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

#include <sptk5/cutils>
#include <sptk5/db/OracleConnection.h>
#include <sptk5/db/Transaction.h>

using namespace std;
using namespace sptk;
using namespace oracle::occi;

namespace sptk {
static void Oracle_readTimestamp(oracle::occi::ResultSet* resultSet, sptk::DatabaseField* field, unsigned int columnIndex);
static void Oracle_readDate(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);
static void Oracle_readBLOB(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);
static void Oracle_readCLOB(oracle::occi::ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex);
} // namespace sptk

OracleConnection::OracleConnection(const String& connectionString)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::ORACLE)
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
        CERR(e.what() << endl)
    }
    catch (const SQLException& e)
    {
        CERR(e.what() << endl)
    }
}

void OracleConnection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
        setInTransaction(false);
        if (newConnectionString.length())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        Statement* createLOBtable = nullptr;
        try
        {
            m_connection = shared_ptr<Connection>(m_environment.createConnection(connectionString()),
                                                  [this](Connection* ptr) {
                                                      disconnectAllQueries();
                                                      m_environment.terminateConnection(ptr);
                                                  });
            createLOBtable = m_connection->createStatement();
            createLOBtable->setSQL(
                "CREATE GLOBAL TEMPORARY TABLE sptk_lobs (sptk_clob CLOB, sptk_blob BLOB) ON COMMIT DELETE ROWS");
            createLOBtable->executeUpdate();
        }
        catch (const SQLException& e)
        {
            if (strstr(e.what(), "already used") == nullptr)
            {
                if (m_connection)
                {
                    m_connection->terminateStatement(createLOBtable);
                    m_connection.reset();
                }
                throwOracleException(string("Can't create connection: ") + e.what())
            }
        }
        m_connection->terminateStatement(createLOBtable);
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
        throwOracleException(string("Can't close connection: ") + e.what())
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
    stringstream connectionString;
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
        throwOracleException("Transaction already started.")

            setInTransaction(true);
}

void OracleConnection::driverEndTransaction(bool commit)
{
    if (!getInTransaction())
        throwOracleException("Transaction isn't started.")

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
        throwOracleException((action + " failed: ") + e.what())
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
    scoped_lock lock(m_mutex);
    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void OracleConnection::queryCloseStmt(Query* query)
{
    scoped_lock lock(m_mutex);
    auto* statement = (OracleStatement*) query->statement();
    if (statement)
    {
        statement->close();
    }
}

void OracleConnection::queryPrepare(Query* query)
{
    scoped_lock lock(m_mutex);

    auto* statement = (OracleStatement*) query->statement();
    statement->enumerateParams(query->params());
    if (query->bulkMode())
    {
        const CParamVector& enumeratedParams = statement->enumeratedParams();
        Statement* stmt = statement->stmt();
        const auto* bulkInsertQuery = dynamic_cast<OracleBulkInsertQuery*>(query);
        if (bulkInsertQuery == nullptr)
        {
            throw Exception("Not a bulk query");
        }
        const QueryColumnTypeSizeMap& columnTypeSizes = bulkInsertQuery->columnTypeSizes();
        setMaxParamSizes(enumeratedParams, stmt, columnTypeSizes);
        stmt->setMaxIterations((unsigned) bulkInsertQuery->batchSize());
    }
    querySetPrepared(query, true);
}

void OracleConnection::setMaxParamSizes(const CParamVector& enumeratedParams, Statement* stmt,
                                        const QueryColumnTypeSizeMap& columnTypeSizes)
{
    unsigned paramIndex = 1;
    for (const auto& param: enumeratedParams)
    {
        if (auto xtor = columnTypeSizes.find(upperCase(param->name()));
            xtor != columnTypeSizes.end())
        {
            if (xtor->second.length)
            {
                stmt->setMaxParamSize(paramIndex, (unsigned) xtor->second.length);
            }
            else
            {
                stmt->setMaxParamSize(paramIndex, 32);
            }
        }
        ++paramIndex;
    }
}

int OracleConnection::queryColCount(Query* query)
{
    const auto* statement = (OracleStatement*) query->statement();
    if (statement == nullptr)
    {
        throwOracleException("Query not opened")
    }
    else
    {
        return (int) statement->colCount();
    }
}

void OracleConnection::queryBindParameters(Query* query)
{
    scoped_lock lock(m_mutex);

    auto* statement = (OracleStatement*) query->statement();
    if (!statement)
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
        case Type(SQLT_NUM):
            return scale == 0 ? VariantDataType::VAR_INT : VariantDataType::VAR_FLOAT;
        case Type(SQLT_INT):
            return VariantDataType::VAR_INT;
        case Type(SQLT_UIN):
            return VariantDataType::VAR_INT64;
        case Type(SQLT_DAT):
        case Type(SQLT_DATE):
            return VariantDataType::VAR_DATE;
        case Type(SQLT_FLT):
        case Type(SQLT_BFLOAT):
        case Type(SQLT_BDOUBLE):
            return VariantDataType::VAR_FLOAT;
        case Type(SQLT_BLOB):
            return VariantDataType::VAR_BUFFER;
        case Type(SQLT_CLOB):
            return VariantDataType::VAR_TEXT;
        case Type(SQLT_TIME):
        case Type(SQLT_TIME_TZ):
        case Type(SQLT_TIMESTAMP):
        case Type(SQLT_TIMESTAMP_TZ):
            return VariantDataType::VAR_DATE_TIME;
        default:
            return VariantDataType::VAR_STRING;
    }
}

Type sptk::VariantTypeToOracleType(VariantDataType dataType)
{
    switch (dataType)
    {
        case VariantDataType::VAR_NONE:
            throwException("Data type is not defined") case VariantDataType::VAR_INT : return (Type) SQLT_INT;
        case VariantDataType::VAR_FLOAT:
            return OCCIBDOUBLE;
        case VariantDataType::VAR_STRING:
            return OCCICHAR;
        case VariantDataType::VAR_TEXT:
            return OCCICLOB;
        case VariantDataType::VAR_BUFFER:
            return OCCIBLOB;
        case VariantDataType::VAR_DATE:
            return OCCIDATE;
        case VariantDataType::VAR_DATE_TIME:
            return OCCITIMESTAMP;
        case VariantDataType::VAR_INT64:
        case VariantDataType::VAR_BOOL:
            return OCCIINT;
        default:
            throwException("Unsupported SPTK data type: " << (int) dataType)
    }
}

void OracleConnection::queryExecute(Query* query)
{
    try
    {
        auto* statement = (OracleStatement*) query->statement();
        if (!statement)
        {
            throw Exception("Query is not prepared");
        }
        if (query->bulkMode())
        {
            const auto* bulkInsertQuery = dynamic_cast<OracleBulkInsertQuery*>(query);
            if (bulkInsertQuery == nullptr)
            {
                throw Exception("Query is not COracleBulkInsertQuery");
            }
            statement->execBulk(getInTransaction(), bulkInsertQuery->lastIteration());
        }
        else
        {
            statement->execute(getInTransaction());
        }
    }
    catch (const SQLException& e)
    {
        throwOracleException(e.what())
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

    if (!query->statement())
    {
        queryAllocStmt(query);
    }

    if (!query->prepared())
    {
        queryPrepare(query);
    }

    // Bind parameters also executes a query
    queryBindParameters(query);

    auto* statement = (OracleStatement*) query->statement();

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
            scoped_lock lock(m_mutex);
            ResultSet* resultSet = statement->resultSet();
            createQueryFieldsFromMetadata(query, resultSet);
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

void OracleConnection::createQueryFieldsFromMetadata(Query* query, ResultSet* resultSet)
{
    vector<MetaData> resultSetMetaData = resultSet->getColumnListMetaData();
    unsigned columnIndex = 0;
    for (const MetaData& metaData: resultSetMetaData)
    {
        auto columnType = (Type) metaData.getInt(MetaData::ATTR_DATA_TYPE);
        int columnScale = metaData.getInt(MetaData::ATTR_SCALE);
        string columnName = metaData.getString(MetaData::ATTR_NAME);
        int columnDataSize = metaData.getInt(MetaData::ATTR_DATA_SIZE);
        if (columnName.empty())
        {
            array<char, 32> alias {};
            snprintf(alias.data(), sizeof(alias) - 1, "column_%02i", int(columnIndex + 1));
            columnName = alias.data();
        }
        if (columnType == OCCI_SQLT_LNG && columnDataSize == 0)
        {
            resultSet->setMaxColumnSize(columnIndex + 1, 16384);
        }
        VariantDataType dataType = OracleTypeToVariantType(columnType, columnScale);
        auto field = make_shared<DatabaseField>(columnName, columnIndex, columnType, dataType, columnDataSize,
                                                columnScale);
        query->fields().push_back(field);

        ++columnIndex;
    }
}

static void sptk::Oracle_readTimestamp(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex)
{
    int year = 0;
    unsigned month = 0;
    unsigned day = 0;
    unsigned hour = 0;
    unsigned min = 0;
    unsigned sec = 0;
    unsigned ms = 0;
    Timestamp timestamp = resultSet->getTimestamp(columnIndex);
    timestamp.getDate(year, month, day);
    timestamp.getTime(hour, min, sec, ms);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(hour), short(min), short(sec)));
}

static void sptk::Oracle_readDate(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex)
{
    int year = 0;
    unsigned month = 0;
    unsigned day = 0;
    unsigned hour = 0;
    unsigned min = 0;
    unsigned sec = 0;
    resultSet->getDate(columnIndex).getDate(year, month, day, hour, min, sec);
    field->setDateTime(DateTime(short(year), short(month), short(day), short(0), short(0), short(0)), true);
}

void OracleConnection::queryFetch(Query* query)
{
    if (!query->active())
        THROW_QUERY_ERROR(query, "Dataset isn't open")

    scoped_lock lock(m_mutex);

    auto* statement = (OracleStatement*) query->statement();

    try
    {
        statement->fetch();
    }
    catch (const oracle::occi::SQLException& e)
    {
        THROW_QUERY_ERROR(query, e.what())
    }

    if (statement->eof())
    {
        querySetEof(query, true);
        return;
    }

    auto fieldCount = query->fieldCount();
    if (!fieldCount)
    {
        return;
    }

    ResultSet* resultSet = statement->resultSet();
    DatabaseField* field = nullptr;
    for (unsigned fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
    {
        try
        {
            field = (DatabaseField*) &(*query)[fieldIndex];

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
            THROW_QUERY_ERROR(query, "Can't read field " << field->fieldName() << ": " << e.what())
        }
        catch (const SQLException& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " << field->fieldName() << ": " << e.what())
        }
    }
}

static void sptk::Oracle_readCLOB(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto& buffer = field->get<Buffer>();
    Clob clob = resultSet->getClob(columnIndex);
    clob.open(OCCI_LOB_READONLY);
    // Attention: clob stored as widechar
    unsigned clobChars = clob.length();
    unsigned clobBytes = clobChars * 4;
    field->checkSize(clobBytes);
    unsigned bytes = clob.read(clobChars, buffer.data(), clobBytes, 1);
    clob.close();
    field->setDataSize(bytes);
    buffer.data()[bytes] = 0;
}

static void sptk::Oracle_readBLOB(ResultSet* resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto& buffer = field->get<Buffer>();
    Blob blob = resultSet->getBlob(columnIndex);
    blob.open(OCCI_LOB_READONLY);
    unsigned bytes = blob.length();
    field->checkSize(bytes);
    blob.read(bytes, buffer.data(), bytes, 1);
    blob.close();
    field->setDataSize(bytes);
}

void OracleConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
        case DatabaseObjectType::PROCEDURES:
            objectsSQL = "SELECT object_name FROM user_procedures WHERE object_type = 'PROCEDURE'";
            break;
        case DatabaseObjectType::FUNCTIONS:
            objectsSQL = "SELECT object_name FROM user_procedures WHERE object_type = 'FUNCTION'";
            break;
        case DatabaseObjectType::TABLES:
            objectsSQL = "SELECT table_name FROM user_tables";
            break;
        case DatabaseObjectType::VIEWS:
            objectsSQL = "SELECT view_name FROM sys.all_views";
            break;
        case DatabaseObjectType::DATABASES:
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

void OracleConnection::_bulkInsert(const String& _tableName, const Strings& columnNames,
                                   const vector<VariantVector>& data)
{
    RegularExpression matchTableAndSchema(R"(^(\S+\.)?(\S+)$)");

    String schema;
    String tableName;
    auto matches = matchTableAndSchema.m(_tableName.toUpperCase());
    if (!matches[0].value.empty())
    {
        schema = matches[0].value;
        schema = schema.substr(0, schema.length() - 1);
    }
    tableName = matches[1].value;

    stringstream columnsInfoSQL;
    columnsInfoSQL << "SELECT column_name, data_type, data_length FROM ";
    if (schema.empty())
    {
        // User' database table
        columnsInfoSQL << "user_tab_columns WHERE table_name = :table_name";
    }
    else
    {
        columnsInfoSQL << "all_tab_columns WHERE owner = :schema AND table_name = :table_name";
    }

    Query tableColumnsQuery(this, columnsInfoSQL.str());
    tableColumnsQuery.param("table_name") = upperCase(tableName);
    if (!schema.empty())
    {
        tableColumnsQuery.param("schema") = schema;
    }
    tableColumnsQuery.open();
    const Field& column_name = tableColumnsQuery["column_name"];
    const Field& data_type = tableColumnsQuery["data_type"];
    const Field& data_length = tableColumnsQuery["data_length"];
    QueryColumnTypeSizeMap columnTypeSizeMap;
    while (!tableColumnsQuery.eof())
    {
        auto columnName = column_name.asString();
        auto columnType = data_type.asString();
        auto maxDataLength = (size_t) data_length.asInteger();
        QueryColumnTypeSize columnTypeSize = {};
        columnTypeSize.type = VariantDataType::VAR_STRING;
        columnTypeSize.length = 0;
        if (columnType.find("LOB") != string::npos)
        {
            constexpr size_t defaultTextSize = 65536;
            columnTypeSize.type = VariantDataType::VAR_TEXT;
            columnTypeSize.length = defaultTextSize;
        }
        else if (columnType.find("CHAR") != string::npos)
        {
            columnTypeSize.length = maxDataLength;
        }
        else if (columnType.find("TIMESTAMP") != string::npos)
        {
            columnTypeSize.type = VariantDataType::VAR_DATE_TIME;
        }
        columnTypeSizeMap[columnName] = columnTypeSize;
        tableColumnsQuery.fetch();
    }
    tableColumnsQuery.close();

    QueryColumnTypeSizeVector columnTypeSizeVector;
    for (const auto& columnName: columnNames)
    {
        auto column = columnTypeSizeMap.find(upperCase(columnName));
        if (column == columnTypeSizeMap.end())
            throwDatabaseException(
                "Column '" << columnName << "' doesn't belong to table " << tableName)
                columnTypeSizeVector.push_back(column->second);
    }

    OracleBulkInsertQuery insertQuery(this,
                                      "INSERT INTO " + tableName + "(" + columnNames.join(",") +
                                          ") VALUES (:" + columnNames.join(",:") + ")",
                                      data.size(),
                                      columnTypeSizeMap);
    for (const auto& row: data)
    {
        bulkInsertSingleRow(columnNames, columnTypeSizeVector, insertQuery, row);
    }
}

void OracleConnection::bulkInsertSingleRow(const Strings& columnNames,
                                           const QueryColumnTypeSizeVector& columnTypeSizeVector,
                                           OracleBulkInsertQuery& insertQuery, const VariantVector& row)
{
    for (size_t i = 0; i < columnNames.size(); ++i)
    {
        const auto& value = row[i];
        switch (columnTypeSizeVector[i].type)
        {
            case VariantDataType::VAR_TEXT:
                if (value.dataSize())
                {
                    insertQuery.param(i).setBuffer((const uint8_t*) value.getText(), value.dataSize(),
                                                   VariantDataType::VAR_TEXT);
                }
                else
                {
                    insertQuery.param(i).setNull(VariantDataType::VAR_TEXT);
                }
                break;
            case VariantDataType::VAR_DATE_TIME:
                insertQuery.param(i).setDateTime(value.get<DateTime>());
                break;
            default:
                insertQuery.param(i).setString(value.asString());
                break;
        }
    }
    insertQuery.execNext();
}

String OracleConnection::driverDescription() const
{
    return OracleEnvironment::clientVersion();
}

String OracleConnection::paramMark(unsigned paramIndex)
{
    array<char, 16> mark {};
    snprintf(mark.data(), sizeof(mark) - 1, ":%i", paramIndex + 1);
    return string(mark.data());
}

void OracleConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    RegularExpression matchStatementEnd("(;\\s*)$");
    RegularExpression matchRoutineStart("^CREATE (OR REPLACE )?(FUNCTION|TRIGGER)", "i");
    RegularExpression matchGo("^/\\s*$");
    RegularExpression matchShowErrors("^SHOW\\s+ERRORS", "i");
    RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    string statement;
    bool routineStarted = false;
    for (const auto& arow: sqlBatch)
    {
        String row = trim(arow);
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

void OracleConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
    notImplemented("queryColAttributes");
}

void OracleConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len)
{
    notImplemented("queryColAttributes");
}

map<OracleConnection*, shared_ptr<OracleConnection>> OracleConnection::s_oracleConnections;

void* oracle_create_connection(const char* connectionString)
{
    auto connection = make_shared<OracleConnection>(connectionString);
    OracleConnection::s_oracleConnections[connection.get()] = connection;
    return connection.get();
}

void oracle_destroy_connection(void* connection)
{
    OracleConnection::s_oracleConnections.erase((OracleConnection*) connection);
}
