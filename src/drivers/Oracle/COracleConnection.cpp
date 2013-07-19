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

COracleConnection::COracleConnection(string connectionString) :
    CDatabaseConnection(connectionString),
    m_connection(NULL)
{
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

        try {
            // Connection string in format: host[:port][/instance]
            string connectionString = m_connString.hostName();
            if (m_connString.portNumber())
                connectionString += ":" + int2string(m_connString.portNumber());
            if (m_connString.databaseName() != "")
                connectionString += "/" + m_connString.databaseName();
            m_connection = m_environment->createConnection(m_connString.userName(), m_connString.password(), connectionString);
        }
        catch (oracle::occi::SQLException& e) {
            m_connection = NULL;
            throwOracleException(string("Can't create connection: ") + e.what());
        }
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
        m_environment->terminateConnection(m_connection);
        m_connection = NULL;
    }
    catch (oracle::occi::SQLException& e) {
        m_connection = NULL;
        throwOracleException(string("Can't create connection: ") + e.what());
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
    catch (oracle::occi::SQLException& e) {
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
    if (statement)
        delete statement;
    querySetStmt(query, 0L);
    querySetPrepared(query, false);
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
    if (statement)
        delete statement;
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

oracle::occi::Type COracleConnection::VariantTypeToOracleType(CVariantType dataType)
{
    switch (dataType)
    {
        case VAR_NONE:
            throwException("Data type is not defined");
        case VAR_INT:
            return (Type) SQLT_INT;
        case VAR_FLOAT:
        case VAR_MONEY:
            return (Type) SQLT_BDOUBLE;
        case VAR_STRING:
            return (Type) SQLT_CHR;
        case VAR_TEXT:
            return (Type) SQLT_CLOB;
        case VAR_BUFFER:
            return (Type) SQLT_BLOB;
        case VAR_DATE:
        case VAR_DATE_TIME:
            return (Type) SQLT_TIMESTAMP;
        case VAR_INT64:
            return (Type) SQLT_INT;
        case VAR_BOOL:
            return (Type) SQLT_CHR;
        default:
            throwException("Unsupported SPTK data type: " + int2string(dataType));
    }
}
/*
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

    //query->fields().clear();

    COracleStatement* statement = (COracleStatement*) query->statement();
    //if (statement->rowCount() == 0)
    //    return;

    short count = queryColCount(query);
    if (count < 1) {
        //queryCloseStmt(query);
        return;
    } else {
        querySetActive(query, true);
        if (query->fieldCount() == 0) {
            SYNCHRONIZED_CODE;
            // Reading the column attributes
            char columnName[256];
            //long  columnType;
            //CVariantType dataType;
            const PGresult* stmt = statement->stmt();
            for (short column = 0; column < count; column++) {
                strncpy(columnName, PQfname(stmt, column), 255);
                columnName[255] = 0;
                if (columnName[0] == 0)
                    sprintf(columnName, "column%02i", column);
                Oid dataType = PQftype(stmt, column);
                CVariantType fieldType;
                PostgreTypeToCType(dataType, fieldType);
                int fieldLength = PQfsize(stmt, column);
                CDatabaseField* field = new CDatabaseField(columnName, column, dataType, fieldType, fieldLength);
                query->fields().push_back(field);
            }
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

// Converts internal NUMERIC Postgresql binary to long double
static long double numericBinaryToLongDouble(const char* v)
{
    int16_t ndigits = ntohs(*(int16_t*) v);
    int16_t weight = ntohs(*(int16_t*) (v + 2));
    int16_t sign = ntohs(*(int16_t*) (v + 4));
    //int16_t dscale  = ntohs(*(int16_t*)(v+6));

    v += 8;
    int64_t value = 0;
    int64_t decValue = 0;
    int64_t divider = 1;
    if (weight < 0) {
        for (int i = 0; i < -(weight + 1); i++)
            divider *= 10000;
        weight = -1;
    }
    for (int i = 0; i < ndigits; i++, v += 2) {
        int16_t digit = ntohs(*(int16_t*) v);
        if (i <= weight)
            value = value * 10000 + digit;
        else {
            decValue = decValue * 10000 + digit;
            divider *= 10000;
        }
    }
    long double finalValue = value + decValue / (long double) (divider);
    if (sign)
        finalValue = -finalValue;
    return finalValue;
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
    int dataLength = 0;

    if (!fieldCount)
        return;

    CDatabaseField* field = 0;
    const PGresult* stmt = statement->stmt();
    int currentRow = statement->currentRow();
    for (int column = 0; column < fieldCount; column++) {
        try {
            field = (CDatabaseField*) &(*query)[(int) column];
            short fieldType = (short) field->fieldType();

            bool isNull = false;
            dataLength = PQgetlength(stmt, currentRow, column);
            if (!dataLength) {
                if (fieldType & (VAR_STRING | VAR_TEXT | VAR_BUFFER))
                    isNull = PQgetisnull(stmt, currentRow, column);
                else
                    isNull = true;
                if (isNull)
                    field->setNull();
                else
                    field->setExternalString("", 0);
            } else {
                char* data = PQgetvalue(stmt, currentRow, column);

                switch (fieldType)
                {

                case PG_BOOL:
                    field->setBool((bool) *data);
                    break;

                case PG_INT2:
                    field->setInteger(ntohs(*(int16_t *) data));
                    break;

                case PG_OID:
                case PG_INT4:
                    field->setInteger(ntohl(*(int32_t *) data));
                    break;

                case PG_INT8:
                    field->setInt64(ntohq(*(int64_t *) data));
                    break;

                case PG_FLOAT4: {
                    int32_t v = ntohl(*(int32_t *) data);
                    field->setFloat(*(float *) (void *) &v);
                    break;
                }

                case PG_FLOAT8: {
                    int64_t v = ntohq(*(int64_t *) data);
                    field->setFloat(*(double *) (void *) &v);
                    break;
                }

                case PG_NUMERIC:
                    field->setFloat(numericBinaryToLongDouble(data));
                    break;

                default:
                    field->setExternalString(data, dataLength);
                    break;

                case PG_BYTEA:
                    field->setExternalBuffer(data, dataLength);
                    break;

                case PG_DATE: {
                    int32_t dt = ntohl(*(int32_t *) data);
                    field->setDateTime(dt + (int32_t) epochDate);
                    break;
                }

                case PG_TIME:
                case PG_TIMESTAMPTZ:
                case PG_TIMESTAMP: {
                    int64_t v = ntohq(*(int64_t *) data);
                    double val = (double) epochDate + *(double *) (void *) &v / 3600.0 / 24.0;
                    field->setDateTime(val);
                    break;
                }
                }
            }

        } catch (exception& e) {
            query->logAndThrow("COracleConnection::queryFetch",
                    "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
    }
}

void COracleConnection::objectList(CDbObjectType objectType, CStrings& objects) throw (CDatabaseException)
{
    string tablesSQL("SELECT table_schema || '.' || table_name "
                     "FROM information_schema.tables "
                     "WHERE table_schema NOT IN ('information_schema','pg_catalog') ");
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
    case DOT_PROCEDURES:
        objectsSQL = "SELECT DISTINCT routine_schema || '.' || routine_name "
                     "FROM information_schema.routines "
                     "WHERE routine_schema NOT IN ('information_schema','pg_catalog')";
        break;
    case DOT_TABLES:
        objectsSQL = tablesSQL + "AND table_type = 'BASE TABLE'";
        break;
    case DOT_VIEWS:
        objectsSQL = tablesSQL + "AND table_type = 'VIEW'";
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
    return "Oracle";
}

std::string COracleConnection::paramMark(unsigned paramIndex)
{
    char mark[16];
    sprintf(mark, "$%i", paramIndex + 1);
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
*/
