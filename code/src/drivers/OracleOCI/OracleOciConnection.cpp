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

#include <format>
#include <sptk5/cutils>
#include <sptk5/db/OracleOciConnection.h>
#include <sptk5/db/OracleOciStatement.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;
using namespace ocilib;

namespace {
void OracleOci_readTimestamp(Resultset resultSet, sptk::DatabaseField* field, unsigned int columnIndex);
void OracleOci_readDate(Resultset resultSet, DatabaseField* field, unsigned int columnIndex);
void OracleOci_readBLOB(Resultset resultSet, DatabaseField* field, unsigned int columnIndex);
void OracleOci_readCLOB(Resultset resultSet, DatabaseField* field, unsigned int columnIndex);
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

        //Statement* createLobTable = nullptr;
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
                    //m_connection->terminateStatement(createLobTable);
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
    PoolDatabaseConnection::executeBatchSQL(batchSQL, errors);
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
}

void OracleOciConnection::driverBeginTransaction()
{
    m_connection->SetAutoCommit(false);
    PoolDatabaseConnection::driverBeginTransaction();
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
            querySetPrepared(query, true);
        }
        catch (const ocilib::Exception& e)
        {
            THROW_QUERY_ERROR(query, e.what());
        }
    }
    /*
    if (query->bulkMode())
    {
        const CParamVector& enumeratedParams = statement->enumeratedParams();
        Statement* stmt = statement->stmt();
        const auto* bulkInsertQuery = dynamic_cast<OracleBulkInsertQuery*>(query);
        if (bulkInsertQuery == nullptr)
        {
            throw Exception("Not a bulk query");
        }
        const QueryColumnTypeSizeMap& columnTypeSizes = bulkInsertQuery->columnTypeSizes();
        setMaxParamSizes(enumeratedParams, stmt, columnTypeSizes);
        stmt->setMaxIterations((unsigned) bulkInsertQuery->batchSize());
    }
     */
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
        /*
        if (query->bulkMode())
        {
            const auto* bulkInsertQuery = dynamic_cast<OracleBulkInsertQuery*>(query);
            if (bulkInsertQuery == nullptr)
            {
                throw Exception("Query is not COracleBulkInsertQuery");
            }
            statement->execBulk(getInTransaction(), bulkInsertQuery->lastIteration());
        }
        else
         */
        {
            statement->execute(getInTransaction());
        }
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
        statement->getOutputParameters(query->fields());
        return;
    }
    else
    {
        querySetActive(query, true);
        if (query->fieldCount() == 0)
        {
            const scoped_lock lock(m_mutex);
            auto resultSet = statement->resultSet();
            createQueryFieldsFromMetadata(query, resultSet);
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

VariantDataType OracleOciConnection::OracleOciTypeToVariantType(ocilib::DataType oracleType, int scale)
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
            return VAR_BOOL;
        default:
            return VAR_STRING;
    }
}

DataType OracleOciConnection::VariantTypeToOracleOciType(VariantDataType dataType)
{
    switch (dataType)
    {
        using enum sptk::VariantDataType;
        case VAR_NONE:
            throw DatabaseException("Data type is not defined");
        case VAR_INT:
        case VAR_FLOAT:
        case VAR_INT64:
            return TypeNumeric;
        case VAR_STRING:
            return TypeString;
        case VAR_TEXT:
        case VAR_BUFFER:
            return TypeLob;
        case VAR_DATE:
            return TypeDate;
        case VAR_DATE_TIME:
            return TypeTimestamp;
        case VAR_BOOL:
            return TypeBoolean;
        default:
            throw DatabaseException(format("Unsupported SPTK data type: {}", (int) dataType));
    }
}

void OracleOciConnection::createQueryFieldsFromMetadata(Query* query, Resultset resultSet)
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
        /*
        if (columnType == OCI_CDT_LONG && columnDataSize == 0)
        {
            const auto maxColumnSize = 16384;
            resultSet->setMaxColumnSize(columnIndex + 1, maxColumnSize);
        }
*/
        const VariantDataType dataType = OracleOciTypeToVariantType(columnType, columnScale);
        auto field = make_shared<DatabaseField>(columnName, columnType, dataType, columnDataSize,
                                                columnScale);
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

    auto fieldCount = query->fieldCount();
    if (fieldCount == 0)
    {
        querySetEof(query, true);
        return;
    }

    for (unsigned fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
    {
        auto* field = bit_cast<DatabaseField*>(&(*query)[fieldIndex]);

        try
        {
            // Result set column index starts from 1
            auto columnIndex = fieldIndex + 1;
            auto dataType = field->dataType();

            if (resultSet.IsColumnNull(columnIndex))
            {
                field->setNull(dataType);
                continue;
            }

            switch (field->dataType())
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
                    OracleOci_readTimestamp(resultSet, field, columnIndex);
                    break;

                case VariantDataType::VAR_BUFFER:
                    OracleOci_readBLOB(resultSet, field, columnIndex);
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

    resultSet.Next();
}

void OracleOciConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, int32_t& value)
{
}

void OracleOciConnection::queryColAttributes(Query* query, int16_t column, int16_t descType, char* buff, int len)
{
}

String OracleOciConnection::paramMark(unsigned int paramIndex)
{
    return format(":{}", paramIndex + 1);
}

String OracleOciConnection::queryError(const Query* query) const
{
    return String();
}

namespace {
void OracleOci_readTimestamp(Resultset resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto date = resultSet.Get<Date>(columnIndex);
    field->setDateTime(DateTime(date.GetYear(), date.GetMonth(), date.GetDay(), date.GetHours(), date.GetMinutes(), date.GetSeconds()), false);
}

void OracleOci_readDate(Resultset resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto date = resultSet.Get<Date>(columnIndex);
    field->setDateTime(DateTime(date.GetYear(), date.GetMonth(), date.GetDay(), short(0), short(0), short(0)), true);
}

void OracleOci_readCLOB(Resultset resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto clob = resultSet.Get<Clob>(columnIndex);
    constexpr size_t SizeBuffer = 16 * 1024 * 1024;
    auto clobData = clob.Read(SizeBuffer);
    field->setBuffer((const uint8_t*) clobData.data(), clobData.size(), VariantDataType::VAR_TEXT);
}

void OracleOci_readBLOB(Resultset resultSet, DatabaseField* field, unsigned int columnIndex)
{
    auto blob = resultSet.Get<Blob>(columnIndex);
    size_t SizeBuffer = 2048;
    if (blob.IsNull())
    {
        field->setNull(VariantDataType::VAR_BUFFER);
    }
    else
    {
        auto blobData = blob.Read(SizeBuffer);
        field->setBuffer((const uint8_t*) blobData.data(), blobData.size(), VariantDataType::VAR_BUFFER);
    }
}

} // namespace

map<OracleOciConnection*, shared_ptr<OracleOciConnection>> OracleOciConnection::s_oracleOciConnections;

[[maybe_unused]] void* oracleoci_create_connection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    auto connection = make_shared<OracleOciConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    OracleOciConnection::s_oracleOciConnections[connection.get()] = connection;
    return connection.get();
}

[[maybe_unused]] void oracleoci_destroy_connection(void* connection)
{
    OracleOciConnection::s_oracleOciConnections.erase(bit_cast<OracleOciConnection*>(connection));
}
