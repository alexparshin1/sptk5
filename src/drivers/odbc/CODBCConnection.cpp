/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CODBCConnection.cpp - description                      ║
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

#include <iomanip>
#include <sptk5/RegularExpression.h>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/ODBCConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

namespace sptk
{

class CODBCField : public DatabaseField
{
    friend class ODBCConnection;

protected:
    char* getData()
    {
        return (char*) &m_data;
    }

public:
    CODBCField(const string& fieldName, int fieldColumn, int fieldType, VariantType dataType, int fieldLength, int fieldScale)
    : DatabaseField(fieldName, fieldColumn, fieldType, dataType, fieldLength, fieldScale)
    {
    }
};
} // namespace sptk

ODBCConnection::ODBCConnection(const String& connectionString)
: PoolDatabaseConnection(connectionString)
{
    m_connect = new ODBCConnectionBase;
    m_connType = DCT_GENERIC_ODBC;
}

ODBCConnection::~ODBCConnection()
{
    try {
        if (m_inTransaction && active())
            rollbackTransaction();
        close();
        while (!m_queryList.empty()) {
            try {
                auto query = (Query*) m_queryList[0];
                query->disconnect();
            } catch (...) {
            }
        }
        m_queryList.clear();
        delete m_connect;
    } catch (...) {
    }
}

String ODBCConnection::nativeConnectionString() const
{
    stringstream connectionString;
    connectionString << "DSN=" << m_connString.hostName();
    if (!m_connString.userName().empty())
        connectionString << ";UID=" << m_connString.userName();
    if (!m_connString.password().empty())
        connectionString << ";PWD=" << m_connString.password();
    if (!m_connString.databaseName().empty())
        connectionString << ";DATABASE=" << m_connString.databaseName();
    //connectionString += ";ClientCharset=UTF-8;ServerCharset=UCS2";
    return connectionString.str();
}

void ODBCConnection::_openDatabase(const String& newConnectionString)
{
    if (!active()) {
        m_inTransaction = false;
        if (!newConnectionString.empty())
            m_connString = DatabaseConnectionString(newConnectionString);

        string finalConnectionString;
        m_connect->connect(nativeConnectionString(), finalConnectionString, false);
        if (m_connect->driverDescription().find("Microsoft SQL Server") != string::npos)
            m_connType = DCT_MSSQL_ODBC;
    }
}

void ODBCConnection::closeDatabase()
{
    for (auto query: m_queryList) {
        try {
            queryFreeStmt(query);
        } catch (...) {
        }
    }
    m_connect->freeConnect();
}

void* ODBCConnection::handle() const
{
    return m_connect->handle();
}

bool ODBCConnection::active() const
{
    return m_connect->isConnected();
}

void ODBCConnection::driverBeginTransaction()
{
    if (!m_connect->isConnected())
        open();

    if (m_inTransaction)
        logAndThrow("CODBCConnection::driverBeginTransaction", "Transaction already started.");

    m_connect->beginTransaction();
    m_inTransaction = true;
}

void ODBCConnection::driverEndTransaction(bool commit)
{
    if (!m_inTransaction) {
        if (commit)
            logAndThrow("CODBCConnection::driverEndTransaction", "Can't commit - transaction isn't started.");
        else
            logAndThrow("CODBCConnection::driverEndTransaction", "Can't rollback - transaction isn't started.");
    }

    if (commit)
        m_connect->commit();
    else
        m_connect->rollback();

    m_connect->setConnectOption(SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------
static inline bool successful(int ret)
{
    return ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO;
}

String ODBCConnection::connectString() const
{
    return m_connect->connectString();
}

String ODBCConnection::queryError(SQLHSTMT stmt) const
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
        rc = SQLError(SQL_NULL_HENV, (SQLHDBC) handle(), nullptr, errorState, &nativeError, errorDescription,
                      sizeof(errorDescription), &pcnmsg);
        if (rc != SQL_SUCCESS && *errorDescription != char(0))
            strncpy((char*) errorDescription, "Unknown error", sizeof(errorDescription));
    }

    return removeDriverIdentification((char*) errorDescription);
}

String ODBCConnection::queryError(const Query* query) const
{
    return queryError(query->statement());
}

void ODBCConnection::queryAllocStmt(Query* query)
{
    lock_guard<mutex> lock(m_connect->m_mutex);

    auto stmt = (SQLHSTMT) query->statement();
    if (stmt != SQL_NULL_HSTMT)
        SQLFreeStmt(stmt, SQL_DROP);

    auto hdb = (SQLHDBC) handle();
    int rc = SQLAllocStmt(hdb, &stmt);

    if (rc != SQL_SUCCESS) {
        String error = queryError(query);
        querySetStmt(query, SQL_NULL_HSTMT);
        logAndThrow("CODBCConnection::queryAllocStmt", error);
    }

    querySetStmt(query, stmt);
}

void ODBCConnection::queryFreeStmt(Query* query)
{
    lock_guard<mutex> lock(m_connect->m_mutex);

    SQLFreeStmt(query->statement(), SQL_DROP);
    querySetStmt(query, SQL_NULL_HSTMT);
    querySetPrepared(query, false);
}

void ODBCConnection::queryCloseStmt(Query* query)
{
    lock_guard<mutex> lock(m_connect->m_mutex);

    SQLFreeStmt(query->statement(), SQL_CLOSE);
}

void ODBCConnection::queryPrepare(Query* query)
{
    lock_guard<mutex> lock(m_connect->m_mutex);

    query->fields().clear();

    if (!successful(SQLPrepare(query->statement(), (SQLCHAR*) query->sql().c_str(), SQL_NTS)))
        THROW_QUERY_ERROR(query, queryError(query));
}

void ODBCConnection::queryUnprepare(Query* query)
{
    queryFreeStmt(query);
    query->fields().clear();
}

void ODBCConnection::queryExecute(Query* query)
{
    lock_guard<mutex> lock(m_connect->m_mutex);

    int rc = 0;
    if (query->prepared())
        rc = SQLExecute(query->statement());
    else
        rc = SQLExecDirect(query->statement(), (SQLCHAR*) query->sql().c_str(), SQL_NTS);

    if (!successful(rc)) {
        SQLCHAR state[16];
        SQLCHAR text[1024];
        SQLINTEGER nativeError = 0;
        SQLINTEGER recordCount = 0;
        SQLSMALLINT textLength = 0;

        rc = SQLGetDiagField(SQL_HANDLE_STMT, query->statement(), 1, SQL_DIAG_NUMBER, &recordCount, sizeof(recordCount),
                             &textLength);
        if (successful(rc)) {
            Strings errors;
            for (SQLSMALLINT recordNumber = 1; recordNumber <= recordCount; recordNumber++) {
                rc = SQLGetDiagRec(SQL_HANDLE_STMT, query->statement(), recordNumber, state, &nativeError, text, sizeof(text),
                                   &textLength);
                if (!successful(rc))
                    break;
                errors.push_back(removeDriverIdentification((const char*) text));
            }
            THROW_QUERY_ERROR(query, errors.asString("; "));
        }
    }

    if (!successful(rc))
        THROW_QUERY_ERROR(query, queryError(query));
}

int ODBCConnection::queryColCount(Query* query)
{
    lock_guard<mutex> lock(m_connect->m_mutex);

    int16_t count = 0;
    if (!successful(SQLNumResultCols(query->statement(), &count)))
        THROW_QUERY_ERROR(query, queryError(query));

    return count;
}

void ODBCConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
    lock_guard<mutex> lock(m_connect->m_mutex);
    SQLLEN result;

    if (!successful(SQLColAttributes(query->statement(), (SQLUSMALLINT) column, (SQLUSMALLINT) descType, nullptr, 0, nullptr, &result)))
        THROW_QUERY_ERROR(query, queryError(query));
    value = (int32_t) result;
}

void ODBCConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, LPSTR buff, int len)
{
    int16_t available;
    if (buff == nullptr || len <= 0)
        THROW_QUERY_ERROR(query, "Invalid buffer or buffer len");

    lock_guard<mutex> lock(m_connect->m_mutex);

    if (!successful(SQLColAttributes(query->statement(), (SQLUSMALLINT) column, (SQLUSMALLINT) descType, buff, (int16_t) len, &available, nullptr)))
        THROW_QUERY_ERROR(query, queryError(query));
}

void ODBCConnection::queryBindParameters(Query* query)
{
    static SQLLEN cbNullValue = SQL_NULL_DATA;

    lock_guard<mutex> lock(m_connect->m_mutex);
    int rc;

    for (uint32_t i = 0; i < query->paramCount(); i++) {
        QueryParameter* param = &query->param(i);
        VariantType ptype = param->dataType();
        auto cblen = (SQLLEN&) param->callbackLength();
        for (unsigned j = 0; j < param->bindCount(); j++) {

            int16_t paramType = 0, sqlType = 0, scale = 0;
            void* buff = param->dataBuffer();
            long len = 0;
            auto paramNumber = int16_t(param->bindIndex(j) + 1);

            int16_t parameterMode = SQL_PARAM_INPUT;
            switch (ptype) {
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
                    buff = (void*) param->getString();
                    len = (long) param->dataSize();
                    paramType = SQL_C_CHAR;
                    sqlType = SQL_WVARCHAR;
                    break;
                case VAR_TEXT:
                    buff = (void*) param->getString();
                    len = (long) param->dataSize();
                    paramType = SQL_C_CHAR;
                    sqlType = SQL_WLONGVARCHAR;
                    break;
                case VAR_BUFFER:
                    buff = (void*) param->getString();
                    len = (long) param->dataSize();
                    cblen = len;
                    rc = SQLBindParameter((SQLHSTMT) query->statement(), (SQLUSMALLINT) paramNumber, parameterMode, SQL_C_BINARY,
                                          SQL_LONGVARBINARY, (SQLULEN) len, scale, buff, SQLINTEGER(len), &cblen);
                    if (rc != 0) {
                        param->m_binding.reset(false);
                        THROW_QUERY_ERROR(query, "Can't bind parameter " << paramNumber);
                    }
                    continue;

                case VAR_DATE: {
                    paramType = SQL_C_TIMESTAMP;
                    sqlType = SQL_TIMESTAMP;
                    len = sizeof(TIMESTAMP_STRUCT);
                    auto t = (TIMESTAMP_STRUCT*) param->conversionBuffer();
                    DateTime dt = param->getDateTime();
                    buff = t;
                    if (!dt.zero()) {
                        short wday, yday;
                        dt.decodeDate(&t->year, (int16_t*) &t->month, (int16_t*) &t->day, &wday, &yday);
                        t->hour = t->minute = t->second = 0;
                        t->fraction = 0;
                    } else {
                        paramType = SQL_C_CHAR;
                        sqlType = SQL_CHAR;
                        *(char*) buff = 0;
                    }
                }
                    break;
                case VAR_DATE_TIME: {
                    paramType = SQL_C_TIMESTAMP;
                    sqlType = SQL_TIMESTAMP;
                    len = sizeof(TIMESTAMP_STRUCT);
                    auto t = (TIMESTAMP_STRUCT*) param->conversionBuffer();
                    DateTime dt = param->getDateTime();
                    int16_t ms;
                    buff = t;
                    if (!dt.zero()) {
                        short wday, yday;
                        dt.decodeDate(&t->year, (int16_t*) &t->month, (int16_t*) &t->day, &wday, &yday);
                        dt.decodeTime((int16_t*) &t->hour, (int16_t*) &t->minute, (int16_t*) &t->second, &ms);
                        t->fraction = 0;
                    } else {
                        paramType = SQL_C_CHAR;
                        sqlType = SQL_CHAR;
                        *(char*) buff = 0;
                    }
                }
                    break;
                default:
                    THROW_QUERY_ERROR(query, "Unknown type of parameter '" << param->name() << "'");
            }
            SQLLEN* cbValue = nullptr;
            if (param->isNull()) {
                cbValue = &cbNullValue;
                len = 0;
            }

            rc = SQLBindParameter((SQLHSTMT) query->statement(), (SQLUSMALLINT) paramNumber, parameterMode, paramType, sqlType, (SQLULEN) len, scale,
                                  buff, SQLINTEGER(len), cbValue);
            if (rc != 0) {
                param->m_binding.reset(false);
                THROW_QUERY_ERROR(query, "Can't bind parameter " << paramNumber);
            }
        }
    }
}

void ODBCConnection::ODBCtypeToCType(int32_t odbcType, int32_t& cType, VariantType& dataType)
{
    switch (odbcType) {
        case SQL_BIGINT:
        case SQL_TINYINT:
        case SQL_SMALLINT:
        case SQL_INTEGER:
            cType = SQL_C_SLONG;
            dataType = VAR_INT;
            break;

        case SQL_NUMERIC:
        case SQL_REAL:
        case SQL_DECIMAL:
        case SQL_DOUBLE:
        case SQL_FLOAT:
            cType = SQL_C_DOUBLE;
            dataType = VAR_FLOAT;
            break;

        case SQL_LONGVARCHAR:
        case SQL_VARCHAR:
        case SQL_CHAR:
            cType = SQL_C_CHAR;
            dataType = VAR_STRING;
            break;

        case SQL_DATE: // ODBC 2.0 only
        case SQL_TYPE_DATE: // ODBC 3.0 only
            cType = SQL_C_TIMESTAMP;
            dataType = VAR_DATE;
            break;

        case SQL_TIME:
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIME:
        case SQL_TYPE_TIMESTAMP:
            cType = SQL_C_TIMESTAMP;
            dataType = VAR_DATE_TIME;
            break;

        case SQL_BINARY:
        case SQL_LONGVARBINARY:
        case SQL_VARBINARY:
            cType = SQL_C_BINARY;
            dataType = VAR_BUFFER;
            break;

        case SQL_BIT:
            cType = SQL_C_BIT;
            dataType = VAR_BOOL;
            break;

        default:
            //cType = 0;
            //dataType = VAR_NONE;
            cType = SQL_C_CHAR;
            dataType = VAR_STRING;
            break;
    }
}

void ODBCConnection::queryOpen(Query* query)
{
    if (!active())
        open();

    if (query->active())
        return;

    if (query->statement() == nullptr)
        queryAllocStmt(query);

    try {
        queryBindParameters(query);
    }
    catch (const DatabaseException& e) {
        throw;
    }
    catch (...) {
        THROW_QUERY_ERROR(query, queryError(query));
    }

    if (query->autoPrepare() && !query->prepared()) {
        queryPrepare(query);
        querySetPrepared(query, true);
    }

    queryExecute(query);

    int count = queryColCount(query);

    if (count < 1) {
        queryCloseStmt(query);
        return;
    }

    querySetActive(query, true);

    if (query->fieldCount() == 0) {
        // Reading the column attributes
        char columnName[256];
        int32_t columnType;
        int32_t columnLength;
        int32_t columnScale;
        int32_t cType;
        VariantType dataType;

        stringstream columnNameStr;
        columnNameStr.fill('0');

        for (int16_t column = 1; column <= count; column++) {
            queryColAttributes(query, column, SQL_COLUMN_NAME, columnName, 255);
            queryColAttributes(query, column, SQL_COLUMN_TYPE, columnType);
            queryColAttributes(query, column, SQL_COLUMN_LENGTH, columnLength);
            queryColAttributes(query, column, SQL_COLUMN_SCALE, columnScale);
            ODBCtypeToCType(columnType, cType, dataType);
            if (dataType == VAR_STRING && columnLength > 65535)
                dataType = VAR_TEXT;
            if (columnName[0] != 0)
                columnNameStr.str(columnName);
            else {
                columnNameStr.str("column");
                columnNameStr << setw(2) << column;
            }
            if (columnLength > FETCH_BUFFER_SIZE)
                columnLength = FETCH_BUFFER_SIZE;

            if (dataType == VAR_FLOAT && (columnScale < 0 || columnScale > 20))
                columnScale = 0;

            Field* field = new CODBCField(columnNameStr.str(), column, cType, dataType, (int) columnLength, (int) columnScale);
            query->fields().push_back(field);
        }
    }

    querySetEof(query, false);
    queryFetch(query);
}

static uint32_t trimField(char* s, uint32_t sz)
{
    char* p = s + sz;
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

void ODBCConnection::queryFetch(Query* query)
{
    if (!query->active())
        THROW_QUERY_ERROR(query, "Dataset isn't open");

    auto statement = (SQLHSTMT) query->statement();

    lock_guard<mutex> lock(m_connect->m_mutex);

    int rc = SQLFetch(statement);

    if (!successful(rc)) {
        if (rc < 0) {
            THROW_QUERY_ERROR(query, queryError(query));
        } else {
            querySetEof(query, rc == SQL_NO_DATA);
            return;
        }
    }

    uint32_t fieldCount = query->fieldCount();
    SQLLEN dataLength = 0;

    if (fieldCount == 0)
        return;

    CODBCField* field = nullptr;
    for (unsigned column = 0; column < fieldCount;)
        try {
            field = (CODBCField*) &(*query)[column];
            auto fieldType = (int16_t) field->fieldType();
            size_t readSize = field->bufferSize();
            char* buffer = field->getData();

            column++;

            switch (fieldType) {

                case SQL_C_SLONG:
                case SQL_C_DOUBLE:
                    rc = SQLGetData(statement, (SQLUSMALLINT) column, fieldType, buffer, 0, &dataLength);
                    break;

                case SQL_C_TIMESTAMP: {
                    TIMESTAMP_STRUCT t = {};
                    rc = SQLGetData(statement, (SQLUSMALLINT) column, fieldType, &t, 0, &dataLength);
                    if (dataLength > 0) {
                        DateTime dt(t.year, t.month, t.day, t.hour, t.minute, t.second);
                        if (field->dataType() == VAR_DATE)
                            field->setDate(dt);
                        else
                            field->setDateTime(dt);
                    }
                    break;
                }

                case SQL_C_BINARY:
                case SQL_C_CHAR:
                    buffer = (char*) field->getBuffer();
                    rc = SQLGetData(statement, (SQLUSMALLINT) column, fieldType, buffer, SQLINTEGER(readSize), &dataLength);
                    if (dataLength > SQLINTEGER(readSize)) { // continue to fetch BLOB data
                        field->checkSize(uint32_t(dataLength + 1));
                        buffer = (char*) field->getBuffer();
                        char* offset = buffer + readSize - 1;
                        readSize = dataLength - readSize + 2;
                        rc = SQLGetData(statement, (SQLUSMALLINT) column, fieldType, offset, SQLINTEGER(readSize), nullptr);
                    }
                    break;

                case SQL_BIT:
                    rc = SQLGetData(statement, (SQLUSMALLINT) column, fieldType, buffer, 1, &dataLength);
                    break;

                default:
                    dataLength = 0;
                    break;
            }

            if (fieldType == SQL_C_CHAR && dataLength > 0)
                dataLength = (SQLINTEGER) trimField(buffer, (uint32_t) dataLength);

            if (dataLength <= 0)
                field->setNull(VAR_NONE);
            else
                field->dataSize((size_t)dataLength);
        } catch (exception& e) {
            query->throwError("CODBCConnection::queryFetch",
                              "Can't read field " + field->fieldName() + "\n" + string(e.what()));
        }
}

String ODBCConnection::driverDescription() const
{
    if (m_connect != nullptr)
        return m_connect->driverDescription();
    return "";
}

void ODBCConnection::listDataSources(Strings& dsns)
{
    dsns.clear();
#define MAX_BUF 1024
    SQLCHAR     datasrc[MAX_BUF] = {0};
    SQLCHAR     descrip[MAX_BUF] = {0};
    SQLSMALLINT rdsrc = 0, rdesc = 0;
    SQLRETURN ret;

    SQLHENV hEnv = ODBCConnectionBase::getEnvironment().handle();
    bool offline = hEnv == nullptr;
    if (offline) {
        if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HENV, &hEnv) != SQL_SUCCESS)
            throw DatabaseException("CODBCConnection::SQLAllocHandle");
        if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER))
            throw DatabaseException("CODBCConnection::SQLSetEnvAttr");
    }

    SQLUSMALLINT direction =
            SQL_FETCH_FIRST;
            // SQL_FETCH_FIRST_USER;
            //SQL_FETCH_FIRST_SYSTEM;
    while (true)
    {
        ret = SQLDataSources(
                hEnv, direction,
                datasrc, MAX_BUF, &rdsrc,
                descrip, MAX_BUF, &rdesc);
        if (ret == SQL_NO_DATA)
            break;
        direction = SQL_FETCH_NEXT;
        //CharToOem(datasrc, datasrc);
        //CharToOem(descrip, descrip);
        dsns.push_back(string((char*) datasrc) + " (" +  string((char*) descrip) + ")");
    }

    if (offline)
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void ODBCConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    lock_guard<mutex> lock(m_connect->m_mutex);

    if (objectType == DOT_DATABASES) {
        listDataSources(objects);
        return;
    }

    SQLHSTMT stmt = nullptr;
    try {
        SQLRETURN rc;
        auto hdb = (SQLHDBC) handle();
        if (SQLAllocStmt(hdb, &stmt) != SQL_SUCCESS)
            throw DatabaseException("CODBCConnection::SQLAllocStmt");

        switch (objectType) {
            case DOT_TABLES:
                if (SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLCHAR*) "TABLE", SQL_NTS) != SQL_SUCCESS)
                    throw DatabaseException("SQLTables");
                break;

            case DOT_VIEWS:
                if (SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLCHAR*) "VIEW", SQL_NTS) != SQL_SUCCESS)
                    throw DatabaseException("SQLTables");
                break;

            case DOT_PROCEDURES:
                rc = SQLProcedures(stmt, nullptr, 0, (SQLCHAR*) "", SQL_NTS, (SQLCHAR*) "%", SQL_NTS);
                if (rc != SQL_SUCCESS)
                    throw DatabaseException("SQLProcedures");
                break;

            case DOT_FUNCTIONS:
                // not yet supported
                break;

            case DOT_DATABASES:
                // handled earlier
                break;
        }

        SQLCHAR objectSchema[256] = {};
        SQLCHAR objectName[256] = {};
        SQLLEN cbObjectSchema;
        SQLLEN cbObjectName;
        if (SQLBindCol(stmt, 2, SQL_C_CHAR, objectSchema, sizeof(objectSchema), &cbObjectSchema) != SQL_SUCCESS)
            throw DatabaseException("SQLBindCol");
        if (SQLBindCol(stmt, 3, SQL_C_CHAR, objectName, sizeof(objectName), &cbObjectName) != SQL_SUCCESS)
            throw DatabaseException("SQLBindCol");

        while (true) {
            objectSchema[0] = 0;
            objectName[0] = 0;
            rc = SQLFetch(stmt);
            if (rc == SQL_NO_DATA_FOUND)
                break;
            if (!successful(rc))
                throw DatabaseException("SQLFetch");
            String objectNameStr = String((char*) objectName).replace(";0$", "");
            objects.push_back(string((char*) objectSchema) + "." + objectNameStr);
        }

        SQLFreeStmt(stmt, SQL_DROP);
    } catch (exception& e) {
        String error;
        if (stmt != nullptr) {
            error = queryError(stmt);
            SQLFreeStmt(stmt, SQL_DROP);
        }
        logAndThrow(e.what(), error);
    } catch (...) {
        logAndThrow("CODBCConnection::objectList", "Unknown error");
    }
}

void ODBCConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    RegularExpression   matchStatementEnd("(;\\s*)$");
    RegularExpression   matchRoutineStart("^CREATE\\s+FUNCTION", "i");
    RegularExpression   matchGo("^\\s*GO\\s*$", "i");
    RegularExpression   matchCommentRow("^\\s*--");

    Strings statements, matches;
    string statement;
    bool routineStarted = false;
    for (String row: sqlBatch) {
        row = trim(row);
        if (row.empty() || matchCommentRow.matches(row))
            continue;

        if (!routineStarted) {
            row = trim(row);
            if (row.empty() || row.startsWith("--"))
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
            //cout << "[ " << statement << " ]" << endl;
            query.exec();
        }
        catch (const exception& e) {
            stringstream error;
            error << e.what() << ". Query: " << stmt;
            if (errors != nullptr)
                errors->push_back(error.str());
            else
                throw DatabaseException(error.str());
        }
    }
}

void* odbc_create_connection(const char* connectionString)
{
    ODBCConnection* connection = new ODBCConnection(connectionString);
    return connection;
}

void odbc_destroy_connection(void* connection)
{
    delete (ODBCConnection*) connection;
}
