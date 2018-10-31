/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       COracleConnection.cpp - description                    ║
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

#include <sptk5/db/OracleConnection.h>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/Query.h>
#include "OracleBulkInsertQuery.h"

using namespace std;
using namespace sptk;
using namespace oracle::occi;

OracleConnection::OracleConnection(const String& connectionString) :
    PoolDatabaseConnection(connectionString),
    m_connection(nullptr)
{
    m_connType = DCT_ORACLE;
}

OracleConnection::~OracleConnection()
{
    try {
        if (m_inTransaction && active())
            rollbackTransaction();
        close();
        while (!m_queryList.empty()) {
            try {
                auto query = (Query *) m_queryList[0];
                query->disconnect();
            } catch (...) {
            }
        }
        m_queryList.clear();
    } catch (...) {
    }
}

void OracleConnection::_openDatabase(const String& newConnectionString)
{
    if (!active()) {
        m_inTransaction = false;
        if (newConnectionString.length())
            m_connString = DatabaseConnectionString(newConnectionString);

        Statement* createLOBtable = nullptr;
        try {
            m_connection = m_environment.createConnection(m_connString);
            createLOBtable = m_connection->createStatement();
            createLOBtable->setSQL("CREATE GLOBAL TEMPORARY TABLE sptk_lobs (sptk_clob CLOB, sptk_blob BLOB) ON COMMIT DELETE ROWS");
            createLOBtable->executeUpdate();
        }
        catch (SQLException& e) {
            if (strstr(e.what(),"already used") == nullptr) {
                if (m_connection) {
                    m_connection->terminateStatement(createLOBtable);
                    m_environment.terminateConnection(m_connection);
                    m_connection = nullptr;
                }
                throwOracleException(string("Can't create connection: ") + e.what());
            }
        }
        m_connection->terminateStatement(createLOBtable);
    }
}

void OracleConnection::closeDatabase()
{
    for (auto query: m_queryList) {
        try {
            queryFreeStmt(query);
        } catch (...) {
        }
    }

    try {
        m_environment.terminateConnection(m_connection);
        m_connection = nullptr;
    }
    catch (SQLException& e) {
        throwOracleException(string("Can't close connection: ") + e.what());
    }
}

void* OracleConnection::handle() const
{
    return m_connection;
}

bool OracleConnection::active() const
{
    return m_connection != nullptr;
}

String OracleConnection::nativeConnectionString() const
{
    // Connection string in format: host[:port][/instance]
    stringstream connectionString;
    connectionString << m_connString.hostName();
    if (m_connString.portNumber())
        connectionString << ":" << m_connString.portNumber();
    if (!m_connString.databaseName().empty())
        connectionString << "/" << m_connString.databaseName();
    return connectionString.str();
}

void OracleConnection::driverBeginTransaction()
{
    if (!m_connection)
        open();

    if (m_inTransaction)
        throwOracleException("Transaction already started.");

    m_inTransaction = true;
}

void OracleConnection::driverEndTransaction(bool commit)
{
    if (!m_inTransaction)
        throwOracleException("Transaction isn't started.");

    string action;
    try {
        if (commit) {
            action = "COMMIT";
            m_connection->commit();
        } else {
            action = "ROLLBACK";
            m_connection->rollback();
        }
    }
    catch (SQLException& e) {
        throwOracleException((action + " failed: ") + e.what());
    }

    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------
String OracleConnection::queryError(const Query *) const
{
    return m_lastError;
}

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void OracleConnection::queryAllocStmt(Query *query)
{
    queryFreeStmt(query);
    querySetStmt(query, new OracleStatement(this, query->sql()));
}

void OracleConnection::queryFreeStmt(Query *query)
{
    lock_guard<mutex> lock(m_mutex);
    auto statement = (OracleStatement*) query->statement();
    if (statement) {
        delete statement;
        querySetStmt(query, nullptr);
        querySetPrepared(query, false);
    }
}

void OracleConnection::queryCloseStmt(Query *query)
{
    lock_guard<mutex> lock(m_mutex);
    auto statement = (OracleStatement*) query->statement();
    if (statement)
        statement->close();
}

void OracleConnection::queryPrepare(Query *query)
{
    lock_guard<mutex> lock(m_mutex);

    auto statement = (OracleStatement*) query->statement();
    if (!statement)
        statement = new OracleStatement(this, query->sql());
    statement->enumerateParams(query->params());
    if (query->bulkMode()) {
        CParamVector& enumeratedParams = statement->enumeratedParams();
        unsigned paramIndex = 1;
        Statement* stmt = statement->stmt();
        auto bulkInsertQuery = dynamic_cast<OracleBulkInsertQuery*>(query);
        if (bulkInsertQuery == nullptr)
            throw Exception("Not a bulk query");
        const QueryColumnTypeSizeMap& columnTypeSizes = bulkInsertQuery->columnTypeSizes();
        for (auto itor = enumeratedParams.begin(); itor != enumeratedParams.end(); ++itor, paramIndex++) {
            QueryParameter* param = *itor;
            auto xtor = columnTypeSizes.find(upperCase(param->name()));
            if (xtor != columnTypeSizes.end()) {
                if (xtor->second.length)
                    stmt->setMaxParamSize(paramIndex, (unsigned) xtor->second.length);
                else
                    stmt->setMaxParamSize(paramIndex, 32);
            }
        }
        stmt->setMaxIterations((unsigned) bulkInsertQuery->batchSize());
    }
    querySetStmt(query, statement);
    querySetPrepared(query, true);
}

void OracleConnection::queryUnprepare(Query *query)
{
    queryFreeStmt(query);
}

int OracleConnection::queryColCount(Query *query)
{
    auto statement = (OracleStatement*) query->statement();
    if (statement == nullptr) {
        throwOracleException("Query not opened");
    }
    else
        return (int) statement->colCount();
}

void OracleConnection::queryBindParameters(Query *query)
{
    lock_guard<mutex> lock(m_mutex);

    auto statement = (OracleStatement*) query->statement();
    if (!statement)
        throwDatabaseException("Query not prepared");
    try {
        statement->setParameterValues();
    }
    catch (const SQLException& e) {
        throwOracleException(e.what());
    }
}

VariantType OracleConnection::OracleTypeToVariantType(Type oracleType, int scale)
{
    switch (oracleType)
    {
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

Type OracleConnection::VariantTypeToOracleType(VariantType dataType)
{
    switch (dataType)
    {
        case VAR_NONE:
            throwException("Data type is not defined");
        case VAR_INT:
            return (Type) SQLT_INT;
        case VAR_FLOAT:
            return (Type) OCCIBDOUBLE;
        case VAR_STRING:
            return (Type) OCCICHAR; //SQLT_CHR;
        case VAR_TEXT:
            return (Type) OCCICLOB;
        case VAR_BUFFER:
            return (Type) OCCIBLOB;
        case VAR_DATE:
            return (Type) OCCIDATE;
        case VAR_DATE_TIME:
            return (Type) OCCITIMESTAMP;
        case VAR_INT64:
            return (Type) OCCIINT;
        case VAR_BOOL:
            return (Type) OCCIINT;
        default:
            throwException("Unsupported SPTK data type: " << dataType);
    }
}

void OracleConnection::queryExecute(Query *query)
{
    try {
        auto statement = (OracleStatement*) query->statement();
        if (!statement)
            throw Exception("Query is not prepared");
        if (query->bulkMode()) {
            auto bulkInsertQuery = dynamic_cast<OracleBulkInsertQuery*>(query);
            if (bulkInsertQuery == nullptr)
                throw Exception("Query is not COracleBulkInsertQuery");
            statement->execBulk(m_inTransaction, bulkInsertQuery->lastIteration());
        }
        else
            statement->execute(m_inTransaction);
    }
    catch (const SQLException& e) {
        throwOracleException(e.what());
    }
}

void OracleConnection::queryOpen(Query *query)
{
    if (!active())
        open();

    if (query->active())
        return;

    if (!query->statement())
        queryAllocStmt(query);

    if (!query->prepared())
        queryPrepare(query);

    // Bind parameters also executes a query
    queryBindParameters(query);

    auto statement = (OracleStatement*) query->statement();

    queryExecute(query);
    int count = queryColCount(query);
    if (count < 1) {
        //queryCloseStmt(query);
        statement->getOutputParameters(query->fields());
        return;
    } else {
        querySetActive(query, true);
        if (query->fieldCount() == 0) {
            lock_guard<mutex> lock(m_mutex);

            ResultSet* resultSet = statement->resultSet();
            vector<MetaData> resultSetMetaData = resultSet->getColumnListMetaData();
            auto itor = resultSetMetaData.begin();
            auto iend = resultSetMetaData.end();
            unsigned columnIndex = 0;
            for (; itor != iend; ++itor, columnIndex++) {
                MetaData& metaData = *itor;
                auto columnType = (Type) metaData.getInt(MetaData::ATTR_DATA_TYPE);
                int columnScale = metaData.getInt(MetaData::ATTR_SCALE);
                string columnName = metaData.getString(MetaData::ATTR_NAME);
                int columnDataSize = metaData.getInt(MetaData::ATTR_DATA_SIZE);
                if (columnName.empty()) {
                    char alias[32];
                    sprintf(alias, "column_%02i", int(columnIndex + 1));
                    columnName = alias;
                }
                if (columnType == Type::OCCI_SQLT_LNG && columnDataSize == 0)
                    resultSet->setMaxColumnSize(columnIndex + 1, 16384);
                VariantType dataType = OracleTypeToVariantType(columnType, columnScale);
                DatabaseField* field = new DatabaseField(columnName, columnIndex, columnType, dataType, columnDataSize, columnScale);
                query->fields().push_back(field);
            }
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

void OracleConnection::queryFetch(Query *query)
{
    if (!query->active())
        THROW_QUERY_ERROR(query, "Dataset isn't open");

    lock_guard<mutex> lock(m_mutex);

    auto statement = (OracleStatement*) query->statement();

    statement->fetch();
    if (statement->eof()) {
        querySetEof(query, true);
        return;
    }

    uint32_t fieldCount = query->fieldCount();
    if (!fieldCount)
        return;

    ResultSet* resultSet = statement->resultSet();
    DatabaseField* field = nullptr;
    for (uint32_t fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {
        try {
            field = (DatabaseField*) &(*query)[fieldIndex];

            // Result set column index starts from 1
            unsigned columnIndex = fieldIndex + 1;

            if (resultSet->isNull(columnIndex)) {
                field->setNull(VAR_NONE);
                continue;
            }

            int         year;
            unsigned    month, day, hour, min, sec;

            switch ((Type)field->fieldType())
            {
                case Type(SQLT_INT):
                case Type(SQLT_UIN):
                    field->setInteger(resultSet->getInt(columnIndex));
                    break;

                case Type(SQLT_NUM):
                    if (field->dataType() == VAR_INT)
                        field->setInteger(resultSet->getInt(columnIndex));
                    else
                        field->setFloat(resultSet->getFloat(columnIndex));
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
                    {
                        resultSet->getDate(columnIndex).getDate(year, month, day, hour, min, sec);
                        field->setDate(DateTime(short(year), short(month), short(day), short(0), short(0), short(0)));
                    }
                    break;

                case Type(SQLT_TIME):
                case Type(SQLT_TIME_TZ):
                case Type(SQLT_TIMESTAMP):
                case Type(SQLT_TIMESTAMP_TZ):
                    {
                        Timestamp timestamp = resultSet->getTimestamp(columnIndex);
                        unsigned ms;
                        timestamp.getDate(year, month, day);
                        timestamp.getTime(hour, min, sec, ms);
                        field->setDateTime(DateTime(short(year), short(month), short(day), short(hour), short(min), short(sec)));
                    }
                    break;

                case Type(SQLT_BLOB):
                    {
                        Blob blob = resultSet->getBlob(columnIndex);
                        blob.open(OCCI_LOB_READONLY);
                        unsigned bytes = blob.length();
                        field->checkSize(bytes);
                        blob.read(bytes,
                                  (unsigned char*) field->getBuffer(),
                                  bytes,
                                  1);
                        blob.close();
                        field->setDataSize(bytes);
                    }
                    break;

                case Type(SQLT_CLOB):
                    {
                        Clob clob = resultSet->getClob(columnIndex);
                        clob.open(OCCI_LOB_READONLY);
                        // Attention: clob stored as widechar
                        unsigned clobChars = clob.length();
                        unsigned clobBytes = clobChars * 4;
                        field->checkSize(clobBytes);
                        unsigned bytes = clob.read(clobChars,
                                                   (unsigned char*) field->getBuffer(),
                                                   clobBytes,
                                                   1);
                        clob.close();
                        field->setDataSize(bytes);
                    }
                    break;

                default:
                    field->setString(resultSet->getString(columnIndex));
                    break;
            }

        } catch (exception& e) {
            THROW_QUERY_ERROR(query, "Can't read field " << field->fieldName() << ": " << e.what());
        }
    }
}

void OracleConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
    case DOT_PROCEDURES:
        objectsSQL = "SELECT object_name FROM user_procedures";
        break;
    case DOT_FUNCTIONS:
        objectsSQL = "SELECT object_name FROM user_functions";
        break;
    case DOT_TABLES:
        objectsSQL = "SELECT table_name FROM user_tables";
        break;
    case DOT_VIEWS:
        objectsSQL = "SELECT view_name FROM user_views";
        break;
    case DOT_DATABASES:
        throw Exception("Not implemented yet");
    }
    Query query(this, objectsSQL);
    query.open();
    while (!query.eof()) {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }
    query.close();
}

void OracleConnection::_bulkInsert(const String& tableName, const Strings& columnNames, const Strings& data,
                                   const String& format)
{
    Query tableColumnsQuery(this,
                            "SELECT column_name, data_type, data_length "
                            "FROM user_tab_columns "
                            "WHERE table_name = :table_name");
    tableColumnsQuery.param("table_name") = upperCase(tableName);
    tableColumnsQuery.open();
    Field& column_name = tableColumnsQuery["column_name"];
    Field& data_type = tableColumnsQuery["data_type"];
    Field& data_length = tableColumnsQuery["data_length"];
    //string numericTypes("DECIMAL|FLOAT|DOUBLE|NUMBER");
    QueryColumnTypeSizeMap columnTypeSizeMap;
    while (!tableColumnsQuery.eof()) {
        String columnName = column_name.asString();
        String columnType = data_type.asString();
        auto maxDataLength = (size_t) data_length.asInteger();
        QueryColumnTypeSize columnTypeSize = {};
        columnTypeSize.type = VAR_STRING;
        columnTypeSize.length = 0;
        if (columnType.find("LOB") != string::npos) {
            columnTypeSize.type = VAR_TEXT;
            columnTypeSize.length = 65536;
        }
        if (columnType.find("CHAR") != string::npos)
            columnTypeSize.length = maxDataLength;
        columnTypeSizeMap[columnName] = columnTypeSize;
        tableColumnsQuery.fetch();
    }
    tableColumnsQuery.close();

    QueryColumnTypeSizeVector columnTypeSizeVector;
    for (auto& columnName: columnNames) {
        auto column = columnTypeSizeMap.find(upperCase(columnName));
        if (column == columnTypeSizeMap.end())
            throwDatabaseException("Column '" << columnName << "' doesn't belong to table " << tableName);
        columnTypeSizeVector.push_back(column->second);
    }

    OracleBulkInsertQuery insertQuery(this,
                                       "INSERT INTO " + tableName + "(" + columnNames.asString(",") +
                                       ") VALUES (:" + columnNames.asString(",:") + ")",
                                       data.size(),
                                       columnTypeSizeMap);
    for (auto& row: data) {
        Strings rowData(row,"\t");
        for (unsigned i = 0; i < columnNames.size(); i++) {
            if (columnTypeSizeVector[i].type == VAR_TEXT)
                insertQuery.param(i).setText(rowData[i]);
            else
                insertQuery.param(i).setString(rowData[i]);
        }
        insertQuery.execNext();
    }
}

String OracleConnection::driverDescription() const
{
    return m_environment.clientVersion();
}

String OracleConnection::paramMark(unsigned paramIndex)
{
    char mark[16];
    sprintf(mark, ":%i", paramIndex + 1);
    return string(mark);
}

void OracleConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    RegularExpression  matchStatementEnd("(;\\s*)$");
    RegularExpression  matchRoutineStart("^CREATE (OR REPLACE )?FUNCTION", "i");
    RegularExpression  matchGo("^/\\s*$");
    RegularExpression  matchShowErrors("^SHOW\\s+ERRORS", "i");
    RegularExpression  matchCommentRow("^\\s*--");

    Strings statements, matches;
    string statement;
    bool routineStarted = false;
    for (auto& arow: sqlBatch) {
        String row = trim(arow);
        if (row.empty() || matchCommentRow.matches(row))
            continue;

        if (!routineStarted) {
            row = trim(row);
            if (row.empty())
                continue;
            if (matchShowErrors.m(row, matches))
                continue;
        }

        if (matchRoutineStart.m(row, matches))
            routineStarted = true;

        if (!routineStarted && matchStatementEnd.m(row, matches)) {
            row = matchStatementEnd.s(row, "");
            statement += row;
            statements.push_back(trim(statement));
            statement = "";
            continue;
        }

        if (matchGo.m(row, matches)) {
            routineStarted = false;
            statements.push_back(trim(statement));
            statement = "";
            continue;
        }

        statement += row + "\n";
    }

    if (!trim(statement).empty())
        statements.push_back(statement);

    for (auto& stmt: statements) {
        try {
            Query query(this, stmt, false);
            query.exec();
        }
        catch (const exception& e) {
            if (errors)
                errors->push_back(e.what());
            else
                throw;
        }

    }
}


void* oracle_create_connection(const char* connectionString)
{
    OracleConnection* connection = new OracleConnection(connectionString);
    return connection;
}

void  oracle_destroy_connection(void* connection)
{
    delete (OracleConnection*) connection;
}
