/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CFirebirdConnection.cpp  -  description
                             -------------------
    begin                : Wed Jul 24 2013
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
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

#include <sptk5/db/CFirebirdConnection.h>
#include <sptk5/db/CFirebirdStatement.h>
#include <sptk5/db/CDatabaseField.h>
#include <sptk5/db/CQuery.h>

#include <string>
#include <stdio.h>

using namespace std;
using namespace sptk;

CFirebirdConnection::CFirebirdConnection(string connectionString) :
    CDatabaseConnection(connectionString),
    m_connection(0)
{
    m_connType = DCT_FIREBIRD;
}

CFirebirdConnection::~CFirebirdConnection()
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

void CFirebirdConnection::checkStatus(const ISC_STATUS* status_vector, const char* file, int line) THROWS_EXCEPTIONS
{
    if (status_vector[0] == 1 && status_vector[1])
    {
        string errors;
        char error[256];
        const ISC_STATUS* pvector = status_vector;
        while (fb_interpret(error, sizeof(error), &pvector))
            errors += string(error) + " ";
        m_lastStatus = errors;
        throw CDatabaseException(errors, file, line);
    } else
        m_lastStatus.clear();
}

void CFirebirdConnection::openDatabase(string newConnectionString) THROWS_EXCEPTIONS
{
    ISC_STATUS status_vector[20];

    if (!active()) {
        m_inTransaction = false;
        if (newConnectionString.length())
            m_connString = newConnectionString;

        char dpb_buffer[256], *dpb;
        short dpb_length;

        dpb = dpb_buffer;
        *dpb++ = isc_dpb_version1;
        *dpb++ = isc_dpb_num_buffers;
        *dpb++ = 1;
        *dpb++ = 90;
        dpb_length = short(dpb - dpb_buffer);

        dpb = dpb_buffer;

        const string& username = m_connString.userName();
        if (!username.empty())
            isc_modify_dpb(&dpb, &dpb_length, isc_dpb_user_name, username.c_str(), (short) username.length());

        const string& password = m_connString.password();
        if (!password.empty())
            isc_modify_dpb(&dpb, &dpb_length, isc_dpb_password, password.c_str(), (short) password.length());

        m_connection = 0;
        string fullDatabaseName = m_connString.hostName() + ":/" + m_connString.databaseName();
        isc_attach_database(status_vector, (short) fullDatabaseName.length(), fullDatabaseName.c_str(), &m_connection, dpb_length, dpb);
        checkStatus(status_vector, __FILE__, __LINE__);
    }
}

void CFirebirdConnection::closeDatabase() THROWS_EXCEPTIONS
{
    ISC_STATUS status_vector[20];

    if (m_transaction)
        driverEndTransaction(false);

    for (unsigned i = 0; i < m_queryList.size(); i++) {
        try {
            CQuery *query = (CQuery *) m_queryList[i];
            queryFreeStmt(query);
        } catch (...) {
        }
    }

    if (m_connection) {
        isc_detach_database(status_vector, &m_connection);
        m_connection = 0;
    }
}

void* CFirebirdConnection::handle() const
{
    union {
        isc_db_handle   connection;
        void*           handle;
    } convert;
    convert.connection = m_connection;
    return convert.handle;
}

bool CFirebirdConnection::active() const
{
    return m_connection != 0L;
}

string CFirebirdConnection::nativeConnectionString() const
{
    // Connection string in format: host[:port][/instance]
    string connectionString = m_connString.hostName();
    if (m_connString.portNumber())
        connectionString += ":" + int2string(m_connString.portNumber());
    if (m_connString.databaseName() != "")
        connectionString += "/" + m_connString.databaseName();
    return connectionString;
}

void CFirebirdConnection::driverBeginTransaction() THROWS_EXCEPTIONS
{
    ISC_STATUS status_vector[20];

    if (!m_connection)
        open();

    if (m_inTransaction) {
        driverEndTransaction(true);
        //throw CDatabaseException("Transaction already started");
    }

    m_transaction = 0L;
    static char isc_tpb[] = { isc_tpb_version3, isc_tpb_write, isc_tpb_read_committed, isc_tpb_no_rec_version, isc_tpb_wait};
    isc_start_transaction(status_vector, &m_transaction, 1, &m_connection, sizeof(isc_tpb), isc_tpb);
    checkStatus(status_vector, __FILE__, __LINE__);

    m_inTransaction = true;
}

void CFirebirdConnection::driverEndTransaction(bool commit) THROWS_EXCEPTIONS
{
    ISC_STATUS status_vector[20];

    if (!m_inTransaction)
        throwDatabaseException("Transaction isn't started.");

    if (commit)
        isc_commit_transaction(status_vector, &m_transaction);
    else
        isc_rollback_transaction(status_vector, &m_transaction);

    checkStatus(status_vector, __FILE__, __LINE__);
    m_transaction = 0L;
    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------
string CFirebirdConnection::queryError(const CQuery *) const
{
    return m_lastStatus;
}

void CFirebirdConnection::queryAllocStmt(CQuery *query)
{
    queryFreeStmt(query);
    querySetStmt(query, new CFirebirdStatement(this, query->sql()));
}

void CFirebirdConnection::queryFreeStmt(CQuery *query)
{
    SYNCHRONIZED_CODE;
    CFirebirdStatement* statement = (CFirebirdStatement*) query->statement();
    if (statement) {
        delete statement;
        querySetStmt(query, 0L);
        querySetPrepared(query, false);
    }
}

void CFirebirdConnection::queryCloseStmt(CQuery *query)
{
    SYNCHRONIZED_CODE;
    try {
        CFirebirdStatement* statement = (CFirebirdStatement*) query->statement();
        if (statement)
            statement->close();
    }
    catch (exception& e) {
        query->logAndThrow("CFirebirdConnection::queryBindParameters", e.what());
    }
}

void CFirebirdConnection::queryPrepare(CQuery *query)
{
    SYNCHRONIZED_CODE;

    if (!query->prepared()) {
        CFirebirdStatement* statement = (CFirebirdStatement*) query->statement();
        try {
            statement->prepare(query->sql());
            statement->enumerateParams(query->params());
        }
        catch (exception& e) {
            query->logAndThrow("CFirebirdConnection::queryBindParameters", e.what());
        }
        querySetPrepared(query, true);
    }
}

void CFirebirdConnection::queryUnprepare(CQuery *query)
{
    queryFreeStmt(query);
}

int CFirebirdConnection::queryColCount(CQuery *query)
{
    int colCount = 0;
    CFirebirdStatement* statement = (CFirebirdStatement*) query->statement();
    try {
        colCount = (int) statement->colCount();
    }
    catch (exception& e) {
        query->logAndThrow("CFirebirdConnection::queryColCount", e.what());
    }
    return colCount;
}

void CFirebirdConnection::queryBindParameters(CQuery *query)
{
    SYNCHRONIZED_CODE;

    CFirebirdStatement* statement = (CFirebirdStatement*) query->statement();
    try {
        if (!statement)
            throwDatabaseException("Query not prepared");
        statement->setParameterValues();
    }
    catch (exception& e) {
        query->logAndThrow("CFirebirdConnection::queryBindParameters", e.what());
    }
}

void CFirebirdConnection::queryExecute(CQuery *query)
{
    CFirebirdStatement* statement = (CFirebirdStatement*) query->statement();
    try {
        if (!statement)
            throwDatabaseException("Query is not prepared");
        statement->execute(m_inTransaction);
    }
    catch (exception& e) {
        query->logAndThrow("CFirebirdConnection::queryExecute", e.what());
    }
}

void CFirebirdConnection::queryOpen(CQuery *query)
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

    CFirebirdStatement* statement = (CFirebirdStatement*) query->statement();

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

void CFirebirdConnection::queryFetch(CQuery *query)
{
    if (!query->active())
        query->logAndThrow("CFirebirdConnection::queryFetch", "Dataset isn't open");

    SYNCHRONIZED_CODE;

    try {
        CFirebirdStatement* statement = (CFirebirdStatement*) query->statement();

        statement->fetch();

        if (statement->eof()) {
            querySetEof(query, true);
            return;
        }

        statement->fetchResult(query->fields());
    }
    catch (exception& e) {
        query->logAndThrow("CFirebirdConnection::queryFetch", e.what());
    }
}

void CFirebirdConnection::objectList(CDbObjectType objectType, CStrings& objects) THROWS_EXCEPTIONS
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
    case DOT_PROCEDURES:
        objectsSQL =
            "SELECT rdb$procedure_name object_name "
            "FROM rdb$procedures "
            "ORDER BY 1";
        break;
    case DOT_TABLES:
        objectsSQL =
            "SELECT rdb$relation_name object_name "
            "FROM rdb$relations "
            "WHERE rdb$system_flag = 0 AND rdb$view_source IS NULL "
            "ORDER BY 1";
        break;
    case DOT_VIEWS:
        objectsSQL =
            "SELECT rdb$relation_name object_name "
            "FROM rdb$relations "
            "WHERE rdb$system_flag = 0 AND rdb$view_source IS NOT NULL "
            "ORDER BY 1";
        break;
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

std::string CFirebirdConnection::driverDescription() const
{
    return "Firebird";
}

std::string CFirebirdConnection::paramMark(unsigned paramIndex)
{
    return "?";
}

void* firebird_create_connection(const char* connectionString)
{
    CFirebirdConnection* connection = new CFirebirdConnection(connectionString);
    return connection;
}

void  firebird_destroy_connection(void* connection)
{
    delete (CFirebirdConnection*) connection;
}
