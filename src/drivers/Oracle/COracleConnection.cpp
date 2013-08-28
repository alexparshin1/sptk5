/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          COracleConnection.cpp  -  description
                             -------------------
    begin                : Tue Nov 27 2012
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/db/COracleConnection.h>
#include <sptk5/db/CDatabaseField.h>
#include <sptk5/db/CQuery.h>

#include <string>
#include <stdio.h>

using namespace std;
using namespace sptk;
using namespace oracle::occi;

COracleConnection::COracleConnection(string connectionString) :
    CDatabaseConnection(connectionString),
    m_connection(NULL)
{
    m_connType = DCT_ORACLE;
}

COracleConnection::~COracleConnection()
{
    try {
        if (m_inTransaction && active())
            rollbackTransaction();
        close();
        while (m_queryList.size()) {
            try {
                CQuery *query = (CQuery *) m_queryList[0];
                query->disconnect();
            } catch (...) {
            }
        }
        m_queryList.clear();
    } catch (...) {
    }
}

void COracleConnection::openDatabase(string newConnectionString) throw (CDatabaseException)
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
                m_connection->terminateStatement(createLOBtable);
                m_environment.terminateConnection(m_connection);
                m_connection = NULL;
                throwOracleException(string("Can't create connection: ") + e.what());
            }
        }
        m_connection->terminateStatement(createLOBtable);
    }
}

void COracleConnection::closeDatabase() throw (CDatabaseException)
{
    for (unsigned i = 0; i < m_queryList.size(); i++) {
        try {
            CQuery *query = (CQuery *) m_queryList[i];
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

void* COracleConnection::handle() const
{
    return m_connection;
}

bool COracleConnection::active() const
{
    return m_connection != 0L;
}

string COracleConnection::nativeConnectionString() const
{
    // Connection string in format: host[:port][/instance]
    string connectionString = m_connString.hostName();
    if (m_connString.portNumber())
        connectionString += ":" + int2string(m_connString.portNumber());
    if (m_connString.databaseName() != "")
        connectionString += "/" + m_connString.databaseName();
    return connectionString;
}

void COracleConnection::driverBeginTransaction() throw (CDatabaseException)
{
    if (!m_connection)
        open();

    if (m_inTransaction)
        throwOracleException("Transaction already started.");

    m_inTransaction = true;
}

void COracleConnection::driverEndTransaction(bool commit) throw (CDatabaseException)
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
string COracleConnection::queryError(const CQuery *query) const
{
    return m_lastError;
}

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void COracleConnection::queryAllocStmt(CQuery *query)
{
    queryFreeStmt(query);
    querySetStmt(query, new COracleStatement(this, query->sql()));
}

void COracleConnection::queryFreeStmt(CQuery *query)
{
    SYNCHRONIZED_CODE;
    COracleStatement* statement = (COracleStatement*) query->statement();
    if (statement) {
        delete statement;
        querySetStmt(query, 0L);
        querySetPrepared(query, false);
    }
}

void COracleConnection::queryCloseStmt(CQuery *query)
{
    SYNCHRONIZED_CODE;
    COracleStatement* statement = (COracleStatement*) query->statement();
    if (statement)
        statement->close();
}

void COracleConnection::queryPrepare(CQuery *query)
{
    SYNCHRONIZED_CODE;

    COracleStatement* statement = (COracleStatement*) query->statement();
    if (!statement)
        statement = new COracleStatement(this, query->sql());
    statement->enumerateParams(query->params());
    querySetStmt(query, statement);
    querySetPrepared(query, true);
}

void COracleConnection::queryUnprepare(CQuery *query)
{
    queryFreeStmt(query);
}

int COracleConnection::queryColCount(CQuery *query)
{
    COracleStatement* statement = (COracleStatement*) query->statement();
    if (!statement)
        throwOracleException("Query not opened");
    return statement->colCount();
}

void COracleConnection::queryBindParameters(CQuery *query)
{
    SYNCHRONIZED_CODE;

    COracleStatement* statement = (COracleStatement*) query->statement();
    if (!statement)
        throwDatabaseException("Query not prepared");
    statement->setParameterValues();
}

CVariantType COracleConnection::OracleTypeToVariantType(Type oracleType)
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

Type COracleConnection::VariantTypeToOracleType(CVariantType dataType)
{
    switch (dataType)
    {
        case VAR_NONE:
            throwException("Data type is not defined");
        case VAR_INT:
            return (Type) SQLT_INT;
        case VAR_FLOAT:
        case VAR_MONEY:
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

void COracleConnection::queryExecute(CQuery *query)
{
    COracleStatement* statement = (COracleStatement*) query->statement();
    if (!statement)
        throwOracleException("Query is not prepared");
    statement->execute(m_inTransaction);
}

void COracleConnection::queryOpen(CQuery *query)
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

    COracleStatement* statement = (COracleStatement*) query->statement();

    queryExecute(query);
    short count = queryColCount(query);
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
                CVariantType dataType = OracleTypeToVariantType(columnType);
                CDatabaseField* field = new CDatabaseField(columnName, columnIndex, columnType, dataType, columnDataSize);
                query->fields().push_back(field);
            }
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

void COracleConnection::queryFetch(CQuery *query)
{
    if (!query->active())
        query->logAndThrow("COracleConnection::queryFetch", "Dataset isn't open");

    SYNCHRONIZED_CODE;

    COracleStatement* statement = (COracleStatement*) query->statement();

    statement->fetch();
    if (statement->eof()) {
        querySetEof(query, true);
        return;
    }

    int fieldCount = query->fieldCount();
    if (!fieldCount)
        return;

    ResultSet* resultSet = statement->resultSet();
    CDatabaseField* field = 0;
    for (int fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {
        try {
            field = (CDatabaseField*) &(*query)[fieldIndex];

            // Result set column index starts from 1
            int columnIndex = fieldIndex + 1;

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
                        field->setDate(CDateTime(year, month, day, 0, 0, 0));
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
                        field->setDateTime(CDateTime(year, month, day, hour, min, sec));
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

void COracleConnection::objectList(CDbObjectType objectType, CStrings& objects) throw (CDatabaseException)
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
    default:
        return; // no information about objects of other types
    }
    CQuery query(this, objectsSQL);
    query.open();
    while (!query.eof()) {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }
    query.close();
}

std::string COracleConnection::driverDescription() const
{
    return m_environment.clientVersion();
}

std::string COracleConnection::paramMark(unsigned paramIndex)
{
    char mark[16];
    sprintf(mark, ":%i", paramIndex + 1);
    return mark;
}


void* oracle_create_connection(const char* connectionString)
{
    COracleConnection* connection = new COracleConnection(connectionString);
    return connection;
}

void  oracle_destroy_connection(void* connection)
{
    delete (COracleConnection*) connection;
}
