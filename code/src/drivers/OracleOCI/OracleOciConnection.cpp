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

#include "sptk5/db/OracleOciBulkInsertQuery.h"
#include "sptk5/db/OracleOciDatabaseField.h"
#include <format>
#include <sptk5/cutils>
#include <sptk5/db/OracleOciConnection.h>
#include <sptk5/db/OracleOciStatement.h>

using namespace std;
using namespace sptk;
using namespace ocilib;

namespace {
void OracleOci_readTimestamp(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex);
void OracleOci_readDateTime(const Resultset& resultSet, sptk::DatabaseField* field, unsigned int columnIndex);
void OracleOci_readDate(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex);
void OracleOci_readBLOB(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex);
void OracleOci_readCLOB(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex);
} // namespace

OracleOciConnection::OracleOciConnection(const String& connectionString, chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::ORACLE_OCI, connectTimeout)
{
    static mutex initializeMutex;
    scoped_lock lock(initializeMutex);
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

DBHandle OracleOciConnection::handle() const
{
    return PoolDatabaseConnection::handle();
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
    if (!resultSet)
    {
        return 0;
    }
    return resultSet.GetColumnCount();
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

    auto* statement = bit_cast<OracleOciStatement*>(query->statement());

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
    switch (oracleType.GetValue())
    {
        using enum sptk::VariantDataType;
        case OCI_CDT_NUMERIC:
            return scale == 0 ? VAR_INT64 : VAR_FLOAT;
        case OCI_CDT_DATETIME:
        case OCI_CDT_TIMESTAMP:
            return VAR_DATE_TIME;
        case OCI_CDT_LOB:
            return VAR_BUFFER;
        case OCI_CDT_BOOLEAN:
            return VAR_INT;
        default:
            return VAR_STRING;
    }
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

    auto* statement = bit_cast<OracleOciStatement*>(query->statement());
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
                case VariantDataType::VAR_INT:
                    field->setInteger(resultSet.Get<int>(columnIndex));
                    break;

                case VariantDataType::VAR_INT64:
                    field->setInt64(resultSet.Get<big_int>(columnIndex));
                    break;

                case VariantDataType::VAR_FLOAT:
                    field->setFloat(resultSet.Get<double>(columnIndex));
                    break;

                case VariantDataType::VAR_DATE:
                    OracleOci_readDate(resultSet, field, columnIndex);
                    break;

                case VariantDataType::VAR_DATE_TIME:
                    if (field->sqlType() == "timestamp")
                    {
                        OracleOci_readTimestamp(resultSet, field, columnIndex);
                    }
                    else if (field->sqlType() == "datetime")
                    {
                        OracleOci_readDate(resultSet, field, columnIndex);
                    }
                    else
                    {
                        OracleOci_readDateTime(resultSet, field, columnIndex);
                    }
                    break;

                case VariantDataType::VAR_BUFFER:
                    switch (field->fieldType())
                    {
                        case OCI_CDT_LOB:
                            if (field->sqlType() == "clob")
                            {
                                OracleOci_readCLOB(resultSet, field, columnIndex);
                            }
                            else
                            {
                                OracleOci_readBLOB(resultSet, field, columnIndex);
                            }
                            break;
                        case OCI_CDT_TEXT:
                            field->setString(resultSet.Get<string>(columnIndex));
                            break;
                        default:
                            throw DatabaseException("Unsupported buffer type");
                    }
                    break;

                case VariantDataType::VAR_TEXT:
                    OracleOci_readCLOB(resultSet, field, columnIndex);
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

void OracleOciConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
    notImplemented("queryColAttributes");
}

void OracleOciConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len)
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
void OracleOci_readTimestamp(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex)
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

void OracleOci_readDateTime(const Resultset& resultSet, sptk::DatabaseField* field, unsigned int columnIndex)
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

void OracleOci_readDate(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex)
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

void OracleOci_readCLOB(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto clob = resultSet.Get<Clob>(columnIndex);
    if (clob.IsNull())
    {
        field->setNull(VariantDataType::VAR_TEXT);
    }
    else
    {
        auto clobData = clob.Read(clob.GetLength());
        field->setBuffer((const uint8_t*) clobData.data(), clobData.size(), VariantDataType::VAR_TEXT);
    }
}

void OracleOci_readBLOB(const Resultset& resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto blob = resultSet.Get<Blob>(columnIndex);
    if (blob.IsNull())
    {
        field->setNull(VariantDataType::VAR_BUFFER);
    }
    else
    {
        auto blobData = blob.Read(blob.GetLength());
        field->setBuffer((const uint8_t*) blobData.data(), blobData.size(), VariantDataType::VAR_BUFFER);
    }
}

} // namespace

void OracleOciConnection::bulkInsert(const String& fullTableName, const Strings& columnNames,
                                     const vector<VariantVector>& data)
{
    const RegularExpression matchTableAndSchema(R"(^(\S+\.)?(\S+)$)");

    String schema;
    String tableName;
    const auto matches = matchTableAndSchema.m(fullTableName.toUpperCase());
    if (!matches[0].value.empty())
    {
        schema = matches[0].value;
        schema = schema.substr(0, schema.length() - 1);
    }
    tableName = matches[1].value;

    stringstream columnsInfoSQL;
    columnsInfoSQL << "SELECT column_name, data_type, data_length FROM ";
    if (schema.empty())
    {
        // User' database table
        columnsInfoSQL << "user_tab_columns WHERE table_name = :table_name";
    }
    else
    {
        columnsInfoSQL << "all_tab_columns WHERE owner = :schema AND table_name = :table_name";
    }

    Query tableColumnsQuery(this, columnsInfoSQL.str());
    tableColumnsQuery.param("table_name") = upperCase(tableName);
    if (!schema.empty())
    {
        tableColumnsQuery.param("schema") = schema;
    }
    tableColumnsQuery.open();
    const Field& column_name = tableColumnsQuery["column_name"];
    const Field& data_type = tableColumnsQuery["data_type"];
    const Field& data_length = tableColumnsQuery["data_length"];
    QueryColumnTypeSizeMap columnTypeSizeMap;
    while (!tableColumnsQuery.eof())
    {
        const auto columnName = column_name.asString();
        const auto columnType = data_type.asString();
        const auto maxDataLength = (size_t) data_length.asInteger();
        QueryColumnTypeSize columnTypeSize = {};
        columnTypeSize.type = VariantDataType::VAR_STRING;
        columnTypeSize.length = 0;
        if (columnType.find("LOB") != string::npos)
        {
            constexpr size_t defaultTextSize = 65536;
            columnTypeSize.type = VariantDataType::VAR_TEXT;
            columnTypeSize.length = defaultTextSize;
        }
        else if (columnType.find("CHAR") != string::npos)
        {
            columnTypeSize.length = maxDataLength;
        }
        else if (columnType.find("TIMESTAMP") != string::npos)
        {
            columnTypeSize.type = VariantDataType::VAR_DATE_TIME;
        }
        columnTypeSizeMap[columnName] = columnTypeSize;
        tableColumnsQuery.fetch();
    }
    tableColumnsQuery.close();

    bool wasInTransaction = inTransaction();
    if (!wasInTransaction)
    {
        beginTransaction();
    }

    QueryColumnTypeSizeVector columnTypeSizeVector;
    for (const auto& columnName: columnNames)
    {
        const auto column = columnTypeSizeMap.find(upperCase(columnName));
        if (column == columnTypeSizeMap.end())
        {
            throw DatabaseException(
                "Column '" + columnName + "' doesn't belong to table " + tableName);
        }
        columnTypeSizeVector.push_back(column->second);
    }

    OracleOciBulkInsertQuery insertQuery(this,
                                         "INSERT INTO " + tableName + "(" + columnNames.join(",") +
                                             ") VALUES (:" + columnNames.join(",:") + ")",
                                         data.size(),
                                         columnTypeSizeMap);
    for (const auto& row: data)
    {
        bulkInsertSingleRow(columnNames, columnTypeSizeVector, insertQuery, row);
    }

    if (!wasInTransaction)
    {
        commitTransaction();
    }
}

void OracleOciConnection::bulkInsertSingleRow(const Strings& columnNames,
                                              const QueryColumnTypeSizeVector& columnTypeSizeVector,
                                              OracleOciBulkInsertQuery& insertQuery, const VariantVector& row)
{
    for (size_t i = 0; i < columnNames.size(); ++i)
    {
        const auto& value = row[i];
        switch (columnTypeSizeVector[i].type)
        {
            using enum sptk::VariantDataType;
            case VAR_TEXT:
                if (value.dataSize())
                {
                    insertQuery.param(i).setBuffer(bit_cast<const uint8_t*>(value.getText()), value.dataSize(),
                                                   VAR_TEXT);
                }
                else
                {
                    insertQuery.param(i).setNull(VAR_TEXT);
                }
                break;
            case VAR_DATE_TIME:
                insertQuery.param(i).setDateTime(value.get<DateTime>());
                break;
            default:
                insertQuery.param(i).setString(value.asString());
                break;
        }
    }
    insertQuery.execNext();
}

map<OracleOciConnection*, shared_ptr<OracleOciConnection>> OracleOciConnection::s_oracleOciConnections;

[[maybe_unused]] [[maybe_unused]] void* oracleoci_create_connection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    auto connection = make_shared<OracleOciConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    OracleOciConnection::s_oracleOciConnections[connection.get()] = connection;
    return connection.get();
}

[[maybe_unused]] [[maybe_unused]] void oracleoci_destroy_connection(void* connection)
{
    OracleOciConnection::s_oracleOciConnections.erase(bit_cast<OracleOciConnection*>(connection));
}
