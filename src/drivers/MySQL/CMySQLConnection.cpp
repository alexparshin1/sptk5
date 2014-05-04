/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CMySQLConnection.cpp  -  description
                             -------------------
    begin                : Wed Jul 24 2013
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

#include <sptk5/db/CMySQLConnection.h>
#include <sptk5/db/CDatabaseField.h>
#include <sptk5/db/CQuery.h>

#include <string>
#include <stdio.h>

using namespace std;
using namespace sptk;

CMySQLConnection::CMySQLConnection(string connectionString) :
    CDatabaseConnection(connectionString),
    m_connection(NULL)
{
    m_connType = DCT_MYSQL;
}

CMySQLConnection::~CMySQLConnection()
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


void CMySQLConnection::openDatabase(string newConnectionString) THROWS_EXCEPTIONS
{
    if (!active()) {
        m_inTransaction = false;
        if (newConnectionString.length())
            m_connString = newConnectionString;

        m_connection = mysql_init(m_connection);

        string connectionError;
        if (!m_connection)
            connectionError = "Can't initialize MySQL environment";
        else {
            if (!mysql_real_connect(m_connection,
                                    m_connString.hostName().c_str(),
                                    m_connString.userName().c_str(),
                                    m_connString.password().c_str(),
                                    m_connString.databaseName().c_str(),
                                    m_connString.portNumber(),
                                    NULL,
                                    CLIENT_MULTI_RESULTS)) {
                connectionError = mysql_error(m_connection);
                mysql_close(m_connection);
                m_connection = NULL;
            }
        }

        if (!m_connection)
            throw CDatabaseException("Can't connect to MySQL: " + connectionError);
    }
}

void CMySQLConnection::closeDatabase() THROWS_EXCEPTIONS
{
    for (unsigned i = 0; i < m_queryList.size(); i++) {
        try {
            CQuery *query = (CQuery *) m_queryList[i];
            queryFreeStmt(query);
        } catch (...) {
        }
    }

    if (m_connection) {
        mysql_close(m_connection);
        m_connection = NULL;
    }
}

void* CMySQLConnection::handle() const
{
    return m_connection;
}

bool CMySQLConnection::active() const
{
    return m_connection != 0L;
}

string CMySQLConnection::nativeConnectionString() const
{
    // Connection string in format: host[:port][/instance]
    string connectionString = m_connString.hostName();
    if (m_connString.portNumber())
        connectionString += ":" + int2string(m_connString.portNumber());
    if (m_connString.databaseName() != "")
        connectionString += "/" + m_connString.databaseName();
    return connectionString;
}

void CMySQLConnection::driverBeginTransaction() THROWS_EXCEPTIONS
{
    if (!m_connection)
        open();

    if (m_inTransaction)
        throwMySQLException("Transaction already started");

    m_inTransaction = true;

    if (mysql_query(m_connection, "BEGIN WORK") != 0)
        throwMySQLException("Can't start transaction");

    MYSQL_RES *result = mysql_store_result(m_connection);
    if (result)
        mysql_free_result(result);

    m_inTransaction = true;
}

void CMySQLConnection::driverEndTransaction(bool commit) THROWS_EXCEPTIONS
{
    if (!m_inTransaction)
        throwDatabaseException("Transaction isn't started.");

    const char* action = commit ? "COMMIT" : "ROLLBACK";
    if (mysql_query(m_connection, action) != 0)
        throwMySQLException(string(action) + " failed");

    MYSQL_RES *result = mysql_store_result(m_connection);
    if (result)
        mysql_free_result(result);

    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------
string CMySQLConnection::queryError(const CQuery *) const
{
    return mysql_error(m_connection);
}

void CMySQLConnection::queryAllocStmt(CQuery *query)
{
    queryFreeStmt(query);
    querySetStmt(query, new CMySQLStatement(this, query->sql()));
}

void CMySQLConnection::queryFreeStmt(CQuery *query)
{
    SYNCHRONIZED_CODE;
    CMySQLStatement* statement = (CMySQLStatement*) query->statement();
    if (statement) {
        delete statement;
        querySetStmt(query, 0L);
        querySetPrepared(query, false);
    }
}

void CMySQLConnection::queryCloseStmt(CQuery *query)
{
    SYNCHRONIZED_CODE;
    try {
        CMySQLStatement* statement = (CMySQLStatement*) query->statement();
        if (statement)
            statement->close();
    }
    catch (exception& e) {
        query->logAndThrow("CMySQLConnection::queryBindParameters", e.what());
    }
}

void CMySQLConnection::queryPrepare(CQuery *query)
{
    SYNCHRONIZED_CODE;

    if (!query->prepared()) {
        CMySQLStatement* statement = (CMySQLStatement*) query->statement();
        try {
            statement->prepare(query->sql());
            statement->enumerateParams(query->params());
        }
        catch (exception& e) {
            query->logAndThrow("CMySQLConnection::queryBindParameters", e.what());
        }
        querySetPrepared(query, true);
    }
}

void CMySQLConnection::queryUnprepare(CQuery *query)
{
    queryFreeStmt(query);
}

int CMySQLConnection::queryColCount(CQuery *query)
{
    int colCount = 0;
    CMySQLStatement* statement = (CMySQLStatement*) query->statement();
    try {
        if (!statement)
            throwDatabaseException("Query not opened");
        colCount = (int) statement->colCount();
    }
    catch (exception& e) {
        query->logAndThrow("CMySQLConnection::queryBindParameters", e.what());
    }
    return colCount;
}

void CMySQLConnection::queryBindParameters(CQuery *query)
{
    SYNCHRONIZED_CODE;

    CMySQLStatement* statement = (CMySQLStatement*) query->statement();
    try {
        if (!statement)
            throwDatabaseException("Query not prepared");
        statement->setParameterValues();
    }
    catch (exception& e) {
        query->logAndThrow("CMySQLConnection::queryBindParameters", e.what());
    }
}

void CMySQLConnection::queryExecute(CQuery *query)
{
    CMySQLStatement* statement = (CMySQLStatement*) query->statement();
    try {
        if (!statement)
            throwDatabaseException("Query is not prepared");
        statement->execute(m_inTransaction);
    }
    catch (exception& e) {
        query->logAndThrow("CMySQLConnection::queryExecute", e.what());
    }
}

void CMySQLConnection::queryOpen(CQuery *query)
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

    CMySQLStatement* statement = (CMySQLStatement*) query->statement();

    queryExecute(query);
    short fieldCount = (short) queryColCount(query);
    if (fieldCount < 1) {
        return;
    } else {
        querySetActive(query, true);
        if (query->fieldCount() == 0) {
            SYNCHRONIZED_CODE;
            statement->bindResult(query->fields());
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

void CMySQLConnection::queryFetch(CQuery *query)
{
    if (!query->active())
        query->logAndThrow("CMySQLConnection::queryFetch", "Dataset isn't open");

    SYNCHRONIZED_CODE;

    try {
        CMySQLStatement* statement = (CMySQLStatement*) query->statement();

        statement->fetch();
        if (statement->eof()) {
            querySetEof(query, true);
            return;
        }

        statement->fetchResult(query->fields());
    }
    catch (exception& e) {
        query->logAndThrow("CMySQLConnection::queryFetch", e.what());
    }
}

void CMySQLConnection::objectList(CDbObjectType objectType, CStrings& objects) THROWS_EXCEPTIONS
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
    case DOT_PROCEDURES:
        objectsSQL = 
            "SELECT CONCAT(routine_schema, '.', routine_name) object_name "
            "FROM information_schema.routines";
        break;
    case DOT_TABLES:
        objectsSQL = 
            "SELECT CONCAT(table_schema, '.', table_name) object_name "
            "FROM information_schema.tables "
            "WHERE NOT table_schema IN ('mysql','information_schema')";
        break;
    case DOT_VIEWS:
        objectsSQL = 
            "SELECT CONCAT(table_schema, '.', table_name) object_name "
            "FROM information_schema.views";
        break;
    default:
        return; // no information about objects of other types
    }
    CQuery query(this, objectsSQL);
    try {
        query.open();
        while (!query.eof()) {
            objects.push_back(query[uint32_t(0)].asString());
            query.next();
        }
        query.close();
    }
    catch (exception& e) {
        cerr << "Error fetching system info: " << e.what() << endl;
    }
}

void CMySQLConnection::bulkInsert(std::string tableName, const CStrings& columnNames, const CStrings& data, std::string format) THROWS_EXCEPTIONS
{
    char    fileName[256];
    sprintf(fileName, ".bulk.insert.%i.%i", getpid(), rand());
    data.saveToFile(fileName);
    string sql = "LOAD DATA LOCAL INFILE '" + string(fileName) + "' INTO TABLE " + tableName + " (" + columnNames.asString(",") + ") " + format;

    int rc = mysql_query(m_connection, sql.c_str());
    unlink(fileName);
    if (rc) {
        string error = mysql_error(m_connection);
        throwDatabaseException(error);
    }    
}

std::string CMySQLConnection::driverDescription() const
{
    if (m_connection)
        return string("MySQL ") + mysql_get_server_info(m_connection);
    return "MySQL";
}

std::string CMySQLConnection::paramMark(unsigned paramIndex)
{
    return "?";
}

void* mysql_create_connection(const char* connectionString)
{
    CMySQLConnection* connection = new CMySQLConnection(connectionString);
    return connection;
}

void  mysql_destroy_connection(void* connection)
{
    delete (CMySQLConnection*) connection;
}
