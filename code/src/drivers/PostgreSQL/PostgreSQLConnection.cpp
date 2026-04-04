/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-06                                             ║
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

#include "PostgreSQLParamValues.h"
#include "htonq.h"
#include "sptk5/db/BulkQuery.h"
#include <sptk5/cutils>
#include <sptk5/db/DatabaseField.h>

using namespace std;
using namespace sptk;

namespace sptk {

constexpr auto hoursPerDay = 24;
const DateTime g_epochDate(2000, 1, 1);

class PostgreSQLStatement
{
public:
    PostgreSQLStatement(PGconn* connect, bool int64timestamps, bool prepared, const DateTime& epochDate)
        : m_connect(connect)
        , m_paramValues(int64timestamps, epochDate)
    {
        if (prepared)
        {
            m_stmtName = format("S{}", nextIndex());
        }
    }

    PostgreSQLStatement(const PostgreSQLStatement&) = delete;

    ~PostgreSQLStatement()
    {
        if (stmt() != nullptr && !name().empty())
        {
            const String deallocateCommand = "DEALLOCATE \"" + name() + "\"";
            PGresult*    res = PQexec(m_connect, deallocateCommand.c_str());
            PQclear(res);
        }

        clear();
    }

    PostgreSQLStatement& operator=(const PostgreSQLStatement&) = delete;

    void clear()
    {
        if (m_stmt)
        {
            PQclear(m_stmt);
            m_stmt = nullptr;
        }
        clearRows();
        m_cols = 0;
    }

    void clearRows()
    {
        m_rows = 0;
        m_currentRow = -1;
    }

    void stmt(PGresult* result, unsigned rows, unsigned cols = static_cast<unsigned>(-1))
    {
        if (m_stmt)
        {
            PQclear(m_stmt);
        }
        m_stmt = result;

        m_rows = static_cast<int>(rows);

        if (cols != static_cast<unsigned>(-1))
        {
            m_cols = static_cast<int>(cols);
        }

        m_currentRow = -1;
    }

    [[nodiscard]] const PGresult* stmt() const
    {
        return m_stmt;
    }

    [[nodiscard]] String name() const
    {
        return m_stmtName;
    }

    void fetch()
    {
        ++m_currentRow;
    }

    [[nodiscard]] bool eof() const
    {
        return m_currentRow >= m_rows;
    }

    [[nodiscard]] unsigned currentRow() const
    {
        return static_cast<unsigned>(m_currentRow);
    }

    [[nodiscard]] unsigned colCount() const
    {
        return static_cast<unsigned>(m_cols);
    }

    [[nodiscard]] PostgreSQLParamValues& paramValues()
    {
        return m_paramValues;
    }

private:
    PGconn*               m_connect {nullptr};
    PGresult*             m_stmt {nullptr};
    String                m_stmtName;
    int                   m_rows {0};
    int                   m_cols {0};
    int                   m_currentRow {0};
    PostgreSQLParamValues m_paramValues;

    static unsigned nextIndex();
};

unsigned PostgreSQLStatement::nextIndex()
{
    static mutex    amutex;
    static unsigned index = 0;

    const scoped_lock lock(amutex);
    return index++;
}

} // namespace sptk

PostgreSQLConnection::PostgreSQLConnection(const String& connectionString, std::chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::POSTGRES, connectTimeout)
{
}

PostgreSQLConnection::~PostgreSQLConnection()
{
    try
    {
        if (PostgreSQLConnection::active())
        {
            if (getInTransaction())
            {
                rollbackTransaction();
            }
            close();
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

namespace {
string csParam(const string& name, const string& value)
{
    return value.empty() ? "" : name + "=" + value + " ";
}
} // namespace

String PostgreSQLConnection::nativeConnectionString() const
{
    String                          port;
    const DatabaseConnectionString& connString = connectionString();

    if (connString.portNumber() != 0)
    {
        port = int2string(connString.portNumber());
    }

    const string result =
        csParam("dbname", connString.databaseName()) +
        csParam("host", connString.hostName()) +
        csParam("user", connString.userName()) +
        csParam("password", connString.password()) +
        csParam("port", port);

    return result;
}

chrono::minutes PostgreSQLConnection::getTimezoneOffset()
{
    Query query(this, "SELECT CAST(EXTRACT(TIMEZONE FROM CURRENT_TIME) AS INT)");
    auto  tzOffset = query.scalar().asInteger() / 60;

    return chrono::minutes(tzOffset);
}

void PostgreSQLConnection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
        setInTransaction(false);

        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        m_connect = PQconnectdb(nativeConnectionString().c_str());

        if (PQstatus(m_connect) != CONNECTION_OK)
        {
            const String error = PQerrorMessage(m_connect);
            PQfinish(m_connect);
            m_connect = nullptr;
            throw DatabaseException(error);
        }

        if (m_timestampsFormat == TimestampFormat::UNKNOWN)
        {
            const char* val = PQparameterStatus(m_connect, "integer_datetimes");
            if (val == nullptr || upperCase(val) == "ON")
            {
                m_timestampsFormat = TimestampFormat::INT64;
            }
            else
            {
                m_timestampsFormat = TimestampFormat::DOUBLE;
            }
        }
        m_sessionTimezoneOffset = getTimezoneOffset();
        m_epochDate = g_epochDate + m_sessionTimezoneOffset;
    }
}

void PostgreSQLConnection::closeDatabase()
{
    disconnectAllQueries();
    if (m_connect)
    {
        PQfinish(m_connect);
        m_connect = nullptr;
    }
}

DBHandle PostgreSQLConnection::handle() const
{
    return bit_cast<DBHandle>(m_connect);
}

bool PostgreSQLConnection::active() const
{
    return m_connect != nullptr && PQstatus(m_connect) == CONNECTION_OK;
}

namespace {
void checkError(const PGconn* conn, PGresult* res, const String& command)
{
    if (res == nullptr)
    {
        throw DatabaseException(command + " command failed: Out of memory");
    }

    if (const auto statusCode = PQresultStatus(res);
        statusCode != PGRES_COMMAND_OK)
    {
        const auto error = command + " command failed: " + string(PQerrorMessage(conn));
        PQclear(res);
        throw DatabaseException(error);
    }
}
} // namespace

void PostgreSQLConnection::driverBeginTransaction()
{
    if (m_connect == nullptr)
    {
        open();
    }

    if (getInTransaction())
    {
        throw DatabaseException("Transaction already started.");
    }

    PGresult* res = PQexec(m_connect, "BEGIN");
    checkError(m_connect, res, "BEGIN");
    PQclear(res);

    setInTransaction(true);
}

void PostgreSQLConnection::driverEndTransaction(bool commit)
{
    if (!getInTransaction())
    {
        throw DatabaseException("Transaction isn't started.");
    }

    string action;

    if (commit)
    {
        action = "COMMIT";
    }
    else
    {
        action = "ROLLBACK";
    }

    PGresult* res = PQexec(m_connect, action.c_str());
    checkError(m_connect, res, action);
    PQclear(res);

    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------

String PostgreSQLConnection::queryError(const Query*) const
{
    return PQerrorMessage(m_connect);
}

// Doesn't allocate stmt but makes sure the previously allocated stmt is released
void PostgreSQLConnection::queryAllocStmt(Query* query)
{
    queryFreeStmt(query);
    const auto stmt = make_shared<PostgreSQLStatement>(m_connect, m_timestampsFormat == TimestampFormat::INT64,
                                                       query->autoPrepare(), m_epochDate - m_sessionTimezoneOffset);
    querySetStmt(query, reinterpret_pointer_cast<uint8_t>(stmt));
}

void PostgreSQLConnection::queryFreeStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    querySetStmt(query, nullptr);

    querySetPrepared(query, false);
}

void PostgreSQLConnection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    if (auto* statement = bit_cast<PostgreSQLStatement*>(query->statement()))
    {
        statement->clearRows();
    }
}

void PostgreSQLConnection::queryPrepare(Query* query)
{
    if (query->prepared())
    {
        queryFreeStmt(query);
        queryAllocStmt(query);
    }

    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());

    PostgreSQLParamValues& params = statement->paramValues();
    params.setParameters(query->params());

    const Oid* paramTypes = params.types();
    const auto paramCount = params.size();

    auto* stmt = PQprepare(m_connect, statement->name().c_str(), query->sql().c_str(), static_cast<int>(paramCount),
                           paramTypes);

    if (stmt == nullptr)
    {
        const String error = "PREPARE command failed: " + string(PQerrorMessage(m_connect));
        throw DatabaseException(error);
    }

    checkError(m_connect, stmt, "PREPARE");

    PGresult* stmt2 = PQdescribePrepared(m_connect, statement->name().c_str());
    if (stmt2 == nullptr)
    {
        const string error = format("DESCRIBE command failed: {}", PQerrorMessage(m_connect));
        PQclear(stmt);
        throw DatabaseException(format("DESCRIBE command failed: {}", PQerrorMessage(m_connect)));
    }

    auto fieldCount = static_cast<unsigned>(PQnfields(stmt2));

    if (fieldCount != 0 && PQftype(stmt2, 0) == VOIDOID)
    {
        fieldCount = 0;
    } // VOID result considered as no result

    PQclear(stmt2);

    statement->stmt(stmt, 0, fieldCount);

    querySetPrepared(query, true);
}

size_t PostgreSQLConnection::queryColCount(Query* query)
{
    const auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());

    return statement->colCount();
}

void PostgreSQLConnection::queryBindParameters(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto*       statement = bit_cast<PostgreSQLStatement*>(query->statement());
    auto&       paramValues = statement->paramValues();
    const auto& params = paramValues.params();

    uint32_t paramNumber = 0;
    for (const auto& param: params)
    {
        paramValues.setParameterValue(paramNumber, param);
        ++paramNumber;
    }

    auto resultFormat = 1; // Results are presented in binary format

    if (statement->colCount() == 0)
    {
        resultFormat = 0;
    } // VOID result or NO results, using text format

    PGresult* stmt = PQexecPrepared(m_connect, statement->name().c_str(), static_cast<int>(paramValues.size()),
                                    bit_cast<const char* const*>(paramValues.values()),
                                    paramValues.lengths(), paramValues.formats(), resultFormat);

    const ExecStatusType statusType = PQresultStatus(stmt);

    string error;
    switch (statusType)
    {
        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            break;

        case PGRES_TUPLES_OK:
            statement->stmt(stmt, static_cast<unsigned>(PQntuples(stmt)));
            break;

        case PGRES_EMPTY_QUERY:
            statement->stmt(stmt, 0, 0);
            error = "EXECUTE command failed: EMPTY QUERY";
            break;

        default:
            statement->stmt(stmt, 0, 0);
            error = "EXECUTE command failed: ";
            error += PQerrorMessage(m_connect);
            break;
    }

    if (!error.empty())
    {
        statement->clear();
        THROW_QUERY_ERROR(query, error);
    }
}

void PostgreSQLConnection::queryExecDirect(const Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());
    auto& paramValues = statement->paramValues();
    paramValues.setParameters(query->params());
    const auto& params = paramValues.params();
    uint32_t    paramNumber = 0;

    for (const auto& param: params)
    {
        paramValues.setParameterValue(paramNumber, param);
        ++paramNumber;
    }

    const auto resultFormat = 1; // Results are presented in binary format
    PGresult*  stmt = PQexecParams(m_connect, query->sql().c_str(), static_cast<int>(paramValues.size()), paramValues.types(),
                                   (const char* const*) paramValues.values(),
                                   paramValues.lengths(), paramValues.formats(), resultFormat);

    const ExecStatusType statusCode = PQresultStatus(stmt);

    string error;
    switch (statusCode)
    {
        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            break;

        case PGRES_TUPLES_OK:
            statement->stmt(stmt, static_cast<unsigned>(PQntuples(stmt)), static_cast<unsigned>(PQnfields(stmt)));
            break;

        case PGRES_EMPTY_QUERY:
            error = "EXECUTE command failed: EMPTY QUERY";
            break;

        default:
            error = "EXECUTE command failed: ";
            error += PQerrorMessage(m_connect);
            break;
    }

    if (!error.empty())
    {
        statement->clear();
        THROW_QUERY_ERROR(query, error);
    }
}

void PostgreSQLConnection::postgreTypeToVariantType(PostgreSQLDataType postgreType, VariantDataType& dataType)
{
    switch (postgreType)
    {
        using enum sptk::VariantDataType;
        using enum PostgreSQLDataType;
        case BOOLEAN:
            dataType = VAR_BOOL;
            break;

        case OID:
        case INT2:
        case INT4:
            dataType = VAR_INT;
            break;

        case INT8:
            dataType = VAR_INT64;
            break;

        case NUMERIC:
        case FLOAT4:
        case FLOAT8:
            dataType = VAR_FLOAT;
            break;

        case BYTEA:
            dataType = VAR_BUFFER;
            break;

        case DATE:
            dataType = VAR_DATE;
            break;

        case TIME:
        case TIMESTAMP:
            dataType = VAR_DATE_TIME;
            break;

        default:
            dataType = VAR_STRING;
            break;
    }
}

void PostgreSQLConnection::variantTypeToPostgreType(VariantDataType dataType, PostgreSQLDataType& postgreType,
                                                    const String& paramName)
{
    switch (dataType)
    {
        using enum PostgreSQLDataType;
        using enum sptk::VariantDataType;
        case VAR_INT:
            postgreType = INT4;
            break; ///< Integer 4 bytes

        case VAR_MONEY:
        case VAR_FLOAT:
            postgreType = FLOAT8;
            break; ///< Floating-point (double)

        case VAR_STRING:
        case VAR_TEXT:
            postgreType = VARCHAR;
            break; ///< Varchar

        case VAR_BUFFER:
            postgreType = BYTEA;
            break; ///< Bytea

        case VAR_DATE:
        case VAR_DATE_TIME:
            postgreType = TIMESTAMP;
            break; ///< Timestamp

        case VAR_INT64:
            postgreType = INT8;
            break; ///< Integer 8 bytes

        case VAR_BOOL:
            postgreType = BOOLEAN;
            break; ///< Boolean

        default:
            throw DatabaseException(
                format("Unsupported parameter type {} for parameter '{}'", static_cast<int>(dataType), paramName.c_str()));
    }
}

void PostgreSQLConnection::queryOpen(Query* query)
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

    if (query->autoPrepare())
    {
        if (!query->prepared())
        {
            queryPrepare(query);
        }
        queryBindParameters(query);
    }
    else
    {
        queryExecDirect(query);
    }

    const auto* statement = bit_cast<const PostgreSQLStatement*>(query->statement());

    const auto columnCount = static_cast<short>(queryColCount(query));
    if (columnCount < 1)
    {
        return;
    }

    querySetActive(query, true);

    if (query->fieldCount() == 0)
    {
        const scoped_lock lock(m_mutex);
        // Reading the column attributes
        const PGresult* stmt = statement->stmt();
        auto&           fields = query->fields();

        for (short column = 0; column < columnCount; ++column)
        {
            String columnName = PQfname(stmt, column);

            if (columnName.empty())
            {
                columnName = format("column_{}", column + 1);
            }

            auto dataType = static_cast<PostgreSQLDataType>(PQftype(stmt, column));
            auto fieldType = VariantDataType::VAR_NONE;
            postgreTypeToVariantType(dataType, fieldType);
            const auto fieldLength = PQfsize(stmt, column);
            auto       field = make_shared<DatabaseField>(columnName, static_cast<int>(dataType), fieldType, fieldLength);
            fields.push_back(field);
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

namespace {
[[nodiscard]] bool readBool(const char* data)
{
    return *data != static_cast<char>(0);
}

int16_t readInt2(const char* data)
{
    return static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(data)));
}

int32_t readInt4(const char* data)
{
    return static_cast<int32_t>(ntohl(*bit_cast<const uint32_t*>(data)));
}

int64_t readInt8(const char* data)
{
    return bit_cast<int64_t>(ntohq(*bit_cast<const uint64_t*>(data)));
}

float readFloat4(const char* data)
{
    auto  value = ntohl(*bit_cast<const uint32_t*>(data));
    void* ptr = &value;
    return *bit_cast<float*>(ptr);
}

double readFloat8(const char* data)
{
    auto  value = ntohq(*bit_cast<const uint64_t*>(data));
    void* ptr = &value;
    return *bit_cast<double*>(ptr);
}

DateTime readDate(const char* data, const DateTime& epochDate)
{
    const auto dateTime = static_cast<int32_t>(ntohl(*bit_cast<const uint32_t*>(data)));
    return epochDate + chrono::hours(dateTime * hoursPerDay);
}

DateTime readTimestamp(const char* data, bool integerTimestamps, const DateTime& epochDate)
{
    const auto           value = ntohq(*bit_cast<const uint64_t*>(data));
    chrono::microseconds epochOffset;

    if (integerTimestamps)
    {
        // time is in usecs
        epochOffset = chrono::microseconds(value);
    }
    else
    {
        const void* ptr = &value;
        const auto  seconds = *bit_cast<const double*>(ptr);
        epochOffset = chrono::seconds(static_cast<int>(seconds));
    }
    return epochDate + epochOffset;
}

// Converts internal NUMERIC Postgresql binary to long double
MoneyData readNumericToScaledInteger(const char* numeric)
{
    const auto ndigits = static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(numeric)));
    const auto weight = static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(numeric + 2)));
    const auto sign = static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(numeric + 4)));
    auto       dscale = ntohs(*bit_cast<const uint16_t*>(numeric + 6));

    if (constexpr auto maxDscale = 16;
        dscale > maxDscale)
    {
        dscale = maxDscale;
    }

    constexpr auto digitsOffset = 8;
    numeric += digitsOffset;
    int64_t value = 0;

    constexpr auto scaleShift = 4;
    auto           scale = 0;
    if (weight < 0)
    {
        for (auto i = 0; i < -(weight + 1); ++i)
        {
            scale += scaleShift;
        }
    }

    constexpr auto digitMultiplier = 10000;
    auto           digitWeight = weight;
    for (auto i = 0; i < ndigits; ++i)
    {
        const auto digit = static_cast<int16_t>(ntohs(*bit_cast<const uint16_t*>(numeric)));

        value = value * digitMultiplier + digit;
        if (digitWeight < 0)
        {
            scale += scaleShift;
        }

        --digitWeight;
        numeric += sizeof(int16_t);
    }

    while (scale < dscale - scaleShift)
    {
        value *= digitMultiplier;
        scale += scaleShift;
    }

    constexpr auto multiplier1M = 1000000;
    constexpr auto multiplier100K = 100000;
    constexpr auto multiplier10K = 10000;
    constexpr auto multiplier1K = 1000;
    constexpr auto multiplier100 = 100;
    constexpr auto multiplier10 = 10;

    constexpr auto scaleMinus6 = -6;
    constexpr auto scaleMinus5 = -5;
    constexpr auto scaleMinus4 = -4;
    constexpr auto scaleMinus3 = -3;
    constexpr auto scaleMinus2 = -2;
    constexpr auto scaleMinus1 = -1;
    constexpr auto scalePlus1 = 1;
    constexpr auto scalePlus2 = 2;
    constexpr auto scalePlus3 = 3;

    switch (scale - dscale)
    {
        case scaleMinus6:
            value *= multiplier1M;
            break;
        case scaleMinus5:
            value *= multiplier100K;
            break;
        case scaleMinus4:
            value *= multiplier10K;
            break;
        case scaleMinus3:
            value *= multiplier1K;
            break;
        case scaleMinus2:
            value *= multiplier100;
            break;
        case scaleMinus1:
            value *= multiplier10;
            break;
        case scalePlus1:
            value /= multiplier10;
            break;
        case scalePlus2:
            value /= multiplier100;
            break;
        case scalePlus3:
            value /= multiplier1K;
            break;
        default:
            break;
    }

    if (sign != 0)
    {
        value = -value;
    }

    MoneyData moneyData(value, static_cast<uint8_t>(dscale));

    return moneyData;
}

void decodeArray(char* data, DatabaseField* field, const PostgreSQLConnection::TimestampFormat timestampFormat, const DateTime& epochDate)
{
    struct PGArrayHeader
    {
        uint32_t dimensionNumber;
        uint32_t hasNull;
        uint32_t elementType;
    };

    struct PGArrayDimension
    {
        uint32_t elementCount;
        uint32_t lowerBound;
    };

    auto* arrayHeader = bit_cast<PGArrayHeader*>(data);
    arrayHeader->dimensionNumber = ntohl(arrayHeader->dimensionNumber);
    arrayHeader->hasNull = ntohl(arrayHeader->hasNull);
    arrayHeader->elementType = ntohl(arrayHeader->elementType);
    data += sizeof(PGArrayHeader);

    auto* dimensions = bit_cast<PGArrayDimension*>(data);
    data += arrayHeader->dimensionNumber * sizeof(PGArrayDimension);

    stringstream output;
    for (size_t dim = 0; dim < arrayHeader->dimensionNumber; ++dim)
    {
        PGArrayDimension* dimension = dimensions + dim;
        dimension->elementCount = ntohl(dimension->elementCount);
        dimension->lowerBound = ntohl(dimension->lowerBound);
        output << "{";
        for (size_t element = 0; element < dimension->elementCount; ++element)
        {
            if (element != 0)
            {
                output << ",";
            }

            const auto dataSize = ntohl(*bit_cast<const int32_t*>(data));
            data += sizeof(uint32_t);

            if (dataSize <= 0)
            {
                output << "NULL";
                continue;
            }

            switch (static_cast<PostgreSQLDataType>(arrayHeader->elementType))
            {
                using enum PostgreSQLDataType;
                case INT2:
                    output << readInt2(data);
                    break;

                case INT4:
                    output << readInt4(data);
                    break;

                case INT8:
                    output << readInt8(data);
                    break;

                case FLOAT4:
                    output << readFloat4(data);
                    break;

                case FLOAT8:
                    output << readFloat8(data);
                    break;

                case TEXT:
                case CHAR:
                case VARCHAR:
                    output << string(data, dataSize);
                    break;

                case DATE:
                    output << readDate(data, epochDate).dateString();
                    break;

                case TIMESTAMPTZ:
                case TIMESTAMP:
                    output << readTimestamp(data, timestampFormat == PostgreSQLConnection::TimestampFormat::INT64, epochDate)
                                  .isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS);
                    break;

                default:
                    throw DatabaseException("Unsupported array element type");
            }
            data += dataSize;
        }
        output << "}";
    }
    field->setString(output.str());
}
} // namespace

void PostgreSQLConnection::queryFetch(Query* query)
{
    if (!query->active())
    {
        THROW_QUERY_ERROR(query, "Dataset isn't open");
    }
    const scoped_lock lock(m_mutex);

    auto* statement = bit_cast<PostgreSQLStatement*>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Statement isn't open");
    }

    statement->fetch();

    if (statement->eof())
    {
        querySetEof(query, true);
        return;
    }

    const auto fieldCount = static_cast<int>(query->fieldCount());

    if (fieldCount == 0)
    {
        return;
    }

    const PGresult*       stmt = statement->stmt();
    const auto            currentRow = static_cast<int>(statement->currentRow());
    const auto            integerTimestamps = m_timestampsFormat == TimestampFormat::INT64;
    static array<char, 1> emptyString {};

    for (auto column = 0; column < fieldCount; ++column)
    {
        auto* field = bit_cast<DatabaseField*>(&(*query)[static_cast<size_t>(column)]);
        try
        {
            const auto fieldType = static_cast<PostgreSQLDataType>(field->fieldType());
            const auto dataType = field->dataType();
            const auto dataLength = PQgetlength(stmt, currentRow, column);
            const auto isNull = PQgetisnull(stmt, currentRow, column) == 1;

            if (isNull)
            {
                field->setNull(dataType);
                continue;
            }

            if (dataLength == 0)
            {
                using enum sptk::VariantDataType;
                switch (dataType)
                {
                    case VAR_STRING:
                    case VAR_TEXT:
                    case VAR_BUFFER:
                        field->setExternalBuffer(bit_cast<uint8_t*>(emptyString.data()), 0, dataType);
                        break;
                    default:
                        field->setNull(dataType);
                        break;
                }
                continue;
            }

            char* data = PQgetvalue(stmt, currentRow, column);

            switch (fieldType)
            {
                using enum PostgreSQLDataType;
                case BOOLEAN:
                    field->setBool(readBool(data));
                    break;

                case INT2:
                    field->setInteger(readInt2(data));
                    break;

                case OID:
                case INT4:
                    field->setInteger(readInt4(data));
                    break;

                case INT8:
                    field->setInt64(readInt8(data));
                    break;

                case FLOAT4:
                    field->setFloat(readFloat4(data));
                    break;

                case FLOAT8:
                    field->setFloat(readFloat8(data));
                    break;

                case NUMERIC:
                    field->setMoney(readNumericToScaledInteger(data));
                    break;

                case BYTEA:
                    field->setExternalBuffer(bit_cast<uint8_t*>(data), static_cast<size_t>(dataLength),
                                             VariantDataType::VAR_BUFFER); // External buffer
                    break;

                case DATE:
                    field->setDateTime(readDate(data, m_epochDate));
                    break;

                case TIMESTAMPTZ:
                    field->setDateTime(readTimestamp(data, integerTimestamps, m_epochDate));
                    break;
                case TIMESTAMP:
                    field->setDateTime(readTimestamp(data, integerTimestamps, g_epochDate));
                    break;

                case CHAR_ARRAY:
                case INT2_VECTOR:
                case INT2_ARRAY:
                case INT4_ARRAY:
                case TEXT_ARRAY:
                case VARCHAR_ARRAY:
                case INT8_ARRAY:
                case FLOAT4_ARRAY:
                case FLOAT8_ARRAY:
                case TIMESTAMP_ARRAY:
                case TIMESTAMPTZ_ARRAY:
                    decodeArray(data, field, m_timestampsFormat, m_epochDate);
                    break;

                default:
                    field->setExternalBuffer(bit_cast<uint8_t*>(data), static_cast<size_t>(dataLength),
                                             VariantDataType::VAR_STRING); // External string
                    break;
            }
        }
        catch (const Exception& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
    }
}

void PostgreSQLConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    const string tablesSQL("SELECT table_schema || '.' || table_name "
                           "FROM information_schema.tables "
                           "WHERE table_schema NOT IN ('information_schema','pg_catalog') ");
    string       objectsSQL;
    objects.clear();

    switch (objectType)
    {
        case DatabaseObjectType::FUNCTIONS:
            objectsSQL =
                "SELECT DISTINCT routine_schema || '.' || routine_name "
                "FROM information_schema.routines "
                "WHERE routine_schema NOT IN ('information_schema','pg_catalog') "
                "AND routine_type = 'FUNCTION'";
            break;

        case DatabaseObjectType::PROCEDURES:
            objectsSQL =
                "SELECT DISTINCT routine_schema || '.' || routine_name "
                "FROM information_schema.routines "
                "WHERE routine_schema NOT IN ('information_schema','pg_catalog') "
                "AND routine_type = 'PROCEDURE'";
            break;

        case DatabaseObjectType::VIEWS:
            objectsSQL = tablesSQL + "AND table_type = 'VIEW'";
            break;

        case DatabaseObjectType::DATABASES:
            objectsSQL =
                "SELECT datname FROM pg_database WHERE datname NOT IN ('postgres','template0','template1')";
            break;

        default:
            objectsSQL = tablesSQL + "AND table_type = 'BASE TABLE'";
            break;
    }

    Query query(this, objectsSQL);
    query.open();

    while (!query.eof())
    {
        objects.push_back(query[0].asString());
        query.next();
    }

    query.close();
}

String PostgreSQLConnection::driverDescription() const
{
    return "PostgreSQL";
}

String PostgreSQLConnection::paramMark(unsigned paramIndex)
{
    return "$" + to_string(paramIndex + 1);
}

void PostgreSQLConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    const Strings statements = extractStatements(batchSQL);

    for (const auto& stmt: statements)
    {
        try
        {
            Query query(this, stmt);
            query.exec();
        }
        catch (const Exception& e)
        {
            if (errors != nullptr)
            {
                errors->push_back(e.what());
            }
            else
            {
                throw;
            }
        }
    }
}

Strings PostgreSQLConnection::extractStatements(const Strings& sqlBatch)
{
    static const RegularExpression matchFunction("^(CREATE|REPLACE) .*FUNCTION", "i");
    static const RegularExpression matchFunctionBodyStart(R"(AS\s+(\S+)\s*$)", "i");
    static const RegularExpression matchStatementEnd(R"(;(\s*|\s*--.*)$)");
    static const RegularExpression matchCommentRow(R"(^\s*--)");

    Strings      statements;
    String       delimiter;
    stringstream statement;

    auto functionHeader = false;
    auto functionBody = false;
    for (auto row: sqlBatch)
    {
        if (!functionHeader && !functionBody)
        {
            row = row.trim();
            if (row.empty() || matchCommentRow.matches(row))
            {
                continue;
            }
        }

        if (!functionHeader && matchFunction.matches(row))
        {
            functionHeader = true;
            statement << row << "\n";
            continue;
        }

        if (functionHeader && !functionBody && matchFunctionBodyStart.matches(row))
        {
            auto matches = matchFunctionBodyStart.m(row);
            functionBody = true;
            functionHeader = false;
            delimiter = matches[0].value;
            statement << row << "\n";
            continue;
        }

        if (functionBody && row.find(delimiter) != string::npos)
        {
            delimiter = "";
            functionBody = false;
        }

        if (!functionBody && matchStatementEnd.matches(row))
        {
            statement << row;
            statements.push_back(statement.str());
            statement.str("");
            continue;
        }

        statement << row << "\n";
    }

    if (!trim(statement.str()).empty())
    {
        statements.push_back(statement.str());
    }
    return statements;
}

void PostgreSQLConnection::queryColAttributes(Query*, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void PostgreSQLConnection::queryColAttributes(Query*, int16_t, int16_t, char*, int)
{
    notImplemented("queryColAttributes");
}

SynchronizedMap<PostgreSQLConnection*, shared_ptr<PostgreSQLConnection>> PostgreSQLConnection::s_postgresqlConnections;

[[maybe_unused]] void* postgresqlCreateConnection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    const auto connection = make_shared<PostgreSQLConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    PostgreSQLConnection::s_postgresqlConnections.insert(connection.get(), connection);
    return connection.get();
}

[[maybe_unused]] void postgresqlDestroyConnection(void* connection)
{
    PostgreSQLConnection::s_postgresqlConnections.erase(bit_cast<PostgreSQLConnection*>(connection));
}
