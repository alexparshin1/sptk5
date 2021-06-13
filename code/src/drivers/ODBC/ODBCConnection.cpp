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
#include <iomanip>
#include <sptk5/RegularExpression.h>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/ODBCConnection.h>
#include <sptk5/db/Query.h>
#include <array>

constexpr size_t MAX_BUF = 1024;
constexpr size_t MAX_NAME_LEN = 256;
constexpr size_t MAX_ERROR_LEN = 1024;

using namespace std;
using namespace sptk;

namespace sptk
{

class CODBCField : public DatabaseField
{
    friend class ODBCConnection;

public:
    CODBCField(const string& fieldName, int fieldColumn, int fieldType, VariantType dataType, int fieldLength, int fieldScale)
    : DatabaseField(fieldName, fieldColumn, fieldType, dataType, fieldLength, fieldScale)
    {
    }

protected:
    char* getData()
    {
        return m_data.getData();
    }
};
} // namespace sptk

ODBCConnection::ODBCConnection(const String& connectionString)
: PoolDatabaseConnection(connectionString, DCT_GENERIC_ODBC)
{
}

ODBCConnection::~ODBCConnection()
{
    try {
        if (getInTransaction() && ODBCConnection::active())
            rollbackTransaction();
        close();
        delete m_connect;
    } catch (const Exception& e) {
        CERR(e.what() << endl)
    }
}

String ODBCConnection::nativeConnectionString() const
{
    const DatabaseConnectionString& connString = connectionString();
    stringstream connectionString;
    connectionString << "DSN=" << connString.hostName();
    if (!connString.userName().empty())
        connectionString << ";UID=" << connString.userName();
    if (!connString.password().empty())
        connectionString << ";PWD=" << connString.password();
    if (!connString.databaseName().empty())
        connectionString << ";DATABASE=" << connString.databaseName();
    return connectionString.str();
}

void ODBCConnection::_openDatabase(const String& newConnectionString)
{
    if (!active()) {
        setInTransaction(false);
        if (!newConnectionString.empty())
            connectionString(DatabaseConnectionString(newConnectionString));

        String finalConnectionString;
        m_connect->connect(nativeConnectionString(), finalConnectionString, false);
        if (m_connect->driverDescription().find("Microsoft SQL Server") != string::npos)
            connectionType(DCT_MSSQL_ODBC);
    }
}

void ODBCConnection::closeDatabase()
{
    disconnectAllQueries();
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

    if (getInTransaction())
        logAndThrow("CODBCConnection::driverBeginTransaction", "Transaction already started.");

    m_connect->beginTransaction();
    setInTransaction(true);
}

void ODBCConnection::driverEndTransaction(bool commit)
{
    if (!getInTransaction()) {
        if (commit)
            logAndThrow("CODBCConnection::driverEndTransaction", "Can't commit - transaction isn't started.");
        else
            logAndThrow("CODBCConnection::driverEndTransaction", "Can't rollback - transaction isn't started.");
    }

    if (commit)
        m_connect->commit();
    else
        m_connect->rollback();

    m_connect->setConnectOption(SQL_ATTR_AUTOCOMMIT, (UDWORD) true);
    setInTransaction(false);
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
    array<SQLCHAR,SQL_MAX_MESSAGE_LENGTH> errorDescription = {};
    array<SQLCHAR,SQL_MAX_MESSAGE_LENGTH> errorState = {};

    SWORD pcnmsg = 0;
    SQLINTEGER nativeError = 0;

    String error;
    int rc = SQLError(SQL_NULL_HENV, handle(), stmt, errorState.data(), &nativeError, errorDescription.data(),
                      (SQLSMALLINT) errorDescription.size(), &pcnmsg);

    if (rc == SQL_SUCCESS) {
        error = (const char*) errorDescription.data();
    } else {
        rc = SQLError(SQL_NULL_HENV, handle(), nullptr, errorState.data(), &nativeError, errorDescription.data(),
                      (SQLSMALLINT) errorDescription.size(), &pcnmsg);
        if (rc == SQL_SUCCESS)
            error = (const char*) errorDescription.data();
    }

    if (error.empty())
        error = "Unknown error";

    return removeDriverIdentification(error.c_str());
}

String ODBCConnection::queryError(const Query* query) const
{
    return queryError(query->statement());
}

void ODBCConnection::queryAllocStmt(Query* query)
{
    lock_guard<mutex> lock(*m_connect);

    auto* stmt = query->statement();
    if (stmt != SQL_NULL_HSTMT)
        SQLFreeStmt(stmt, SQL_DROP);

    auto* hdb = handle();
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
    lock_guard<mutex> lock(*m_connect);

    SQLFreeStmt(query->statement(), SQL_DROP);
    querySetStmt(query, SQL_NULL_HSTMT);
    querySetPrepared(query, false);
}

void ODBCConnection::queryCloseStmt(Query* query)
{
    lock_guard<mutex> lock(*m_connect);

    SQLFreeStmt(query->statement(), SQL_CLOSE);
}

void ODBCConnection::queryPrepare(Query* query)
{
    lock_guard<mutex> lock(*m_connect);

    query->fields().clear();

    char* sql = query->sql().empty()? nullptr: &query->sql()[0];
    if (!successful(SQLPrepare(query->statement(), (SQLCHAR*) sql, SQL_NTS)))
        THROW_QUERY_ERROR(query, queryError(query))
}

void ODBCConnection::queryUnprepare(Query* query)
{
    queryFreeStmt(query);
    query->fields().clear();
}

void ODBCConnection::queryExecute(Query* query)
{
    lock_guard<mutex> lock(*m_connect);

    int rc = 0;
    if (query->prepared())
        rc = SQLExecute(query->statement());
    else {
        char* sql = query->sql().empty()? nullptr: &query->sql()[0];
        rc = SQLExecDirect(query->statement(), (SQLCHAR*) sql, SQL_NTS);
    }

    if (successful(rc))
        return;

    array<SQLCHAR,16> state = {};
    array<SQLCHAR,MAX_ERROR_LEN> text = {};
    SQLINTEGER nativeError = 0;
    SQLSMALLINT recordCount = 0;
    SQLSMALLINT textLength = 0;

    rc = SQLGetDiagField(SQL_HANDLE_STMT, query->statement(), 1, SQL_DIAG_NUMBER, &recordCount, sizeof(recordCount),
                         &textLength);
    if (successful(rc)) {
        Strings errors;
        for (SQLSMALLINT recordNumber = 1; recordNumber <= recordCount; ++recordNumber) {
            rc = SQLGetDiagRec(SQL_HANDLE_STMT, query->statement(), recordNumber, state.data(), &nativeError,
                               text.data(), (SQLSMALLINT) text.size(), &textLength);
            if (!successful(rc))
                break;
            errors.push_back(removeDriverIdentification((const char*) text.data()));
        }
        THROW_QUERY_ERROR(query, errors.join("; "))
    }

    if (!successful(rc))
        THROW_QUERY_ERROR(query, queryError(query))
}

int ODBCConnection::queryColCount(Query* query)
{
    lock_guard<mutex> lock(*m_connect);

    int16_t count = 0;
    if (!successful(SQLNumResultCols(query->statement(), &count)))
        THROW_QUERY_ERROR(query, queryError(query))

    return count;
}

void ODBCConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
    lock_guard<mutex> lock(*m_connect);
    SQLLEN result = 0;

    if (!successful(SQLColAttributes(query->statement(), (SQLUSMALLINT) column, (SQLUSMALLINT) descType, nullptr, 0, nullptr, &result)))
        THROW_QUERY_ERROR(query, queryError(query))
    value = (int32_t) result;
}

void ODBCConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, LPSTR buff, int len)
{
    int16_t available = 0;
    if (buff == nullptr || len <= 0)
        THROW_QUERY_ERROR(query, "Invalid buffer or buffer len")

    lock_guard<mutex> lock(*m_connect);

    if (!successful(SQLColAttributes(query->statement(), (SQLUSMALLINT) column, (SQLUSMALLINT) descType, buff, (int16_t) len, &available, nullptr)))
        THROW_QUERY_ERROR(query, queryError(query))
}

static bool dateTimeToTimestamp(TIMESTAMP_STRUCT* t, DateTime dt, bool dateOnly)
{
    if (!dt.zero()) {
        short wday = 0;
        short yday = 0;
        short ms = 0;
        dt.decodeDate(&t->year, (int16_t*) &t->month, (int16_t*) &t->day, &wday, &yday);
        if (dateOnly)
            t->hour = t->minute = t->second = 0;
        else
            dt.decodeTime((int16_t*) &t->hour, (int16_t*) &t->minute, (int16_t*) &t->second, &ms);
        t->fraction = 0;
        return true;
    }
    return false;
}

void ODBCConnection::queryBindParameter(const Query* query, QueryParameter* param)
{
    static SQLLEN cbNullValue = SQL_NULL_DATA;
    int rc = 0;

    VariantType ptype = param->dataType();
    for (unsigned j = 0; j < param->bindCount(); ++j) {
        int16_t paramType = 0;
        int16_t sqlType = 0;
        int16_t scale = 0;
        void* buff = (void*)&param->getInt64();
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
                paramType = SQL_C_BINARY;
                sqlType = SQL_LONGVARBINARY;
                buff = (void*) param->getString();
                len = (long) param->dataSize();
                break;

            case VAR_DATE:
                paramType = SQL_C_TIMESTAMP;
                sqlType = SQL_TIMESTAMP;
                len = sizeof(TIMESTAMP_STRUCT);
                buff = param->conversionBuffer();
                if (!dateTimeToTimestamp((TIMESTAMP_STRUCT*)param->conversionBuffer(), param->getDateTime(), true)) {
                    paramType = SQL_C_CHAR;
                    sqlType = SQL_CHAR;
                    *(char*) buff = 0;
                }
                break;

            case VAR_DATE_TIME:
                paramType = SQL_C_TIMESTAMP;
                sqlType = SQL_TIMESTAMP;
                len = sizeof(TIMESTAMP_STRUCT);
                buff = param->conversionBuffer();
                if (!dateTimeToTimestamp((TIMESTAMP_STRUCT*)param->conversionBuffer(), param->getDateTime(), false)) {
                    paramType = SQL_C_CHAR;
                    sqlType = SQL_CHAR;
                    *(char*) buff = 0;
                }
                break;

            default:
                THROW_QUERY_ERROR(query, "Unknown type of parameter '" << param->name() << "'")
        }
        SQLLEN* cbValue = nullptr;
        if (param->isNull()) {
            cbValue = &cbNullValue;
            len = 0;
        }

        rc = SQLBindParameter(query->statement(), (SQLUSMALLINT) paramNumber, parameterMode, paramType, sqlType, (SQLULEN) len, scale,
                              buff, SQLINTEGER(len), cbValue);
        if (rc != 0) {
            param->binding().reset(false);
            THROW_QUERY_ERROR(query, "Can't bind parameter " << paramNumber)
        }
    }
}

void ODBCConnection::queryBindParameters(Query* query)
{
    lock_guard<mutex> lock(*m_connect);

    for (uint32_t i = 0; i < query->paramCount(); ++i) {
        QueryParameter* param = &query->param(i);
        queryBindParameter(query, param);
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
            cType = SQL_C_CHAR;
            dataType = VAR_STRING;
            break;
    }
}

void ODBCConnection::parseColumns(Query* query, int count)
{
    // Reading the column attributes
    array<char,MAX_NAME_LEN> columnName = {};
    int32_t columnType = 0;
    int32_t columnLength = 0;
    int32_t columnScale = 0;
    int32_t cType = 0;
    VariantType dataType = VAR_NONE;

    stringstream columnNameStr;
    columnNameStr.fill('0');

    for (int16_t column = 1; column <= int16_t(count); ++column) {
        queryColAttributes(query, column, SQL_COLUMN_NAME, columnName.data(), MAX_NAME_LEN - 1);
        queryColAttributes(query, column, SQL_COLUMN_TYPE, columnType);
        queryColAttributes(query, column, SQL_COLUMN_LENGTH, columnLength);
        queryColAttributes(query, column, SQL_COLUMN_SCALE, columnScale);
        ODBCtypeToCType(columnType, cType, dataType);
        if (dataType == VAR_STRING && columnLength > 65535)
            dataType = VAR_TEXT;
        if (columnName[0] != 0)
            columnNameStr.str(columnName.data());
        else {
            columnNameStr.str("column");
            columnNameStr << setw(2) << column;
        }
        if (columnLength > FETCH_BUFFER_SIZE)
            columnLength = FETCH_BUFFER_SIZE;

        if (dataType == VAR_FLOAT && (columnScale < 0 || columnScale > 20))
            columnScale = 0;

        Field* field = new CODBCField(columnNameStr.str(), column, cType, dataType, columnLength, columnScale);
        query->fields().push_back(field);
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
        throw DatabaseException(e.what());
    }
    catch (const Exception& e) {
        THROW_QUERY_ERROR(query, e.what())
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
        parseColumns(query, count);

    }

    querySetEof(query, false);
    queryFetch(query);
}

static uint32_t trimField(char* s, uint32_t sz)
{
    char* p = s + sz;
    char ch = s[0];
    s[0] = '!';

    while (*(--p) == ' ');
    *(++p) = 0;

    if (ch == ' ' && s[1] == 0) {
        s[0] = 0;
        return 0;
    }

    s[0] = ch;
    return uint32_t(p - s);
}

SQLRETURN ODBCConnection::readStringOrBlobField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column,
                                                int16_t fieldType, SQLLEN& dataLength)
{
    field->checkSize(uint32_t(128));
    auto readSize = (SQLLEN) field->bufferSize();
    auto* buffer = field->getBuffer();
    auto rc = SQLGetData(statement, column, fieldType, buffer, SQLINTEGER(readSize), &dataLength);

    SQLLEN offset = readSize - 1;
    if (dataLength > SQLINTEGER(readSize)) { // continue to fetch BLOB data in one go
        field->checkSize(uint32_t(dataLength + 1));
        buffer = field->getBuffer();
        readSize = dataLength - readSize + 2;
        rc = SQLGetData(statement, column, fieldType, buffer + offset, SQLINTEGER(readSize), nullptr);
    }
    else if (dataLength == SQL_NO_TOTAL) {
        size_t bufferSize = field->bufferSize();
        SQLLEN remainingSize = 0;

        dataLength = readSize;
        readSize = 16384;
        while (rc != SQL_SUCCESS) {
            bufferSize += readSize;
            field->checkSize(uint32_t(bufferSize));
            buffer = field->getBuffer();
            rc = SQLGetData(statement, column, fieldType, buffer + offset, SQLINTEGER(readSize), &remainingSize);
            if (remainingSize > 0)
                readSize = remainingSize; // Last chunk received
            offset += readSize - 1;
            dataLength += readSize - 1;
        }
    }

    return rc;
}

SQLRETURN ODBCConnection::readTimestampField(SQLHSTMT statement, DatabaseField* field, SQLUSMALLINT column,
                                             int16_t fieldType, SQLLEN& dataLength)
{
    TIMESTAMP_STRUCT t = {};
    SQLRETURN rc = SQLGetData(statement, column, fieldType, (SQLPOINTER) &t, 0, &dataLength);
    if (dataLength > 0) {
        DateTime dt(t.year, t.month, t.day, t.hour, t.minute, t.second);
        field->setDateTime(dt, field->dataType() == VAR_DATE);
    }
    return rc;
}

void ODBCConnection::queryFetch(Query* query)
{
    if (!query->active())
        THROW_QUERY_ERROR(query, "Dataset isn't open")

    auto* statement = query->statement();

    lock_guard<mutex> lock(*m_connect);

    int rc = SQLFetch(statement);

    if (!successful(rc)) {
        if (rc < 0) {
            THROW_QUERY_ERROR(query, queryError(query))
        } else {
            querySetEof(query, rc == SQL_NO_DATA);
            return;
        }
    }

    auto   fieldCount = query->fieldCount();
    SQLLEN dataLength = 0;

    if (fieldCount == 0)
        return;

    CODBCField* field = nullptr;
    for (SQLUSMALLINT column = 0; column < (SQLUSMALLINT) fieldCount;)
        try {
            field = (CODBCField*) &(*query)[column];
            auto fieldType = (int16_t) field->fieldType();
            char* buffer = field->getData();

            ++column;

            rc = SQL_SUCCESS;
            switch (fieldType) {

                case SQL_C_SLONG:
                case SQL_C_DOUBLE:
                    rc = SQLGetData(statement, column, fieldType, buffer, 0, &dataLength);
                    break;

                case SQL_C_TIMESTAMP:
                    rc = readTimestampField(statement, field, column, fieldType, dataLength);
                    break;

                case SQL_C_BINARY:
                case SQL_C_CHAR:
                    rc = readStringOrBlobField(statement, field, column, fieldType, dataLength);
                    buffer = field->getBuffer();
                    break;

                case SQL_BIT:
                    rc = SQLGetData(statement, column, fieldType, buffer, 1, &dataLength);
                    break;

                default:
                    dataLength = 0;
                    break;
            }

            if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
                throw Exception(queryError(query));

            if (fieldType == SQL_C_CHAR && dataLength > 0)
                dataLength = (SQLINTEGER) trimField(buffer, (uint32_t) dataLength);

            if (dataLength <= 0)
                field->setNull(VAR_NONE);
            else
                field->dataSize((size_t)dataLength);
        }
        catch (const Exception& e) {
            Query::throwError("CODBCConnection::queryFetch",
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
    array<SQLCHAR,MAX_BUF> datasrc = {0};
    array<SQLCHAR,MAX_BUF> descrip = {0};
    SQLSMALLINT rdsrc = 0;
    SQLSMALLINT rdesc = 0;
    SQLRETURN   ret = 0;

    SQLHENV hEnv = ODBCConnectionBase::getEnvironment().handle();
    bool offline = hEnv == nullptr;
    if (offline) {
        if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HENV, &hEnv) != SQL_SUCCESS)
            throw DatabaseException("CODBCConnection::SQLAllocHandle");
        if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER))
            throw DatabaseException("CODBCConnection::SQLSetEnvAttr");
    }

    SQLUSMALLINT direction = SQL_FETCH_FIRST;
    while (true)
    {
        ret = SQLDataSources(
                hEnv, direction,
                datasrc.data(), (SQLSMALLINT) datasrc.size(), &rdsrc,
                descrip.data(), (SQLSMALLINT) descrip.size(), &rdesc);
        if (ret == SQL_NO_DATA)
            break;
        direction = SQL_FETCH_NEXT;
        dsns.push_back(String((char*) datasrc.data()) + " (" +  String((char*) descrip.data()) + ")");
    }

    if (offline)
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void ODBCConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    lock_guard<mutex> lock(*m_connect);

    if (objectType == DOT_DATABASES) {
        listDataSources(objects);
        return;
    }

    SQLHSTMT stmt = nullptr;
    try {
        SQLRETURN rc = 0;
        auto* hdb = handle();
        if (SQLAllocStmt(hdb, &stmt) != SQL_SUCCESS)
            throw DatabaseException("CODBCConnection::SQLAllocStmt");

        switch (objectType) {
            case DOT_TABLES:
                if (SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLCHAR*) Buffer("TABLE").data(), SQL_NTS) != SQL_SUCCESS)
                    throw DatabaseException("SQLTables");
                break;

            case DOT_VIEWS:
                if (SQLTables(stmt, nullptr, 0, nullptr, 0, nullptr, 0, (SQLCHAR*) Buffer("VIEW").data(), SQL_NTS) != SQL_SUCCESS)
                    throw DatabaseException("SQLTables");
                break;

            case DOT_PROCEDURES:
                rc = SQLProcedures(stmt, nullptr, 0, (SQLCHAR*) Buffer("").data(), SQL_NTS, (SQLCHAR*) Buffer("%").data(), SQL_NTS);
                if (rc != SQL_SUCCESS)
                    throw DatabaseException("SQLProcedures");
                break;

            default:
                break;
        }

        array<SQLCHAR,MAX_NAME_LEN> objectSchema = {};
        array<SQLCHAR,MAX_NAME_LEN> objectName = {};
        SQLLEN  cbObjectSchema = 0;
        SQLLEN  cbObjectName = 0;
        if (SQLBindCol(stmt, 2, SQL_C_CHAR, objectSchema.data(), objectSchema.size(), &cbObjectSchema) != SQL_SUCCESS)
            throw DatabaseException("SQLBindCol");
        if (SQLBindCol(stmt, 3, SQL_C_CHAR, objectName.data(), objectName.size(), &cbObjectName) != SQL_SUCCESS)
            throw DatabaseException("SQLBindCol");

        while (true) {
            objectSchema[0] = 0;
            objectName[0] = 0;
            rc = SQLFetch(stmt);
            if (rc == SQL_NO_DATA_FOUND)
                break;
            if (!successful(rc))
                throw DatabaseException("SQLFetch");
            String objectNameStr = String((char*) objectName.data()).replace(";0$", "");
            objects.push_back(String((char*) objectSchema.data()) + "." + objectNameStr);
        }

        SQLFreeStmt(stmt, SQL_DROP);
    }
    catch (const Exception& e) {
        String error;
        if (stmt != nullptr) {
            error = queryError(stmt);
            SQLFreeStmt(stmt, SQL_DROP);
        }
        logAndThrow(e.what(), error);
    }
}

void ODBCConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    RegularExpression   matchStatementEnd("(;\\s*)$");
    RegularExpression   matchRoutineStart("^CREATE\\s+FUNCTION", "i");
    RegularExpression   matchGo("^\\s*GO\\s*$", "i");
    RegularExpression   matchCommentRow("^\\s*--");

    Strings statements;
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

        if (matchRoutineStart.matches(row))
            routineStarted = true;

        if (!routineStarted && matchStatementEnd.matches(row)) {
            row = matchStatementEnd.s(row, "");
            statement += row;
            statements.push_back(trim(statement));
            statement = "";
            continue;
        }

        if (matchGo.matches(row)) {
            routineStarted = false;
            statements.push_back(trim(statement));
            statement = "";
            continue;
        }

        statement += row + "\n";
    }

    if (!trim(statement).empty())
        statements.push_back(statement);

    for (const auto& stmt: statements) {
        try {
            Query query(this, stmt, false);
            query.exec();
        }
        catch (const Exception& e) {
            stringstream error;
            error << e.what() << ". Query: " << stmt;
            if (errors != nullptr)
                errors->push_back(error.str());
            else
                throw DatabaseException(error.str());
        }
    }
}

void ODBCConnection::_bulkInsert(const String& tableName, const Strings& columnNames, const vector<VariantVector>& data)
{
    auto begin = data.begin();
    auto end = data.begin();
    for (; end != data.end(); ++end) {
        if (end - begin > 16) {
            bulkInsertRecords(tableName, columnNames, begin, end);
            begin = end;
        }
    }

    if (begin != end)
        bulkInsertRecords(tableName, columnNames, begin, end);
}

void* odbc_create_connection(const char* connectionString)
{
    auto* connection = new ODBCConnection(connectionString);
    return connection;
}

void odbc_destroy_connection(void* connection)
{
    delete (ODBCConnection*) connection;
}
