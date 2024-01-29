/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/db/OracleOciDatabaseField.h"
#include <format>
#include <sptk5/cutils>
#include <sptk5/db/BulkQuery.h>
#include <sptk5/db/OracleOciConnection.h>
#include <sptk5/db/OracleOciStatement.h>

using namespace std;
using namespace sptk;
using namespace ocilib;

namespace {
void readTimestamp(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex);
void readDateTime(const Resultset& resultSet, sptk::DatabaseField* field, unsigned int columnIndex);
void readDate(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex);
void readBLOB(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex);
void readCLOB(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex);
} // namespace

OracleOciConnection::OracleOciConnection(const String& connectionString, chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::ORACLE_OCI, connectTimeout)
{
    static mutex initializeMutex;
    const scoped_lock lock(initializeMutex);
    if (!ocilib::Environment::Initialized())
    {
        ocilib::Environment::Initialize();
    }
}

OracleOciConnection::~OracleOciConnection()
{
    try
    {
        if (getInTransaction() && OracleOciConnection::active())
        {
            rollbackTransaction();
        }
        disconnectAllQueries();
        close();
    }
    catch (const sptk::Exception& e)
    {
        CERR(e.what() << endl);
    }
    catch (const ocilib::Exception& e)
    {
        CERR(e.what() << endl);
    }
}

Connection* OracleOciConnection::connection() const
{
    return m_connection.get();
}

void OracleOciConnection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
        setInTransaction(false);
        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        try
        {
            DatabaseConnectionString dbConnectionString = connectionString();
            auto oracleService = format("{}:{}/{}", dbConnectionString.hostName().c_str(),
                                        dbConnectionString.portNumber(), dbConnectionString.databaseName().c_str());
            m_connection = make_shared<ocilib::Connection>(oracleService, dbConnectionString.userName(), dbConnectionString.password());
            m_connection->SetAutoCommit(true);
        }
        catch (const ocilib::Exception& e)
        {
            if (strstr(e.what(), "already used") == nullptr)
            {
                if (m_connection)
                {
                    m_connection.reset();
                }
                throw DatabaseException(string("Can't create connection: ") + e.what());
            }
        }
    }
}

void OracleOciConnection::closeDatabase()
{
    try
    {
        m_connection.reset();
    }
    catch (const ocilib::Exception& e)
    {
        throw DatabaseException(string("Can't close connection: ") + e.what());
    }
}

void OracleOciConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    static const RegularExpression matchStatementEnd("(;\\s*)$");
    static const RegularExpression matchRoutineStart("^CREATE (OR REPLACE )?(FUNCTION|TRIGGER)", "i");
    static const RegularExpression matchGo("^/\\s*$");
    static const RegularExpression matchShowErrors("^SHOW\\s+ERRORS", "i");
    static const RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    string statement;
    bool routineStarted = false;
    for (const auto& aRow: batchSQL)
    {
        String row = trim(aRow);
        if (row.empty() || matchCommentRow.matches(row))
        {
            continue;
        }

        if (!routineStarted)
        {
            row = trim(row);
            if (row.empty())
            {
                continue;
            }
            if (matchShowErrors.matches(row))
            {
                continue;
            }
        }

        if (matchRoutineStart.matches(row))
        {
            routineStarted = true;
        }

        if (!routineStarted && matchStatementEnd.matches(row))
        {
            row = matchStatementEnd.s(row, "");
            statement += row;
            statements.push_back(trim(statement));
            statement = "";
            continue;
        }

        if (matchGo.matches(row))
        {
            routineStarted = false;
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

    executeMultipleStatements(statements, errors);
}

void OracleOciConnection::executeMultipleStatements(const Strings& statements, Strings* errors)
{
    for (const auto& stmt: statements)
    {
        try
        {
            Query query(this, stmt, false);
            query.exec();
        }
        catch (const Exception& e)
        {
            if (errors)
            {
                errors->push_back(e.what() + String(": ") + stmt);
            }
            else
            {
                throw;
            }
        }
        catch (const ocilib::Exception& e)
        {
            if (errors)
            {
                errors->push_back(e.what() + String(": ") + stmt);
            }
            else
            {
                throw;
            }
        }
    }
}

bool OracleOciConnection::active() const
{
    return (bool) m_connection;
}

String OracleOciConnection::driverDescription() const
{
    auto driverVersion = format("OracleOci {}.{}", Environment::GetCompileMajorVersion(), Environment::GetCompileMinorVersion());
    return driverVersion;
}

void OracleOciConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
        using enum sptk::DatabaseObjectType;
        case PROCEDURES:
            objectsSQL = "SELECT object_name FROM user_procedures WHERE object_type = 'PROCEDURE'";
            break;
        case FUNCTIONS:
            objectsSQL = "SELECT object_name FROM user_procedures WHERE object_type = 'FUNCTION'";
            break;
        case TABLES:
            objectsSQL = "SELECT table_name FROM user_tables";
            break;
        case VIEWS:
            objectsSQL = "SELECT view_name FROM sys.all_views";
            break;
        case DATABASES:
            objectsSQL = "SELECT username FROM all_users";
            break;
        default:
            throw Exception("Not implemented yet");
    }
    Query query(this, objectsSQL);
    query.open();
    while (!query.eof())
    {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }
    query.close();
}

void OracleOciConnection::driverBeginTransaction()
{
    m_connection->SetAutoCommit(false);
}

void OracleOciConnection::driverEndTransaction(bool commit)
{
    if (commit)
    {
        m_connection->Commit();
    }
    else
    {
        m_connection->Rollback();
    }
    m_connection->SetAutoCommit(true);
}

void OracleOciConnection::queryAllocStmt(Query* query)
{
    auto stmt = make_shared<OracleOciStatement>(this, query->sql());
    querySetStmt(query, reinterpret_pointer_cast<uint8_t>(stmt));
}

void OracleOciConnection::queryFreeStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void OracleOciConnection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    auto* statement = bit_cast<OracleOciStatement*>(query->statement());
    if (statement)
    {
        statement->close();
    }
}

void OracleOciConnection::queryPrepare(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<OracleOciStatement*>(query->statement());
    statement->enumerateParams(query->params());

    if (!query->prepared())
    {
        try
        {
            statement->statement()->Prepare(query->sql());
            statement->bindParameters();
            querySetPrepared(query, true);
        }
        catch (const ocilib::Exception& e)
        {
            THROW_QUERY_ERROR(query, e.what());
        }
    }
    querySetPrepared(query, true);
}

void OracleOciConnection::queryExecute(Query* query)
{
    try
    {
        auto* statement = bit_cast<OracleOciStatement*>(query->statement());
        if (statement == nullptr)
        {
            throw Exception("Query is not prepared");
        }
        statement->execute(getInTransaction());
    }
    catch (const ocilib::Exception& e)
    {
        throw DatabaseException(e.what());
    }
}

size_t OracleOciConnection::queryColCount(Query* query)
{
    const auto* statement = bit_cast<OracleOciStatement*>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Query not opened");
    }
    const auto resultSet = statement->statement()->GetResultset();
    const size_t columnCount = resultSet ? resultSet.GetColumnCount() : 0;
    return columnCount;
}

void OracleOciConnection::queryBindParameters(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<OracleOciStatement*>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Query not prepared");
    }

    try
    {
        statement->setParameterValues();
    }
    catch (const ocilib::Exception& e)
    {
        throw DatabaseException(e.what());
    }
}

void OracleOciConnection::queryOpen(Query* query)
{
    if (!active())
    {
        open();
    }

    if (query->active())
    {
        return;
    }

    if (query->statement() == nullptr)
    {
        queryAllocStmt(query);
    }

    if (!query->prepared())
    {
        queryPrepare(query);
    }

    // Bind parameters also executes a query
    queryBindParameters(query);

    const auto* statement = bit_cast<OracleOciStatement*>(query->statement());

    queryExecute(query);
    if (queryColCount(query) < 1)
    {
        return;
    }
    else
    {
        querySetActive(query, true);
        if (query->fieldCount() == 0)
        {
            const scoped_lock lock(m_mutex);
            const auto resultSet = statement->resultSet();
            createQueryFieldsFromMetadata(query, resultSet);
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

VariantDataType OracleOciConnection::oracleOciTypeToVariantType(ocilib::DataType oracleType, int scale)
{
    VariantDataType dataType {VariantDataType::VAR_STRING};
    switch (oracleType.GetValue())
    {
        using enum sptk::VariantDataType;
        case OCI_CDT_NUMERIC:
            dataType = scale == 0 ? VAR_INT64 : VAR_FLOAT;
            break;
        case OCI_CDT_DATETIME:
        case OCI_CDT_TIMESTAMP:
            dataType = VAR_DATE_TIME;
            break;
        case OCI_CDT_LOB:
            dataType = VAR_BUFFER;
            break;
        case OCI_CDT_BOOLEAN:
            dataType = VAR_INT;
            break;
        default:
            break;
    }
    return dataType;
}

void OracleOciConnection::createQueryFieldsFromMetadata(Query* query, const Resultset& resultSet)
{
    unsigned columnCount = resultSet.GetColumnCount();
    for (unsigned columnIndex = 0; columnIndex < columnCount; ++columnIndex)
    {
        auto column = resultSet.GetColumn(columnIndex + 1);
        auto columnType = column.GetType();
        auto columnSqlType = column.GetSQLType();
        const int columnScale = column.GetScale();
        String columnName(column.GetName());
        const auto columnDataSize = column.GetSize();
        if (columnName.empty())
        {
            columnName = format("column_{}", columnIndex + 1);
        }

        const VariantDataType dataType = oracleOciTypeToVariantType(columnType, columnScale);
        auto field = make_shared<OracleOciDatabaseField>(
            columnName, columnType, dataType, columnDataSize,
            columnScale, columnSqlType);
        query->fields().push_back(field);
    }
}

void OracleOciConnection::queryFetch(Query* query)
{
    if (!query->active())
    {
        THROW_QUERY_ERROR(query, "Dataset isn't open");
    }

    const scoped_lock lock(m_mutex);

    const auto* statement = bit_cast<OracleOciStatement*>(query->statement());
    auto resultSet = statement->resultSet();
    if (!resultSet)
    {
        querySetEof(query, true);
        return;
    }

    if (!resultSet.Next())
    {
        querySetEof(query, true);
    }

    const auto fieldCount = query->fieldCount();
    if (fieldCount == 0)
    {
        querySetEof(query, true);
        return;
    }

    for (unsigned fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
    {
        auto* field = dynamic_cast<OracleOciDatabaseField*>(&(*query)[fieldIndex]);

        try
        {
            // Result set column index starts from 1
            const auto columnIndex = fieldIndex + 1;
            const auto dataType = field->dataType();

            if (resultSet.IsColumnNull(columnIndex))
            {
                field->setNull(dataType);
                continue;
            }

            switch (dataType)
            {
                using enum sptk::VariantDataType;
                case VAR_INT:
                    field->setInteger(resultSet.Get<int>(columnIndex));
                    break;

                case VAR_INT64:
                    field->setInt64(resultSet.Get<big_int>(columnIndex));
                    break;

                case VAR_FLOAT:
                    field->setFloat(resultSet.Get<double>(columnIndex));
                    break;

                case VAR_DATE:
                case VAR_DATE_TIME:
                    readDateTimeOrTimestamp(resultSet, field, columnIndex);
                    break;

                case VAR_BUFFER:
                case VAR_TEXT:
                case VAR_STRING:
                    readBuffer(resultSet, field, columnIndex);
                    break;

                default:
                    field->setString(resultSet.Get<string>(columnIndex));
                    break;
            }
        }
        catch (const Exception& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
        catch (const ocilib::Exception& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
    }
}

void OracleOciConnection::readDateTimeOrTimestamp(const ocilib::Resultset& resultSet, OracleOciDatabaseField* field, unsigned columnIndex)
{
    if (field->sqlType() == "timestamp")
    {
        readTimestamp(resultSet, field, columnIndex);
    }
    else if (field->sqlType() == "datetime")
    {
        readDate(resultSet, field, columnIndex);
    }
    else
    {
        readDateTime(resultSet, field, columnIndex);
    }
}

void OracleOciConnection::readBuffer(const ocilib::Resultset& resultSet, OracleOciDatabaseField* field, unsigned columnIndex)
{
    switch (field->fieldType())
    {
        case OCI_CDT_LOB:
            if (field->sqlType() == "clob")
            {
                readCLOB(resultSet, field, columnIndex);
            }
            else
            {
                readBLOB(resultSet, field, columnIndex);
            }
            break;
        case OCI_CDT_TEXT:
            field->setString(resultSet.Get<string>(columnIndex));
            break;
        default:
            throw DatabaseException("Unsupported buffer type");
    }
}

void OracleOciConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
    notImplemented("queryColAttributes");
}

void OracleOciConnection::queryColAttributes(Query*, int16_t, int16_t, char*, int)
{
    notImplemented("queryColAttributes");
}

String OracleOciConnection::paramMark(unsigned int paramIndex)
{
    return format(":{}", paramIndex + 1);
}

String OracleOciConnection::queryError(const Query* query) const
{
    return {};
}

namespace {
void readTimestamp(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto date = resultSet.Get<Timestamp>(columnIndex);
    if (date.IsNull())
    {
        field->setNull(sptk::VariantDataType::VAR_DATE_TIME);
    }
    else
    {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        int millisecond = 0;
        date.GetDateTime(year, month, day, hour, minute, second, millisecond);
        field->setDateTime(DateTime(static_cast<short>(year), static_cast<short>(month), static_cast<short>(day), static_cast<short>(hour), static_cast<short>(minute), static_cast<short>(second)), false);
    }
}

void readDateTime(const Resultset& resultSet, sptk::DatabaseField* field, unsigned int columnIndex)
{
    auto date = resultSet.Get<Date>(columnIndex);
    if (date.IsNull())
    {
        field->setNull(sptk::VariantDataType::VAR_DATE_TIME);
    }
    else
    {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        date.GetDateTime(year, month, day, hour, minute, second);
        field->setDateTime(DateTime(static_cast<short>(year), static_cast<short>(month), static_cast<short>(day),
                                    static_cast<short>(hour), static_cast<short>(minute), static_cast<short>(second)),
                           false);
    }
}

void readDate(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto date = resultSet.Get<Date>(columnIndex);
    if (date.IsNull())
    {
        field->setNull(sptk::VariantDataType::VAR_DATE);
    }
    else
    {
        int year = 0;
        int month = 0;
        int day = 0;
        date.GetDate(year, month, day);
        field->setDateTime(DateTime(static_cast<short>(year), static_cast<short>(month), static_cast<short>(day), static_cast<short>(0), static_cast<short>(0), static_cast<short>(0)), true);
    }
}

void readCLOB(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto clob = resultSet.Get<Clob>(columnIndex);
    if (clob.IsNull())
    {
        field->setNull(VariantDataType::VAR_TEXT);
    }
    else
    {
        auto clobData = clob.Read((unsigned) clob.GetLength());
        field->setBuffer(bit_cast<const uint8_t*>(clobData.data()), clobData.size(), VariantDataType::VAR_TEXT);
    }
}

void readBLOB(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto blob = resultSet.Get<Blob>(columnIndex);
    if (blob.IsNull())
    {
        field->setNull(VariantDataType::VAR_BUFFER);
    }
    else
    {
        auto blobData = blob.Read((unsigned) blob.GetLength());
        field->setBuffer(bit_cast<const uint8_t*>(blobData.data()), blobData.size(), VariantDataType::VAR_BUFFER);
    }
}

} // namespace

map<OracleOciConnection*, shared_ptr<OracleOciConnection>> OracleOciConnection::s_oracleOciConnections;

[[maybe_unused]] void* oracleCreateConnection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    const auto connection = make_shared<OracleOciConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    OracleOciConnection::s_oracleOciConnections[connection.get()] = connection;
    return connection.get();
}

[[maybe_unused]] void oracleDestroyConnection(void* connection)
{
    OracleOciConnection::s_oracleOciConnections.erase(bit_cast<OracleOciConnection*>(connection));
}
