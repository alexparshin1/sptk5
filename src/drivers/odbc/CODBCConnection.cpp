/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CODBCDatabase.cpp  -  description
                             -------------------
    begin                : Fri Oct 03 2003
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#include <sptk5/db/CODBCDatabase.h>
#include <sptk5/db/CQuery.h>
#include <sptk5/db/CParams.h>
#include <sptk5/db/CDatabaseField.h>
#include <string>
#include <stdio.h>

using namespace std;
using namespace sptk;

namespace sptk {

    class CODBCField : public CDatabaseField
    {
        friend class CODBCDatabase;
    protected:
        char *getData()
        {
            return (char *) &m_data;
        }
    public:
        CODBCField(const std::string fieldName, int fieldColumn, int fieldType, CVariantType dataType, int fieldLength, int fieldScale) :
            CDatabaseField(fieldName, fieldColumn, fieldType, dataType, fieldLength, fieldScale) {}};

}

CODBCDatabase::CODBCDatabase(string connectionString) :
        CDatabaseDriver(connectionString)
{
    m_connect = new CODBCConnection;
}

CODBCDatabase::~CODBCDatabase()
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
        delete m_connect;
    } catch (...) {
    }
}

void CODBCDatabase::openDatabase(const string newConnectionString) throw (CDatabaseException)
{
    if (!active()) {
        m_inTransaction = false;
        if (newConnectionString.length())
            m_connString = newConnectionString;

        string finalConnectionString;
        m_connect->connect(m_connString, finalConnectionString, false);
    }
}

void CODBCDatabase::closeDatabase() throw (CDatabaseException)
{
    for (unsigned i = 0; i < m_queryList.size(); i++) {
        try {
            queryFreeStmt(m_queryList[i]);
        } catch (...) {
        }
    }
    m_connect->freeConnect();
}

void* CODBCDatabase::handle() const
{
    return m_connect->handle();
}

bool CODBCDatabase::active() const
{
    return m_connect->isConnected();
}

void CODBCDatabase::driverBeginTransaction() throw (CDatabaseException)
{
    if (!m_connect->isConnected())
        open();

    if (m_inTransaction)
        logAndThrow("CODBCDatabase::driverBeginTransaction", "Transaction already started.");

    m_connect->beginTransaction();
    m_inTransaction = true;
}

void CODBCDatabase::driverEndTransaction(bool commit) throw (CDatabaseException)
{
    if (!m_inTransaction) {
        if (commit)
            logAndThrow("CODBCDatabase::driverEndTransaction", "Can't commit - transaction isn't started.");
        else
            logAndThrow("CODBCDatabase::driverEndTransaction", "Can't rollback - transaction isn't started.");
    }

    if (commit)
        m_connect->commit();
    else
        m_connect->rollback();

    m_connect->setConnectOption(SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------
static inline BOOL successful(int ret)
{
    return ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO;
}

string CODBCDatabase::connectString() const
{
    return m_connect->connectString();
}

string CODBCDatabase::queryError(SQLHSTMT stmt) const
{
    SQLCHAR errorDescription[SQL_MAX_MESSAGE_LENGTH];
    SQLCHAR errorState[SQL_MAX_MESSAGE_LENGTH];

    SWORD pcnmsg = 0;
    SQLINTEGER nativeError = 0;
    *errorDescription = 0;
    *errorState = 0;

    int rc = SQLError(SQL_NULL_HENV, (SQLHDBC) handle(), stmt, errorState, &nativeError, errorDescription,
            sizeof(errorDescription), &pcnmsg);
    if (rc != SQL_SUCCESS) {
        rc = SQLError(SQL_NULL_HENV, (SQLHDBC) handle(), 0L, errorState, &nativeError, errorDescription,
                sizeof(errorDescription), &pcnmsg);
        if (rc != SQL_SUCCESS)
            if (!*errorDescription)
                strcpy((char *) errorDescription, "Unknown error");
    }

    return string(removeDriverIdentification((char *) errorDescription));
}

string CODBCDatabase::queryError(const CQuery *query) const
{
    return queryError(query->statement());
}

void CODBCDatabase::queryAllocStmt(CQuery *query)
{
    CSynchronizedCode lock(m_connect);

    SQLHSTMT stmt = (SQLHSTMT) query->statement();
    if (stmt != SQL_NULL_HSTMT)
        SQLFreeStmt(stmt, SQL_DROP);

    SQLHDBC hdb = (SQLHDBC) handle();
    int rc = SQLAllocStmt(hdb, &stmt);

    if (rc != SQL_SUCCESS) {
        string error = queryError(query);
        querySetStmt(query, SQL_NULL_HSTMT);
        logAndThrow("CODBCDatabase::queryAllocStmt", "Can't allocate statement. ");
    }

    querySetStmt(query, stmt);
}

void CODBCDatabase::queryFreeStmt(CQuery *query)
{
    CSynchronizedCode lock(m_connect);

    SQLFreeStmt(query->statement(), SQL_DROP);
    querySetStmt(query, SQL_NULL_HSTMT);
    querySetPrepared(query, false);
}

void CODBCDatabase::queryCloseStmt(CQuery *query)
{
    CSynchronizedCode lock(m_connect);

    SQLFreeStmt(query->statement(), SQL_CLOSE);
}

void CODBCDatabase::queryPrepare(CQuery *query)
{
    CSynchronizedCode lock(m_connect);

    query->fields().clear();

    if (!successful(SQLPrepare(query->statement(), (SQLCHAR *) query->sql().c_str(), SQL_NTS)))
        query->logAndThrow("CODBCDatabase::queryPrepare", queryError(query));
}

void CODBCDatabase::queryUnprepare(CQuery *query)
{
    queryFreeStmt(query);
    query->fields().clear();
}

void CODBCDatabase::queryExecute(CQuery *query)
{
    CSynchronizedCode lock(m_connect);

    int rc = 0;
    if (query->prepared())
        rc = SQLExecute(query->statement());
    else
        rc = SQLExecDirect(query->statement(), (SQLCHAR *) query->sql().c_str(), SQL_NTS);

    if (rc != SQL_SUCCESS) {
        SQLCHAR state[16];
        SQLCHAR text[1024];
        SQLINTEGER nativeError = 0;
        SQLINTEGER recordCount = 0;
        SQLSMALLINT textLength = 0;

        int rc = SQLGetDiagField(SQL_HANDLE_STMT, query->statement(), 1, SQL_DIAG_NUMBER, &recordCount, sizeof(recordCount),
                &textLength);
        if (!successful(rc))
            query->logAndThrow("CODBCDatabase::queryExecute", "Unable to retrieve diag records");
        for (SQLSMALLINT recordNumber = 1; recordNumber <= recordCount; recordNumber++) {
            rc = SQLGetDiagRec(SQL_HANDLE_STMT, query->statement(), recordNumber, state, &nativeError, text, sizeof(text),
                    &textLength);
            if (!successful(rc))
                break;
            query->messages().push_back(removeDriverIdentification((const char *) text));
        }
    }

    if (!successful(rc))
        query->logAndThrow("CODBCDatabase::queryExecute", queryError(query));
}

int CODBCDatabase::queryColCount(CQuery *query)
{
    CSynchronizedCode lock(m_connect);

    int16_t count = 0;
    if (!successful(SQLNumResultCols(query->statement(), &count)))
        query->logAndThrow("CODBCDatabase::queryColCount", queryError(query));

    return count;
}

void CODBCDatabase::queryColAttributes(CQuery *query, int16_t column, int16_t descType, int32_t& value)
{
    CSynchronizedCode lock(m_connect);

    if (!successful(SQLColAttributes(query->statement(), column, descType, 0, 0, 0, (SQLLEN *) &value)))
        query->logAndThrow("CODBCDatabase::queryColAttributes", queryError(query));
}

void CODBCDatabase::queryColAttributes(CQuery *query, int16_t column, int16_t descType, LPSTR buff, int len)
{
    int16_t available;
    if (!buff || len <= 0)
        query->logAndThrow("CODBCDatabase::queryColAttributes", "Invalid buffer or buffer len");

    CSynchronizedCode lock(m_connect);

    if (!successful(SQLColAttributes(query->statement(), (int16_t) column, descType, buff, (int16_t) len, &available, 0)))
        query->logAndThrow("CODBCDatabase::queryColAttributes", queryError(query));
}

void CODBCDatabase::queryBindParameters(CQuery *query)
{
    static SQLLEN cbNullValue = SQL_NULL_DATA;

    CSynchronizedCode lock(m_connect);
    int rc;

    for (uint32_t i = 0; i < query->paramCount(); i++) {
        CParam *param = &query->param(i);
        CVariantType ptype = param->dataType();
        SQLLEN& cblen = (SQLLEN&) param->callbackLength();
        for (unsigned j = 0; j < param->bindCount(); j++) {

            int16_t paramType = 0, sqlType = 0, scale = 0;
            void *buff = param->dataBuffer();
            long len = 0;
            int16_t paramNumber = int16_t(param->bindIndex(j) + 1);

            int16_t parameterMode = SQL_PARAM_INPUT;
            switch (ptype)
            {
            case VAR_BOOL:
                paramType = SQL_C_BIT;
                sqlType = SQL_BIT;
                break;
            case VAR_INT:
                paramType = SQL_C_SLONG;
                sqlType = SQL_INTEGER;
                break;
            case VAR_INT64:
                paramType = SQL_C_SBIGINT;
                sqlType = SQL_BIGINT;
                break;
            case VAR_FLOAT:
                paramType = SQL_C_DOUBLE;
                sqlType = SQL_DOUBLE;
                break;
            case VAR_STRING:
                buff = (void *) param->getString();
                len = (long) param->dataSize();
                paramType = SQL_C_CHAR;
                sqlType = SQL_CHAR;
                break;
            case VAR_TEXT:
                buff = (void *) param->getString();
                len = (long) param->dataSize();
                paramType = SQL_C_CHAR;
                sqlType = SQL_LONGVARCHAR;
                break;
            case VAR_BUFFER: {
                buff = (void *) param->getString();
                len = (long) param->dataSize();
                paramType = SQL_C_BINARY;
                sqlType = SQL_LONGVARBINARY;
                cblen = len;
                rc = SQLBindParameter((SQLHSTMT) query->statement(), paramNumber, parameterMode, SQL_C_BINARY,
                        SQL_LONGVARBINARY, len, scale, buff, SQLINTEGER(len), &cblen);
                continue;
            }
                break;
            case VAR_DATE: {
                paramType = SQL_C_TIMESTAMP;
                sqlType = SQL_TIMESTAMP;
                len = sizeof(TIMESTAMP_STRUCT);
                TIMESTAMP_STRUCT *t = (TIMESTAMP_STRUCT *) param->conversionBuffer();
                CDateTime dt = param->getDateTime();
                buff = t;
                if (dt) {
                    dt.decodeDate((int16_t *) &t->year, (int16_t *) &t->month, (int16_t *) &t->day);
                    t->hour = t->minute = t->second = 0;
                    t->fraction = 0;
                } else {
                    paramType = SQL_C_CHAR;
                    sqlType = SQL_CHAR;
                    *(char *) buff = 0;
                }
            }
                break;
            case VAR_DATE_TIME: {
                paramType = SQL_C_TIMESTAMP;
                sqlType = SQL_TIMESTAMP;
                len = sizeof(TIMESTAMP_STRUCT);
                TIMESTAMP_STRUCT *t = (TIMESTAMP_STRUCT *) param->conversionBuffer();
                CDateTime dt = param->getDateTime();
                int16_t ms;
                buff = t;
                if (dt) {
                    dt.decodeDate((int16_t *) &t->year, (int16_t *) &t->month, (int16_t *) &t->day);
                    dt.decodeTime((int16_t *) &t->hour, (int16_t *) &t->minute, (int16_t *) &t->second, &ms);
                    t->fraction = 0;
                } else {
                    paramType = SQL_C_CHAR;
                    sqlType = SQL_CHAR;
                    *(char *) buff = 0;
                }
            }
                break;
            default:
                query->logAndThrow("CODBCDatabase::queryBindParameters",
                        "Unknown type of parameter " + int2string(paramNumber));
            }
            SQLLEN* cbValue = NULL;
            if (param->isNull()) {
                cbValue = &cbNullValue;
                len = 0;
            }

            rc = SQLBindParameter((SQLHSTMT) query->statement(), paramNumber, parameterMode, paramType, sqlType, len, scale,
                    buff, SQLINTEGER(len), cbValue);
            if (rc != 0) {
                param->m_binding.reset();
                query->logAndThrow("CODBCDatabase::queryBindParameters", "Can't bind parameter " + int2string(paramNumber));
            }
        }
    }
}

void CODBCDatabase::ODBCtypeToCType(int32_t odbcType, int32_t &cType, CVariantType& dataType)
{
    switch (odbcType)
    {
    case SQL_BIGINT:
    case SQL_TINYINT:
    case SQL_SMALLINT:
    case SQL_INTEGER:
        cType = SQL_C_SLONG;
        dataType = VAR_INT;
        return;

    case SQL_NUMERIC:
    case SQL_REAL:
    case SQL_DECIMAL:
    case SQL_DOUBLE:
    case SQL_FLOAT:
        cType = SQL_C_DOUBLE;
        dataType = VAR_FLOAT;
        return;

    case SQL_LONGVARCHAR:
    case SQL_VARCHAR:
    case SQL_CHAR:
        cType = SQL_C_CHAR;
        dataType = VAR_STRING;
        return;

    case SQL_DATE: // ODBC 2.0 only
    case SQL_TYPE_DATE: // ODBC 3.0 only
        cType = SQL_C_TIMESTAMP;
        dataType = VAR_DATE;
        return;

    case SQL_TIME:
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIME:
    case SQL_TYPE_TIMESTAMP:
        if (odbcType == SQL_TYPE_DATE) {
            cType = SQL_C_TIMESTAMP;
            dataType = VAR_DATE;
        } else {
            cType = SQL_C_TIMESTAMP;
            dataType = VAR_DATE_TIME;
        }
        return;

    case SQL_BINARY:
    case SQL_LONGVARBINARY:
    case SQL_VARBINARY:
        cType = SQL_C_BINARY;
        dataType = VAR_BUFFER;
        return;

    case SQL_BIT:
        cType = SQL_C_BIT;
        dataType = VAR_BOOL;
        return;
    }
    cType = 0;
    dataType = VAR_NONE;
}

void CODBCDatabase::queryOpen(CQuery *query)
{
    if (!active())
        open();

    if (query->active())
        return;

    if (!query->statement())
        queryAllocStmt(query);

    try {
        queryBindParameters(query);
    } catch (exception& e) {
        string error = queryError(query);
        query->logAndThrow("CODBCDatabase::queryOpen", string(e.what()) + "\n" + error);
    }

    if (query->autoPrepare() && !query->prepared()) {
        queryPrepare(query);
        querySetPrepared(query, true);
    }

    queryExecute(query);

    int16_t count = queryColCount(query);

    if (count < 1) {
        queryCloseStmt(query);
        return;
    } else {
        querySetActive(query, true);

        if (query->fieldCount() == 0) {
            // Reading the column attributes
            char columnName[256];
            int32_t columnType;
            int32_t columnLength;
            int32_t columnScale;
            int32_t cType;
            CVariantType dataType;
            for (int16_t column = 1; column <= count; column++) {
                queryColAttributes(query, column, SQL_COLUMN_NAME, columnName, 255);
                queryColAttributes(query, column, SQL_COLUMN_TYPE, columnType);
                queryColAttributes(query, column, SQL_COLUMN_LENGTH, columnLength);
                queryColAttributes(query, column, SQL_COLUMN_SCALE, columnScale);
                ODBCtypeToCType(columnType, cType, dataType);
                if (dataType == VAR_STRING && columnLength > 65535)
                    dataType = VAR_TEXT;
                if (columnName[0] == 0)
                    sprintf(columnName, "column%02i", column);
                if (columnLength > FETCH_BUFFER_SIZE)
                    columnLength = FETCH_BUFFER_SIZE;

                if (dataType == VAR_FLOAT && (columnScale < 0 || columnScale > 20))
                    columnScale = 0;

                CField *field = new CODBCField(columnName, column, cType, dataType, (int) columnLength, (int) columnScale);
                query->fields().push_back(field);
            }
        }
    }
    querySetEof(query, false);
    queryFetch(query);
}

static uint32_t trimField(char *s, uint32_t sz)
{
    register char *p = s + sz;
    char ch = s[0];
    s[0] = '!';
    while (*(--p) == ' ') {
    }
    *(++p) = 0;
    if (ch == ' ' && s[1] == 0) {
        s[0] = 0;
        return 0;
    }
    s[0] = ch;
    return uint32_t(p - s);
}

void CODBCDatabase::queryFetch(CQuery *query)
{
    if (!query->active())
        query->logAndThrow("CODBCDatabase::queryFetch", "Dataset isn't open");

    SQLHSTMT statement = (SQLHSTMT) query->statement();

    CSynchronizedCode lock(m_connect);

    int rc = SQLFetch(statement);

    if (!successful(rc)) {
        if (rc < 0) {
            string error = queryError(query);
            query->logText(error);
            throw CDatabaseException(error, __FILE__, __LINE__, query->sql().c_str());
        } else {
            querySetEof(query, rc == SQL_NO_DATA);
            return;
        }
    }

    uint32_t fieldCount = query->fieldCount();
    SQLLEN dataLength = 0;

    if (!fieldCount)
        return;

    CODBCField *field = 0;
    for (unsigned column = 0; column < fieldCount;)
        try {
            field = (CODBCField *) &(*query)[column];
            const int16_t fieldType = (int16_t) field->fieldType();
            uint32_t readSize = field->bufferSize();
            char* buffer = field->getData();

            column++;

            switch (fieldType)
            {

            case SQL_C_SLONG:
            case SQL_C_DOUBLE:
                rc = SQLGetData(statement, column, fieldType, buffer, 0, &dataLength);
                break;

            case SQL_C_TIMESTAMP: {
                TIMESTAMP_STRUCT t;
                rc = SQLGetData(statement, column, fieldType, &t, 0, &dataLength);
                if (dataLength > 0) {
                    CDateTime dt(t.year, t.month, t.day, t.hour, t.minute, t.second);
                    if (field->dataType() == VAR_DATE)
                        field->setDate(dt);
                    else
                        field->setDateTime(dt);
                }
                break;
            }

            case SQL_C_BINARY:
            case SQL_C_CHAR:
                buffer = (char *) field->getBuffer();
                rc = SQLGetData(statement, column, fieldType, buffer, SQLINTEGER(readSize), &dataLength);
                if (dataLength > SQLINTEGER(readSize)) { // continue to fetch BLOB data
                    field->checkSize(dataLength + 1);
                    buffer = (char *) field->getBuffer();
                    char *offset = buffer + readSize - 1;
                    readSize = dataLength - readSize + 2;
                    rc = SQLGetData(statement, column, fieldType, offset, SQLINTEGER(readSize), NULL);
                }
                break;

            case SQL_BIT:
                rc = SQLGetData(statement, column, fieldType, buffer, 1, &dataLength);
                break;

            default:
                dataLength = 0;
                break;
            }

            if (fieldType == SQL_C_CHAR && dataLength > 0)
                dataLength = (SQLINTEGER) trimField(buffer, dataLength);

            if (dataLength <= 0) {
                field->setNull();
            } else {
                field->dataSize(dataLength);
            }
        } catch (exception& e) {
            query->logAndThrow("CODBCDatabase::queryFetch", "Can't read field " + field->fieldName() + "\n" + string(e.what()));
        }
}

string CODBCDatabase::driverDescription() const
{
    if (m_connect)
        return m_connect->driverDescription();
    else
        return "";
}

void CODBCDatabase::objectList(CDbObjectType objectType, CStrings& objects) throw (CDatabaseException)
{
    CSynchronizedCode lock(m_connect);

    SQLHSTMT stmt = 0;
    try {
        SQLRETURN rc;
        SQLHDBC hdb = (SQLHDBC) handle();
        if (SQLAllocStmt(hdb, &stmt) != SQL_SUCCESS)
            throw CDatabaseException("CODBCDatabase::queryAllocStmt");

        switch (objectType)
        {
        case DOT_TABLES:
            if (SQLTables(stmt, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR*) "TABLE", SQL_NTS) != SQL_SUCCESS)
                throw CDatabaseException("SQLTables");
            break;

        case DOT_VIEWS:
            if (SQLTables(stmt, NULL, 0, NULL, 0, NULL, 0, (SQLCHAR*) "VIEW", SQL_NTS) != SQL_SUCCESS)
                throw CDatabaseException("SQLTables");
            break;

        case DOT_PROCEDURES:
            rc = SQLProcedures(stmt, NULL, 0, (SQLCHAR*) "", SQL_NTS, (SQLCHAR*) "%", SQL_NTS);
            if (rc != SQL_SUCCESS)
                throw CDatabaseException("SQLProcedures");
            break;
        }

        SQLCHAR objectSchema[256];
        SQLCHAR objectName[256];
        SQLLEN cbObjectSchema;
        SQLLEN cbObjectName;
        if (SQLBindCol(stmt, 2, SQL_C_CHAR, objectSchema, sizeof(objectSchema), &cbObjectSchema) != SQL_SUCCESS)
            throw CDatabaseException("SQLBindCol");
        if (SQLBindCol(stmt, 3, SQL_C_CHAR, objectName, sizeof(objectName), &cbObjectName) != SQL_SUCCESS)
            throw CDatabaseException("SQLBindCol");

        while (true) {
            objectSchema[0] = 0;
            objectName[0] = 0;
            int rc = SQLFetch(stmt);
            if (rc == SQL_NO_DATA_FOUND)
                break;
            if (!successful(rc))
                throw CDatabaseException("SQLFetch");
            objects.push_back(string((char*) objectSchema) + "." + string((char*) objectName));
        }

        SQLFreeStmt(stmt, SQL_DROP);
    } catch (exception& e) {
        string error;
        if (stmt) {
            error = queryError(stmt);
            SQLFreeStmt(stmt, SQL_DROP);
        }
        logAndThrow(e.what(), error);
    } catch (...) {
        logAndThrow("CODBCDatabase::objectList", "Unknown error");
    }
}

void* odbc_createDriverInstance(const char* connectionString)
{
    CODBCDatabase* database = new CODBCDatabase(connectionString);
    return database;
}

void odbc_destroyDriverInstance(void* driverInstance)
{
    delete (CODBCDatabase*) driverInstance;
}
