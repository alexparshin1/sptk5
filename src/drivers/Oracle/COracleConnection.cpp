/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       COracleConnection.cpp - description                    ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
#include "COracleBulkInsertQuery.h"

#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;
using namespace oracle::occi;

OracleConnection::OracleConnection(string connectionString) :
    DatabaseConnection(connectionString),
    m_connection(NULL)
{
    m_connType = DCT_ORACLE;
}

OracleConnection::~OracleConnection()
{
    try {
        if (m_inTransaction && active())
            rollbackTransaction();
        close();
        while (m_queryList.size()) {
            try {
                Query *query = (Query *) m_queryList[0];
                query->disconnect();
            } catch (...) {
            }
        }
        m_queryList.clear();
    } catch (...) {
    }
}

void OracleConnection::openDatabase(string newConnectionString) THROWS_EXCEPTIONS
{
    if (!active()) {
        m_inTransaction = false;
        if (newConnectionString.length())
            m_connString = newConnectionString;

        Statement* createLOBtable = NULL;
        try {
            m_connection = m_environment.createConnection(m_connString);
            createLOBtable = m_connection->createStatement();
            createLOBtable->setSQL("CREATE GLOBAL TEMPORARY TABLE sptk_lobs (sptk_clob CLOB, sptk_blob BLOB) ON COMMIT DELETE ROWS");
            createLOBtable->executeUpdate();
        }
        catch (SQLException& e) {
            if (strstr(e.what(),"already used") == NULL) {
                if (m_connection) {
                    m_connection->terminateStatement(createLOBtable);
                    m_environment.terminateConnection(m_connection);
                    m_connection = NULL;
                }
                throwOracleException(string("Can't create connection: ") + e.what());
            }
        }
        m_connection->terminateStatement(createLOBtable);
    }
}

void OracleConnection::closeDatabase() THROWS_EXCEPTIONS
{
    for (unsigned i = 0; i < m_queryList.size(); i++) {
        try {
            Query *query = (Query *) m_queryList[i];
            queryFreeStmt(query);
        } catch (...) {
        }
    }

    try {
        m_environment.terminateConnection(m_connection);
        m_connection = NULL;
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
    return m_connection != 0L;
}

string OracleConnection::nativeConnectionString() const
{
    // Connection string in format: host[:port][/instance]
    string connectionString = m_connString.hostName();
    if (m_connString.portNumber())
        connectionString += ":" + int2string(m_connString.portNumber());
    if (m_connString.databaseName() != "")
        connectionString += "/" + m_connString.databaseName();
    return connectionString;
}

void OracleConnection::driverBeginTransaction() THROWS_EXCEPTIONS
{
    if (!m_connection)
        open();

    if (m_inTransaction)
        throwOracleException("Transaction already started.");

    m_inTransaction = true;
}

void OracleConnection::driverEndTransaction(bool commit) THROWS_EXCEPTIONS
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
string OracleConnection::queryError(const Query *query) const
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
    SYNCHRONIZED_CODE;
    OracleStatement* statement = (OracleStatement*) query->statement();
    if (statement) {
        delete statement;
        querySetStmt(query, 0L);
        querySetPrepared(query, false);
    }
}

void OracleConnection::queryCloseStmt(Query *query)
{
    SYNCHRONIZED_CODE;
    OracleStatement* statement = (OracleStatement*) query->statement();
    if (statement)
        statement->close();
}

void OracleConnection::queryPrepare(Query *query)
{
    SYNCHRONIZED_CODE;

    OracleStatement* statement = (OracleStatement*) query->statement();
    if (!statement)
        statement = new OracleStatement(this, query->sql());
    statement->enumerateParams(query->params());
    if (query->bulkMode()) {
        CParamVector& enumeratedParams = statement->enumeratedParams();
        unsigned paramIndex = 1;
        Statement* stmt = statement->stmt();
        COracleBulkInsertQuery* bulkInsertQuery = dynamic_cast<COracleBulkInsertQuery*>(query);
        const QueryColumnTypeSizeMap& columnTypeSizes = bulkInsertQuery->columnTypeSizes();
        for (CParamVector::iterator itor = enumeratedParams.begin(); itor != enumeratedParams.end(); itor++, paramIndex++) {
            QueryParameter* param = *itor;
            QueryColumnTypeSizeMap::const_iterator xtor = columnTypeSizes.find(upperCase(param->name()));
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
    OracleStatement* statement = (OracleStatement*) query->statement();
    if (!statement)
        throwOracleException("Query not opened");
    return (int) statement->colCount();
}

void OracleConnection::queryBindParameters(Query *query)
{
    SYNCHRONIZED_CODE;

    OracleStatement* statement = (OracleStatement*) query->statement();
    if (!statement)
        throwDatabaseException("Query not prepared");
    try {
        statement->setParameterValues();
    }
    catch (const SQLException& e) {
        throwOracleException(e.what());
    }
}

VariantType OracleConnection::OracleTypeToVariantType(Type oracleType)
{
    switch (oracleType)
    {
        case SQLT_NUM:
            return VAR_FLOAT;
        case SQLT_INT:
        case SQLT_FLT:
        case SQLT_UIN:
            return VAR_INT64;
        case SQLT_DAT:
        case SQLT_DATE:
            return VAR_DATE;
        case SQLT_BFLOAT:
        case SQLT_BDOUBLE:
            return VAR_FLOAT;
        case SQLT_BLOB:
            return VAR_BUFFER;
        case SQLT_CLOB:
            return VAR_TEXT;
        case SQLT_TIME:
        case SQLT_TIME_TZ:
        case SQLT_TIMESTAMP:
        case SQLT_TIMESTAMP_TZ:
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
            throwException("Unsupported SPTK data type: " + int2string(dataType));
    }
}

void OracleConnection::queryExecute(Query *query)
{
    try {
        OracleStatement* statement = (OracleStatement*) query->statement();
        if (!statement)
            throwOracleException("Query is not prepared");
        if (query->bulkMode()) {
            COracleBulkInsertQuery* bulkInsertQuery = dynamic_cast<COracleBulkInsertQuery*>(query);
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

    OracleStatement* statement = (OracleStatement*) query->statement();

    queryExecute(query);
    int count = queryColCount(query);
    if (count < 1) {
        //queryCloseStmt(query);
        return;
    } else {
        querySetActive(query, true);
        if (query->fieldCount() == 0) {
            SYNCHRONIZED_CODE;

            ResultSet* resultSet = statement->resultSet();
            vector<MetaData> resultSetMetaData = resultSet->getColumnListMetaData();
            vector<MetaData>::iterator
                itor = resultSetMetaData.begin(),
                iend = resultSetMetaData.end();
            int columnIndex = 0;
            for (; itor != iend; itor++, columnIndex++) {
                MetaData& metaData = *itor;
                Type columnType = (Type) metaData.getInt(MetaData::ATTR_DATA_TYPE);
                string columnName = metaData.getString(MetaData::ATTR_NAME);
                int columnDataSize = metaData.getInt(MetaData::ATTR_DATA_SIZE);
                if (columnName.empty()) {
                    char alias[32];
                    sprintf(alias, "column_%02i", columnIndex + 1);
                    columnName = alias;
                }
                if (columnType == Type::OCCI_SQLT_LNG && columnDataSize == 0)
                    resultSet->setMaxColumnSize(columnIndex + 1, 16384);
                VariantType dataType = OracleTypeToVariantType(columnType);
                DatabaseField* field = new DatabaseField(columnName, columnIndex, columnType, dataType, columnDataSize);
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
        query->logAndThrow("COracleConnection::queryFetch", "Dataset isn't open");

    SYNCHRONIZED_CODE;

    OracleStatement* statement = (OracleStatement*) query->statement();

    statement->fetch();
    if (statement->eof()) {
        querySetEof(query, true);
        return;
    }

    uint32_t fieldCount = query->fieldCount();
    if (!fieldCount)
        return;

    ResultSet* resultSet = statement->resultSet();
    DatabaseField* field = 0;
    for (uint32_t fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {
        try {
            field = (DatabaseField*) &(*query)[fieldIndex];

            // Result set column index starts from 1
            unsigned columnIndex = fieldIndex + 1;

            if (resultSet->isNull(columnIndex)) {
                field->setNull();
                continue;
            }

            int         year;
            unsigned    month, day, hour, min, sec, ms;

            switch ((Type)field->fieldType())
            {
                case SQLT_INT:
                case SQLT_UIN:
                    field->setInteger(resultSet->getInt(columnIndex));
                    break;

                case SQLT_NUM:
                    field->setFloat(resultSet->getNumber(columnIndex));
                    break;

                case SQLT_FLT:
                case SQLT_BFLOAT:
                    field->setFloat(resultSet->getFloat(columnIndex));
                    break;

                case SQLT_BDOUBLE:
                    field->setFloat(resultSet->getDouble(columnIndex));
                    break;

                case SQLT_DAT:
                case SQLT_DATE:
                    {
                        resultSet->getDate(columnIndex).getDate(year, month, day, hour, min, sec);
                        field->setDate(DateTime(short(year), short(month), short(day), short(0), short(0), short(0)));
                    }
                    break;

                case SQLT_TIME:
                case SQLT_TIME_TZ:
                case SQLT_TIMESTAMP:
                case SQLT_TIMESTAMP_TZ:
                    {
                        Timestamp timestamp = resultSet->getTimestamp(columnIndex);
                        timestamp.getDate(year, month, day);
                        timestamp.getTime(hour, min, sec, ms);
                        field->setDateTime(DateTime(short(year), short(month), short(day), short(hour), short(min), short(sec)));
                    }
                    break;

                case SQLT_BLOB:
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

                case SQLT_CLOB:
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
            query->logAndThrow("COracleConnection::queryFetch",
                               "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
    }
}

void OracleConnection::objectList(DatabaseObjectType objectType, Strings& objects) THROWS_EXCEPTIONS
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
    case DOT_PROCEDURES:
        objectsSQL = "SELECT object_name FROM user_procedures";
        break;
    case DOT_TABLES:
        objectsSQL = "SELECT table_name FROM user_tables";
        break;
    case DOT_VIEWS:
        objectsSQL = "SELECT view_name FROM user_views";
        break;
    }
    Query query(this, objectsSQL);
    query.open();
    while (!query.eof()) {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }
    query.close();
}

void OracleConnection::bulkInsert(std::string tableName, const Strings& columnNames, const Strings& data, std::string format) THROWS_EXCEPTIONS
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
        string columnName = column_name.asString();
        string columnType = data_type.asString();
        size_t maxDataLength = (size_t) data_length.asInteger();
        QueryColumnTypeSize columnTypeSize;
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
    for (Strings::const_iterator itor = columnNames.begin(); itor != columnNames.end(); itor++) {
        map<string,QueryColumnTypeSize>::iterator column = columnTypeSizeMap.find(upperCase(*itor));
        if (column == columnTypeSizeMap.end())
            throwDatabaseException("Column '" + *itor + "' doesn't belong to table " + tableName);
        columnTypeSizeVector.push_back(column->second);
    }

    COracleBulkInsertQuery insertQuery(this,
                                       "INSERT INTO " + tableName + "(" + columnNames.asString(",") +
                                       ") VALUES (:" + columnNames.asString(",:") + ")",
                                       data.size(),
                                       columnTypeSizeMap);
    for (Strings::const_iterator row = data.begin(); row != data.end(); row++) {
        Strings rowData(*row,"\t");
        for (unsigned i = 0; i < columnNames.size(); i++) {
            if (columnTypeSizeVector[i].type == VAR_TEXT)
                insertQuery.param(i).setText(rowData[i]);
            else
                insertQuery.param(i).setString(rowData[i]);
        }
        insertQuery.execNext();
    }
}

std::string OracleConnection::driverDescription() const
{
    return m_environment.clientVersion();
}

std::string OracleConnection::paramMark(unsigned paramIndex)
{
    char mark[16];
    sprintf(mark, ":%i", paramIndex + 1);
    return string(mark);
}

void OracleConnection::executeBatchFile(const Strings& sqlBatch) THROWS_EXCEPTIONS
{
    RegularExpression  matchStatementEnd("(;\\s*)$");
    RegularExpression  matchRoutineStart("^CREATE (OR REPLACE )?FUNCTION", "i");
    RegularExpression  matchGo("^/\\s*$");
    RegularExpression  matchShowErrors("^SHOW\\s+ERRORS", "i");

    Strings statements, matches;
    string statement;
    bool routineStarted = false;
    for (string row: sqlBatch) {

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

    for (string stmt: statements) {
        Query query(this, stmt, false);
        //cout << "[ " << statement << " ]" << endl;
        query.exec();
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
