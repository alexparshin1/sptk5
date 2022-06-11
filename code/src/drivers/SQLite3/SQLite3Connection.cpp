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

#include <sptk5/sptk.h>

#ifdef HAVE_SQLITE3

#include <sptk5/cutils>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/Query.h>
#include <sptk5/db/SQLite3Connection.h>

namespace sptk {

class SQLite3Field
    : public DatabaseField
{
    friend class SQLite3Connection;

public:
    SQLite3Field(const std::string& fieldName, int fieldColumn)
        : DatabaseField(fieldName, fieldColumn, 0, VariantDataType::VAR_NONE, 0, 0)
    {
    }
};

} // namespace sptk

using namespace std;
using namespace sptk;

SQLite3Connection::SQLite3Connection(const String& connectionString, chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::SQLITE3, connectTimeout)
{
}

void SQLite3Connection::closeAndClean()
{
    try
    {
        if (getInTransaction() && SQLite3Connection::active())
        {
            rollbackTransaction();
        }
        close();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }
}

String SQLite3Connection::nativeConnectionString() const
{
    return "/" + connectionString().databaseName() + "/" + connectionString().schema();
}

void SQLite3Connection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
        setInTransaction(false);

        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        sqlite3* connect = nullptr;
        if (sqlite3_open(nativeConnectionString().c_str(), &connect) != 0)
        {
            string error = sqlite3_errmsg(connect);
            sqlite3_close(connect);
            m_connect.reset();
            throw DatabaseException(error);
        }

        m_connect = shared_ptr<sqlite3>(connect,
                                        [this](const sqlite3*) {
                                            closeAndClean();
                                        });
    }
}

void SQLite3Connection::closeDatabase()
{
    disconnectAllQueries();
    sqlite3_close(m_connect.get());
    m_connect = nullptr;
}

DBHandle SQLite3Connection::handle() const
{
    return (DBHandle) m_connect.get();
}

bool SQLite3Connection::active() const
{
    return m_connect != nullptr;
}

void SQLite3Connection::driverBeginTransaction()
{
    if (m_connect == nullptr)
    {
        open();
    }

    if (getInTransaction())
    {
        throw DatabaseException("Transaction already started.");
    }


    if (char* zErrMsg = nullptr;
        sqlite3_exec(m_connect.get(), "BEGIN TRANSACTION", nullptr, nullptr, &zErrMsg) != SQLITE_OK)
    {
        throw DatabaseException(zErrMsg);
    }

    setInTransaction(true);
}

void SQLite3Connection::driverEndTransaction(bool commit)
{
    if (!getInTransaction())
    {
        throw DatabaseException("Transaction isn't started.");
    }

    const char* action = commit ? "COMMIT" : "ROLLBACK";


    if (char* zErrMsg = nullptr;
        sqlite3_exec(m_connect.get(), action, nullptr, nullptr, &zErrMsg) != SQLITE_OK)
    {
        throw DatabaseException(zErrMsg);
    }

    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------

String SQLite3Connection::queryError(const Query*) const
{
    return sqlite3_errmsg(m_connect.get());
}

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void SQLite3Connection::queryAllocStmt(Query* query)
{
    scoped_lock lock(m_mutex);

    if (auto* stmt = (SQLHSTMT) query->statement(); stmt != nullptr)
    {
        sqlite3_finalize(stmt);
    }

    querySetStmt(query, nullptr);
}

void SQLite3Connection::queryFreeStmt(Query* query)
{
    scoped_lock lock(m_mutex);

    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void SQLite3Connection::queryCloseStmt(Query* query)
{
    queryFreeStmt(query);
}

void SQLite3Connection::queryPrepare(Query* query)
{
    scoped_lock lock(m_mutex);

    SQLHSTMT hStmt = nullptr;

    if (const char* pzTail = nullptr;
        sqlite3_prepare(m_connect.get(), query->sql().c_str(), int(query->sql().length()), &hStmt, &pzTail) !=
        SQLITE_OK)
    {
        const char* errorMsg = sqlite3_errmsg(m_connect.get());
        throw DatabaseException(errorMsg, __FILE__, __LINE__, query->sql());
    }

    auto statement = shared_ptr<uint8_t>((StmtHandle) hStmt,
                                         [](StmtHandle ptr) {
                                             auto* stmt = (SQLHSTMT) ptr;
                                             sqlite3_finalize(stmt);
                                         });
    querySetStmt(query, statement);
    querySetPrepared(query, true);
}

void SQLite3Connection::queryExecute(Query* query)
{
    scoped_lock lock(m_mutex);

    if (!query->prepared())
    {
        throw DatabaseException("Query isn't prepared");
    }
}

int SQLite3Connection::queryColCount(Query* query)
{
    scoped_lock lock(m_mutex);

    auto* stmt = (SQLHSTMT) query->statement();

    return sqlite3_column_count(stmt);
}

void SQLite3Connection::queryBindParameters(Query* query)
{
    scoped_lock lock(m_mutex);

    for (uint32_t i = 0; i < query->paramCount(); ++i)
    {
        bindParameter(query, i);
    }
}

void SQLite3Connection::bindParameter(const Query* query, uint32_t paramNumber) const
{
    auto* stmt = (SQLHSTMT) query->statement();
    QueryParameter* param = &query->param(paramNumber);
    VariantDataType ptype = param->dataType();

    for (unsigned j = 0; j < param->bindCount(); ++j)
    {

        int rc {0};
        auto paramBindNumber = short(param->bindIndex(j) + 1);

        if (param->isNull())
        {
            rc = sqlite3_bind_null(stmt, paramBindNumber);
        }
        else
        {
            switch (ptype)
            {
                case VariantDataType::VAR_BOOL:
                    rc = sqlite3_bind_int(stmt, paramBindNumber, param->get<bool>());
                    break;

                case VariantDataType::VAR_INT:
                    rc = sqlite3_bind_int(stmt, paramBindNumber, param->get<int>());
                    break;

                case VariantDataType::VAR_INT64:
                    rc = sqlite3_bind_int64(stmt, paramBindNumber, param->get<int64_t>());
                    break;

                case VariantDataType::VAR_FLOAT:
                    rc = sqlite3_bind_double(stmt, paramBindNumber, param->get<double>());
                    break;

                case VariantDataType::VAR_DATE_TIME:
                    rc = transformDateTimeParameter(stmt, param, paramBindNumber);
                    break;

                case VariantDataType::VAR_STRING:
                case VariantDataType::VAR_TEXT:
                    rc = sqlite3_bind_text(stmt, paramBindNumber, param->getString(), int(param->dataSize()),
                                           nullptr);
                    break;

                case VariantDataType::VAR_BUFFER:
                    rc = sqlite3_bind_blob(stmt, paramBindNumber, param->getString(), int(param->dataSize()),
                                           nullptr);
                    break;

                default:
                    throw DatabaseException(
                        "Unsupported parameter type(" + to_string((int) param->dataType()) + ") for parameter '" +
                        param->name() + "'");
            }
        }

        if (rc != SQLITE_OK)
        {
            string error = sqlite3_errmsg(m_connect.get());
            throw DatabaseException(
                error + ", in binding parameter " + int2string(paramBindNumber), __FILE__, __LINE__,
                query->sql());
        }
    }
}
int SQLite3Connection::transformDateTimeParameter(sqlite3_stmt* stmt, QueryParameter* param, short paramBindNumber) const
{
    int rc;
    auto dt = param->get<DateTime>();
    param->setString(dt.isoDateTimeString());
    rc = sqlite3_bind_text(stmt, paramBindNumber, param->getString(), int(param->dataSize()),
                           nullptr);
    return rc;
}

void SQLite3Connection::queryOpen(Query* query)
{
    if (!active())
    {
        open();
    }

    if (query->active())
    {
        return;
    }

    if (active() && query->statement() == nullptr)
    {
        queryAllocStmt(query);
    }

    if (!query->prepared())
    {
        queryPrepare(query);
    }

    queryBindParameters(query);
    queryExecute(query);

    auto count = (short) queryColCount(query);

    query->fields().clear();

    auto* stmt = (SQLHSTMT) query->statement();

    if (count < 1)
    {
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            String error = queryError(query);
            queryCloseStmt(query);
            throw DatabaseException(error, __FILE__, __LINE__, query->sql());
        }

        queryCloseStmt(query);
        return;
    }

    querySetActive(query, true);

    // Reading the column attributes
    for (short column = 1; column <= count; ++column)
    {
        String columnName(sqlite3_column_name(stmt, column - 1));
        if (columnName.empty())
        {
            columnName = "column_" + to_string(column);
        }

        auto field = make_shared<SQLite3Field>(columnName, column);
        query->fields().push_back(field);
    }

    querySetEof(query, false);

    queryFetch(query);
}

static uint32_t trimField(char* s, uint32_t sz)
{
    if (sz == 0)
    {
        return 0;
    }

    char* p = s + sz - 1;
    char ch = s[0];
    s[0] = '!';

    while (*p == ' ')
    {
        --p;
    }

    *(++p) = 0;

    if (ch == ' ' && s[1] == 0)
    {
        return 0;
    }

    s[0] = ch;
    return uint32_t(p - s);
}

void SQLite3Connection::queryFetch(Query* query)
{
    if (!query->active())
    {
        throw DatabaseException("Dataset isn't open", __FILE__, __LINE__, query->sql());
    }

    auto* statement = (SQLHSTMT) query->statement();

    scoped_lock lock(m_mutex);

    switch (sqlite3_step(statement))
    {
        case SQLITE_DONE:
            querySetEof(query, true);
            return;

        case SQLITE_ROW:
            break;

        default:
            throw DatabaseException(queryError(query), __FILE__, __LINE__, query->sql());
    }

    auto fieldCount = query->fieldCount();
    if (fieldCount == 0)
    {
        return;
    }

    SQLite3Field* field = nullptr;

    for (uint32_t column = 0; column < fieldCount; ++column)
    {
        try
        {
            field = (SQLite3Field*) &(*query)[column];

            auto fieldType = (short) field->fieldType();
            if (fieldType == 0 || fieldType == 5)
            {
                fieldType = (short) sqlite3_column_type(statement, int(column));
                field->setFieldType(fieldType, 0, 0);
            }

            auto dataLength = (uint32_t) sqlite3_column_bytes(statement, int(column));

            if (dataLength != 0)
            {
                switch (fieldType)
                {

                    case SQLITE_INTEGER:
                        field->setInt64(sqlite3_column_int64(statement, int(column)));
                        break;

                    case SQLITE_FLOAT:
                        field->setFloat(sqlite3_column_double(statement, int(column)));
                        break;

                    case SQLITE_TEXT:
                        field->setBuffer(sqlite3_column_text(statement, int(column)), dataLength,
                                         VariantDataType::VAR_STRING);
                        dataLength = trimField((char*) field->get<Buffer>().data(), dataLength);
                        break;

                    case SQLITE_BLOB:
                        field->setBuffer((const uint8_t*) sqlite3_column_blob(statement, int(column)), dataLength,
                                         VariantDataType::VAR_BUFFER);
                        break;

                    default:
                        dataLength = 0;
                        break;
                }

                field->dataSize(dataLength);
            }
            else
            {
                field->setString("");
                field->setNull(VariantDataType::VAR_NONE);
            }
        }
        catch (const Exception& e)
        {
            throw DatabaseException(
                "Can't read field " + field->fieldName() + "\n" + string(e.what()), __FILE__, __LINE__,
                query->sql());
        }
    }
}

void SQLite3Connection::_executeBatchSQL(const sptk::Strings& sqlBatch, Strings* errors)
{
    RegularExpression matchStatementEnd("(;\\s*)$");
    RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    string statement;
    for (String row: sqlBatch)
    {
        row = trim(row);
        if (row.empty() || matchCommentRow.matches(row))
        {
            continue;
        }

        row = trim(row);
        if (row.empty() || row.startsWith("--"))
        {
            continue;
        }

        if (matchStatementEnd.matches(row))
        {
            row = matchStatementEnd.s(row, "");
            statement += row;
            statements.push_back(trim(statement));
            statement = "";
            continue;
        }

        statement += row + "\n";
    }

    if (!trim(statement).empty())
    {
        statements.push_back(statement);
    }

    for (const auto& stmt: statements)
    {
        try
        {
            Query query(this, stmt, false);
            query.exec();
        }
        catch (const Exception& e)
        {
            stringstream error;
            error << e.what() << ". Query: " << stmt;
            if (errors != nullptr)
            {
                errors->push_back(error.str());
            }
            else
            {
                throw DatabaseException(error.str());
            }
        }
    }
}

void SQLite3Connection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectTypeName;
    objects.clear();

    switch (objectType)
    {
        case DatabaseObjectType::TABLES:
            objectTypeName = "table";
            break;

        case DatabaseObjectType::VIEWS:
            objectTypeName = "view";
            break;

        default:
            return; // no information about objects of other types
    }

    Query query(this, "SELECT name FROM sqlite_master WHERE type='" + objectTypeName + "'");
    query.open();

    while (!query.eof())
    {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }

    query.close();
}

String SQLite3Connection::driverDescription() const
{
    return "SQLite3 " SQLITE_VERSION;
}

void SQLite3Connection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
    notImplemented("queryColAttributes");
}

void SQLite3Connection::queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len)
{
    notImplemented("queryColAttributes");
}

map<SQLite3Connection*, shared_ptr<SQLite3Connection>> SQLite3Connection::s_sqlite3Connections;

[[maybe_unused]] void* sqlite3_create_connection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    auto connection = make_shared<SQLite3Connection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    SQLite3Connection::s_sqlite3Connections[connection.get()] = connection;
    return connection.get();
}

[[maybe_unused]] void sqlite3_destroy_connection(void* connection)
{
    SQLite3Connection::s_sqlite3Connections.erase((SQLite3Connection*) connection);
}

#endif
