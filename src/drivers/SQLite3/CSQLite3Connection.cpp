/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CSQLite3Connection.cpp - description                   ║
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

#include <sptk5/sptk.h>

#if HAVE_SQLITE3

#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/Query.h>
#include <sptk5/db/SQLite3Connection.h>

namespace sptk
{

class CSQLite3Field : public DatabaseField
{
    friend class SQLite3Connection;

public:
    CSQLite3Field(const std::string& fieldName, int fieldColumn)
            :
            DatabaseField(fieldName, fieldColumn, 0, VAR_NONE, 0, 0)
    {
    }

    void setFieldType(int fieldType, int fieldLength, int fieldScale)
    {
        m_fldType = fieldType;
        m_fldSize = fieldLength;
        m_fldScale = fieldScale;
    }
};

} // namespace sptk

using namespace std;
using namespace sptk;

extern "C" {
typedef void (* sqlite3cb)(void*);
}

SQLite3Connection::SQLite3Connection(const String& connectionString)
        :
        PoolDatabaseConnection(connectionString)
{
    m_connect = nullptr;
    m_connType = DCT_SQLITE3;
}

SQLite3Connection::~SQLite3Connection()
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

String SQLite3Connection::nativeConnectionString() const
{
    return m_connString.databaseName();
}

void SQLite3Connection::_openDatabase(const String& newConnectionString)
{
    if (!active()) {
        m_inTransaction = false;

        if (!newConnectionString.empty())
            m_connString = DatabaseConnectionString(newConnectionString);

        if (sqlite3_open(nativeConnectionString().c_str(), &m_connect) != 0) {
            string error = sqlite3_errmsg(m_connect);
            sqlite3_close(m_connect);
            m_connect = nullptr;
            throw DatabaseException(error);
        }
    }
}

void SQLite3Connection::closeDatabase()
{
    for (auto query: m_queryList) {
        try {
            queryFreeStmt(query);
        } catch (...) {
        }
    }

    sqlite3_close(m_connect);
    m_connect = nullptr;
}

void* SQLite3Connection::handle() const
{
    return m_connect;
}

bool SQLite3Connection::active() const
{
    return m_connect != nullptr;
}

void SQLite3Connection::driverBeginTransaction()
{
    if (m_connect == nullptr)
        open();

    if (m_inTransaction)
        throw DatabaseException("Transaction already started.");

    char* zErrMsg;

    if (sqlite3_exec(m_connect, "BEGIN TRANSACTION", nullptr, nullptr, &zErrMsg) != SQLITE_OK)
        throw DatabaseException(zErrMsg);

    m_inTransaction = true;
}

void SQLite3Connection::driverEndTransaction(bool commit)
{
    if (!m_inTransaction)
        throw DatabaseException("Transaction isn't started.");

    string action;

    if (commit)
        action = "COMMIT";
    else
        action = "ROLLBACK";

    char* zErrMsg;

    if (sqlite3_exec(m_connect, action.c_str(), nullptr, nullptr, &zErrMsg) != SQLITE_OK)
        throw DatabaseException(zErrMsg);

    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------

String SQLite3Connection::queryError(const Query*) const
{
    return sqlite3_errmsg(m_connect);
}

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void SQLite3Connection::queryAllocStmt(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto stmt = (SQLHSTMT) query->statement();
    if (stmt != nullptr)
        sqlite3_finalize(stmt);

    querySetStmt(query, nullptr);
}

void SQLite3Connection::queryFreeStmt(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto stmt = (SQLHSTMT) query->statement();

    if (stmt != nullptr)
        sqlite3_finalize(stmt);

    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void SQLite3Connection::queryCloseStmt(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto stmt = (SQLHSTMT) query->statement();
    if (stmt != nullptr)
        sqlite3_finalize(stmt);

    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void SQLite3Connection::queryPrepare(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    SQLHSTMT stmt;
    const char* pzTail;

    if (sqlite3_prepare(m_connect, query->sql().c_str(), int(query->sql().length()), &stmt, &pzTail) != SQLITE_OK) {
        const char* errorMsg = sqlite3_errmsg(m_connect);
        throw DatabaseException(errorMsg, __FILE__, __LINE__, query->sql());
    }

    //sqlite3_clear_bindings(stmt);

    querySetStmt(query, stmt);
    querySetPrepared(query, true);
}

void SQLite3Connection::queryUnprepare(Query* query)
{
    queryFreeStmt(query);
}

void SQLite3Connection::queryExecute(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    if (!query->prepared())
        throw DatabaseException("Query isn't prepared");
}

int SQLite3Connection::queryColCount(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto stmt = (SQLHSTMT) query->statement();

    return sqlite3_column_count(stmt);
}

void SQLite3Connection::queryBindParameters(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto stmt = (SQLHSTMT) query->statement();

    for (uint32_t i = 0; i < query->paramCount(); i++) {
        QueryParameter* param = &query->param(i);
        VariantType ptype = param->dataType();

        //SQLINTEGER& cblen = param->callbackLength();
        for (unsigned j = 0; j < param->bindCount(); j++) {

            int rc;
            auto paramNumber = short(param->bindIndex(j) + 1);

            if (param->isNull())
                rc = sqlite3_bind_null(stmt, paramNumber);
            else
                switch (ptype) {
                    case VAR_BOOL:
                    case VAR_INT:
                        rc = sqlite3_bind_int(stmt, paramNumber, param->getInteger());
                        break;

                    case VAR_INT64:
                        rc = sqlite3_bind_int64(stmt, paramNumber, param->getInt64());
                        break;

                    case VAR_FLOAT:
                        rc = sqlite3_bind_double(stmt, paramNumber, param->getFloat());
                        break;

                    case VAR_STRING:
                    case VAR_TEXT:
                        rc = sqlite3_bind_text(stmt, paramNumber, param->getString(), int(param->dataSize()),
                                               (sqlite3cb) SQLITE_STATIC);
                        break;

                    case VAR_BUFFER:
                        rc = sqlite3_bind_blob(stmt, paramNumber, param->getString(), int(param->dataSize()),
                                               (sqlite3cb) SQLITE_STATIC);
                        break;

                    case VAR_DATE:
                    case VAR_DATE_TIME:
                        throwException("Date and time types isn't yet supported for SQLite3");

                    default:
                        throw DatabaseException(
                                "Unsupported type of parameter " + int2string(paramNumber), __FILE__, __LINE__,
                                query->sql());
                }

            if (rc != SQLITE_OK) {
                string error = sqlite3_errmsg(m_connect);
                throw DatabaseException(
                        error + ", in binding parameter " + int2string(paramNumber), __FILE__, __LINE__,
                        query->sql());
            }
        }
    }
}

void SQLite3Connection::SQLITEtypeToCType(int sqliteType, VariantType& dataType)
{
    switch (sqliteType) {

        case SQLITE_INTEGER:
            dataType = VAR_INT64;
            break;

        case SQLITE_FLOAT:
            dataType = VAR_FLOAT;
            break;

        case 0:
        case SQLITE_TEXT:
            dataType = VAR_STRING;
            break;

        case SQLITE_BLOB:
            dataType = VAR_BUFFER;
            break;

        default:
            dataType = VAR_NONE;
            break;
    }
}

void SQLite3Connection::queryOpen(Query* query)
{
    if (!active())
        open();

    if (query->active())
        return;

    if (active() && query->statement() == nullptr)
        queryAllocStmt(query);

    if (!query->prepared())
        queryPrepare(query);

    queryBindParameters(query);
    queryExecute(query);

    auto count = (short) queryColCount(query);

    query->fields().clear();

    auto stmt = (SQLHSTMT) query->statement();

    if (count < 1) {
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            string error = queryError(query);
            queryCloseStmt(query);
            throw DatabaseException(error, __FILE__, __LINE__, query->sql());
        }

        queryCloseStmt(query);
        return;
    }

    querySetActive(query, true);

    // Reading the column attributes
    char columnName[256];

    //long  columnType;
    //VariantType dataType;
    for (short column = 1; column <= count; column++) {
        strncpy(columnName, sqlite3_column_name(stmt, column - 1), 255);
        columnName[255] = 0;

        if (columnName[0] == 0)
            snprintf(columnName, sizeof(columnName), "column%02i", column);

        CSQLite3Field* field = new CSQLite3Field(columnName, column);
        query->fields().push_back(field);
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

    if (ch == ' ' && s[1] == 0)
        return 0;

    s[0] = ch;
    return uint32_t(p - s);
}

void SQLite3Connection::queryFetch(Query* query)
{
    if (!query->active())
        throw DatabaseException("Dataset isn't open", __FILE__, __LINE__, query->sql());

    auto statement = (SQLHSTMT) query->statement();

    lock_guard<mutex> lock(m_mutex);

    int rc = sqlite3_step(statement);

    switch (rc) {
        case SQLITE_DONE:
            querySetEof(query, true);
            return;

        case SQLITE_ROW:
            break;

        default:
            throw DatabaseException(queryError(query), __FILE__, __LINE__, query->sql());
    }

    uint32_t fieldCount = query->fieldCount();
    uint32_t dataLength = 0;

    if (fieldCount == 0)
        return;

    CSQLite3Field* field = nullptr;

    for (uint32_t column = 0; column < fieldCount; column++) {
        try {
            field = (CSQLite3Field*) &(*query)[(uint32_t) column];

            auto fieldType = (short) field->fieldType();
            if (fieldType == 0) {
                fieldType = (short) sqlite3_column_type(statement, int(column));
                field->setFieldType(fieldType, 0, 0);
            }

            dataLength = (uint32_t) sqlite3_column_bytes(statement, int(column));

            if (dataLength != 0) {
                switch (fieldType) {

                    case SQLITE_INTEGER:
                        field->setInt64(sqlite3_column_int64(statement, int(column)));
                        break;

                    case SQLITE_FLOAT:
                        field->setFloat(sqlite3_column_double(statement, int(column)));
                        break;

                    case SQLITE_TEXT:
                        field->setString((const char*) sqlite3_column_text(statement, int(column)), dataLength);
                        dataLength = trimField((char*) field->getString(), dataLength);
                        break;

                    case SQLITE_BLOB:
                        field->setBuffer(sqlite3_column_blob(statement, int(column)), dataLength);
                        break;

                    default:
                        dataLength = 0;
                        break;
                }

                field->dataSize(dataLength);

            } else {
                field->setString("");
                field->setNull(VAR_NONE);
            }
        } catch (exception& e) {
            throw DatabaseException(
                    "Can't read field " + field->fieldName() + "\n" + string(e.what()), __FILE__, __LINE__,
                    query->sql());
        }
    }
}

void SQLite3Connection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectTypeName;
    objects.clear();

    switch (objectType) {
        case DOT_TABLES:
            objectTypeName = "table";
            break;

        case DOT_VIEWS:
            objectTypeName = "view";
            break;

        default:
            return; // no information about objects of other types
    }

    Query query(this, "SELECT name FROM sqlite_master WHERE type='" + objectTypeName + "'");
    query.open();

    while (!query.eof()) {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }

    query.close();
}

String SQLite3Connection::driverDescription() const
{
    return "SQLite3 " SQLITE_VERSION;
}

void* sqlite3_create_connection(const char* connectionString)
{
    SQLite3Connection* connection = new SQLite3Connection(connectionString);
    return connection;
}

void sqlite3_destroy_connection(void* connection)
{
    delete (SQLite3Connection*) connection;
}

#endif
