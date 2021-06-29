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
#include "PostgreSQLParamValues.h"
#include "htonq.h"
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

namespace sptk {

const DateTime epochDate(2000, 1, 1);
const long daysSinceEpoch = chrono::duration_cast<chrono::hours>(epochDate.timePoint().time_since_epoch()).count() / 24;
const int64_t microsecondsSinceEpoch = chrono::duration_cast<chrono::microseconds>(
    epochDate.timePoint().time_since_epoch()).count();

class PostgreSQLStatement
{
public:

    PostgreSQLStatement(bool int64timestamps, bool prepared)
        : m_paramValues(int64timestamps)
    {
        if (prepared)
        {
            stringstream str;
            str << "S" << setfill('0') << setw(4) << index;
            m_stmtName = str.str();
            ++index;
        }
    }

    void clear()
    {
        clearRows();
        m_cols = 0;
    }

    void clearRows()
    {
        m_stmt.reset();
        m_rows = 0;
        m_currentRow = -1;
    }

    void stmt(PGresult* st, unsigned rows, unsigned cols = 99999)
    {
        m_stmt = shared_ptr<PGresult>(st, [](auto* ptr) { PQclear(ptr); });
        m_rows = (int) rows;

        if (cols != 99999)
        {
            m_cols = (int) cols;
        }

        m_currentRow = -1;
    }

    [[nodiscard]] const PGresult* stmt() const
    {
        return m_stmt.get();
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
        return (unsigned) m_currentRow;
    }

    [[nodiscard]] unsigned colCount() const
    {
        return (unsigned) m_cols;
    }

    PostgreSQLParamValues& paramValues()
    {
        return m_paramValues;
    }

private:

    shared_ptr<PGresult> m_stmt;
    String m_stmtName;
    static unsigned index;
    int m_rows {0};
    int m_cols {0};
    int m_currentRow {0};
    PostgreSQLParamValues m_paramValues;
};

unsigned PostgreSQLStatement::index;

} // namespace sptk

PostgreSQLConnection::PostgreSQLConnection(const String& connectionString)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::POSTGRES)
{
}

PostgreSQLConnection::~PostgreSQLConnection()
{
    try
    {
        if (getInTransaction() && PostgreSQLConnection::active())
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

static string csParam(const string& name, const string& value)
{
    if (!value.empty())
    {
        return name + "=" + value + " ";
    }
    return "";
}

String PostgreSQLConnection::nativeConnectionString() const
{
    String port;
    const DatabaseConnectionString& connString = connectionString();

    if (connString.portNumber() != 0)
    {
        port = int2string(connString.portNumber());
    }

    string result =
        csParam("dbname", connString.databaseName()) +
        csParam("host", connString.hostName()) +
        csParam("user", connString.userName()) +
        csParam("password", connString.password()) +
        csParam("port", port);

    return result;
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
            String error = PQerrorMessage(m_connect);
            PQfinish(m_connect);
            m_connect = nullptr;
            throw DatabaseException(error);
        }

        if (m_timestampsFormat == TimestampFormat::UNKNOWN)
        {
            const char* val = PQparameterStatus(m_connect, "integer_datetimes");
            if (upperCase(val) == "ON")
            {
                m_timestampsFormat = TimestampFormat::INT64;
            }
            else
            {
                m_timestampsFormat = TimestampFormat::DOUBLE;
            }
        }
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
    return (DBHandle) m_connect;
}

bool PostgreSQLConnection::active() const
{
    return m_connect != nullptr;
}

static void checkError(const PGconn* conn, PGresult* res, const String& command,
                       ExecStatusType expectedResult = PGRES_COMMAND_OK)
{
    auto rc = PQresultStatus(res);
    if (rc != expectedResult)
    {
        String error = command + " command failed: ";
        error += PQerrorMessage(conn);
        PQclear(res);
        throw DatabaseException(error);
    }
}

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

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void PostgreSQLConnection::queryAllocStmt(Query* query)
{
    queryFreeStmt(query);
    querySetStmt(query,
                 shared_ptr<uint8_t>((StmtHandle) new PostgreSQLStatement(m_timestampsFormat == TimestampFormat::INT64,
                                                                          query->autoPrepare()),
                                     [this](StmtHandle stmtHandle) {
                                         auto* statement = (PostgreSQLStatement*) stmtHandle;
                                         if (statement->stmt() != nullptr && !statement->name().empty())
                                         {
                                             String deallocateCommand =
                                                 "DEALLOCATE \"" + statement->name() + "\"";
                                             PGresult* res = PQexec(m_connect, deallocateCommand.c_str());
                                             checkError(m_connect, res, "DEALLOCATE");
                                             PQclear(res);
                                         }
                                         delete statement;
                                     }));
}

void PostgreSQLConnection::queryFreeStmt(Query* query)
{
    scoped_lock lock(m_mutex);

    querySetStmt(query, nullptr);

    querySetPrepared(query, false);
}

void PostgreSQLConnection::queryCloseStmt(Query* query)
{
    scoped_lock lock(m_mutex);

    auto* statement = (PostgreSQLStatement*) query->statement();
    statement->clearRows();
}

void PostgreSQLConnection::queryPrepare(Query* query)
{
    if (query->prepared())
    {
        queryFreeStmt(query);
        queryAllocStmt(query);
    }

    scoped_lock lock(m_mutex);

    auto* statement = (PostgreSQLStatement*) query->statement();

    PostgreSQLParamValues& params = statement->paramValues();
    params.setParameters(query->params());

    const Oid* paramTypes = params.types();
    unsigned paramCount = params.size();

    auto* stmt = PQprepare(m_connect, statement->name().c_str(), query->sql().c_str(), (int) paramCount,
                           paramTypes);

    checkError(m_connect, stmt, "PREPARE");

    PGresult* stmt2 = PQdescribePrepared(m_connect, statement->name().c_str());
    auto fieldCount = (unsigned) PQnfields(stmt2);

    if (fieldCount != 0 && PQftype(stmt2, 0) == VOIDOID)
    {
        fieldCount = 0;
    }   // VOID result considered as no result

    PQclear(stmt2);

    statement->stmt(stmt, 0, fieldCount);

    querySetPrepared(query, true);
}

void PostgreSQLConnection::queryUnprepare(Query* query)
{
    queryFreeStmt(query);
}

int PostgreSQLConnection::queryColCount(Query* query)
{
    const auto* statement = (PostgreSQLStatement*) query->statement();

    return (int) statement->colCount();
}

void PostgreSQLConnection::queryBindParameters(Query* query)
{
    scoped_lock lock(m_mutex);

    auto* statement = (PostgreSQLStatement*) query->statement();
    PostgreSQLParamValues& paramValues = statement->paramValues();
    const CParamVector& params = paramValues.params();

    uint32_t paramNumber = 0;
    for (auto& param: params)
    {
        paramValues.setParameterValue(paramNumber, param);
        ++paramNumber;
    }

    int resultFormat = 1;   // Results are presented in binary format

    if (statement->colCount() == 0)
    {
        resultFormat = 0;
    }   // VOID result or NO results, using text format

    PGresult* stmt = PQexecPrepared(m_connect, statement->name().c_str(), (int) paramValues.size(),
                                    (const char* const*) paramValues.values(),
                                    paramValues.lengths(), paramValues.formats(), resultFormat);

    ExecStatusType rc = PQresultStatus(stmt);

    string error;
    switch (rc)
    {
        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            break;

        case PGRES_TUPLES_OK:
            statement->stmt(stmt, (unsigned) PQntuples(stmt));
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
        PQclear(stmt);
        statement->clear();
        THROW_QUERY_ERROR(query, error)
    }
}

void PostgreSQLConnection::queryExecDirect(Query* query)
{
    scoped_lock lock(m_mutex);

    auto* statement = (PostgreSQLStatement*) query->statement();
    PostgreSQLParamValues& paramValues = statement->paramValues();
    const CParamVector& params = paramValues.params();
    uint32_t paramNumber = 0;

    for (auto& param: params)
    {
        paramValues.setParameterValue(paramNumber, param);
        ++paramNumber;
    }

    int resultFormat = 1;   // Results are presented in binary format
    PGresult* stmt = PQexecParams(m_connect, query->sql().c_str(), (int) paramValues.size(), paramValues.types(),
                                  (const char* const*) paramValues.values(),
                                  paramValues.lengths(), paramValues.formats(), resultFormat);

    ExecStatusType rc = PQresultStatus(stmt);

    string error;
    switch (rc)
    {
        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            break;

        case PGRES_TUPLES_OK:
            statement->stmt(stmt, (unsigned) PQntuples(stmt), (unsigned) PQnfields(stmt));
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
        PQclear(stmt);
        statement->clear();
        THROW_QUERY_ERROR(query, error)
    }
}

void PostgreSQLConnection::PostgreTypeToCType(PostgreSQLDataType postgreType, VariantDataType& dataType)
{
    switch (postgreType)
    {
        case PostgreSQLDataType::BOOLEAN:
            dataType = VariantDataType::VAR_BOOL;
            return;

        case PostgreSQLDataType::OID:
        case PostgreSQLDataType::INT2:
        case PostgreSQLDataType::INT4:
            dataType = VariantDataType::VAR_INT;
            return;

        case PostgreSQLDataType::INT8:
            dataType = VariantDataType::VAR_INT64;
            return;

        case PostgreSQLDataType::NUMERIC:
        case PostgreSQLDataType::FLOAT4:
        case PostgreSQLDataType::FLOAT8:
            dataType = VariantDataType::VAR_FLOAT;
            return;

        case PostgreSQLDataType::BYTEA:
            dataType = VariantDataType::VAR_BUFFER;
            return;

        case PostgreSQLDataType::DATE:
            dataType = VariantDataType::VAR_DATE;
            return;

        case PostgreSQLDataType::TIME:
        case PostgreSQLDataType::TIMESTAMP:
            dataType = VariantDataType::VAR_DATE_TIME;
            return;

        default:
            dataType = VariantDataType::VAR_STRING;
            return;
    }
}

void PostgreSQLConnection::CTypeToPostgreType(VariantDataType dataType, PostgreSQLDataType& postgreType,
                                              const String& paramName)
{
    switch (dataType)
    {
        case VariantDataType::VAR_INT:
            postgreType = PostgreSQLDataType::INT4;
            return;        ///< Integer 4 bytes

        case VariantDataType::VAR_MONEY:
        case VariantDataType::VAR_FLOAT:
            postgreType = PostgreSQLDataType::FLOAT8;
            return;        ///< Floating-point (double)

        case VariantDataType::VAR_STRING:
        case VariantDataType::VAR_TEXT:
            postgreType = PostgreSQLDataType::VARCHAR;
            return;        ///< Varchar

        case VariantDataType::VAR_BUFFER:
            postgreType = PostgreSQLDataType::BYTEA;
            return;        ///< Bytea

        case VariantDataType::VAR_DATE:
        case VariantDataType::VAR_DATE_TIME:
            postgreType = PostgreSQLDataType::TIMESTAMP;
            return;        ///< Timestamp

        case VariantDataType::VAR_INT64:
            postgreType = PostgreSQLDataType::INT8;
            return;        ///< Integer 8 bytes

        case VariantDataType::VAR_BOOL:
            postgreType = PostgreSQLDataType::BOOLEAN;
            return;           ///< Boolean

        default:
            throw DatabaseException(
                "Unsupported parameter type(" + to_string((int) dataType) + ") for parameter '" + paramName + "'");
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

    const auto* statement = (const PostgreSQLStatement*) query->statement();

    auto count = (short) queryColCount(query);
    if (count < 1)
    {
        return;
    }

    querySetActive(query, true);

    if (query->fieldCount() == 0)
    {
        scoped_lock lock(m_mutex);
        // Reading the column attributes
        const PGresult* stmt = statement->stmt();

        stringstream columnName;
        columnName.fill('0');
        for (short column = 0; column < count; ++column)
        {
            columnName.str(PQfname(stmt, column));

            if (columnName.str().empty())
            {
                columnName << "column" << setw(2) << (column + 1);
            }

            auto dataType = (PostgreSQLDataType) PQftype(stmt, column);
            VariantDataType fieldType = VariantDataType::VAR_NONE;
            PostgreTypeToCType(dataType, fieldType);
            int fieldLength = PQfsize(stmt, column);
            auto* field = new DatabaseField(columnName.str(), column, (int) dataType, fieldType, fieldLength);
            query->fields().push_back(field);
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

static inline bool readBool(const char* data)
{
    return *data != char(0);
}

static inline int16_t readInt2(const char* data)
{
    return (int16_t) ntohs(*(const uint16_t*) data);
}

static inline int32_t readInt4(const char* data)
{
    return (int32_t) ntohl(*(const uint32_t*) data);
}

static inline int64_t readInt8(const char* data)
{
    return (int64_t) ntohq(*(const uint64_t*) data);
}

static inline float readFloat4(const char* data)
{
    uint32_t value = ntohl(*(const uint32_t*) data);
    void* ptr = &value;
    return *(float*) ptr;
}

static inline double readFloat8(const char* data)
{
    uint64_t value = ntohq(*(const uint64_t*) data);
    void* ptr = &value;
    return *(double*) ptr;
}

static inline DateTime readDate(const char* data)
{
    auto dt = (int32_t) ntohl(*(const uint32_t*) data);
    return epochDate + chrono::hours(dt * 24);
}

static inline DateTime readTimestamp(const char* data, bool integerTimestamps)
{
    uint64_t value = ntohq(*(const uint64_t*) data);

    if (integerTimestamps)
    {
        // time is in usecs
        return epochDate + chrono::microseconds(value);
    }

    void* ptr = &value;
    double seconds = *(double*) ptr;
    DateTime ts = epochDate + chrono::microseconds((int64_t) seconds * 1000000);
    return ts;
}

// Converts internal NUMERIC Postgresql binary to long double
static inline MoneyData readNumericToScaledInteger(const char* v)
{
    auto ndigits = (int16_t) ntohs(*(const uint16_t*) v);
    auto weight = (int16_t) ntohs(*(const uint16_t*) (v + 2));
    auto sign = (int16_t) ntohs(*(const uint16_t*) (v + 4));
    uint16_t dscale = ntohs(*(const uint16_t*) (v + 6));

    if (dscale > 16)
    {
        dscale = 16;
    }

    v += 8;
    int64_t value = 0;

    int scale = 0;
    if (weight < 0)
    {
        for (int i = 0; i < -(weight + 1); ++i)
        {
            scale += 4;
        }
    }

    int16_t digitWeight = weight;
    for (int i = 0; i < ndigits; ++i)
    {
        auto digit = (int16_t) ntohs(*(const uint16_t*) v);

        value = value * 10000 + digit;
        if (digitWeight < 0)
        {
            scale += 4;
        }

        --digitWeight;
        v += 2;
    }

    while (scale < dscale - 4)
    {
        value *= 10000;
        scale += 4;
    }

    switch (scale - dscale)
    {
        case -6:
            value *= 1000000;
            break;
        case -5:
            value *= 100000;
            break;
        case -4:
            value *= 10000;
            break;
        case -3:
            value *= 1000;
            break;
        case -2:
            value *= 100;
            break;
        case -1:
            value *= 10;
            break;
        case 1:
            value /= 10;
            break;
        case 2:
            value /= 100;
            break;
        case 3:
            value /= 1000;
            break;
        default:
            break;
    }

    if (sign != 0)
    {
        value = -value;
    }

    MoneyData moneyData = {value, uint8_t(dscale)};

    return moneyData;
}


static void decodeArray(char* data, DatabaseField* field, PostgreSQLConnection::TimestampFormat timestampFormat)
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

    auto* arrayHeader = (PGArrayHeader*) data;
    arrayHeader->dimensionNumber = ntohl(arrayHeader->dimensionNumber);
    arrayHeader->hasNull = ntohl(arrayHeader->hasNull);
    arrayHeader->elementType = ntohl(arrayHeader->elementType);
    data += sizeof(PGArrayHeader);

    auto* dimensions = (PGArrayDimension*) data;
    data += arrayHeader->dimensionNumber * sizeof(PGArrayDimension);

    stringstream output;
    for (size_t dim = 0; dim < arrayHeader->dimensionNumber; ++dim)
    {
        PGArrayDimension* dimension = dimensions + dim;
        dimension->elementCount = htonl(dimension->elementCount);
        dimension->lowerBound = htonl(dimension->lowerBound);
        output << "{";
        for (size_t element = 0; element < dimension->elementCount; ++element)
        {
            if (element != 0)
            {
                output << ",";
            }

            uint32_t dataSize = ntohl(*(const uint32_t*) data);
            data += sizeof(uint32_t);

            switch ((PostgreSQLDataType) arrayHeader->elementType)
            {
                case PostgreSQLDataType::INT2:
                    output << readInt2(data);
                    break;

                case PostgreSQLDataType::INT4:
                    output << readInt4(data);
                    break;

                case PostgreSQLDataType::INT8:
                    output << readInt8(data);
                    break;

                case PostgreSQLDataType::FLOAT4:
                    output << readFloat4(data);
                    break;

                case PostgreSQLDataType::FLOAT8:
                    output << readFloat8(data);
                    break;

                case PostgreSQLDataType::TEXT:
                case PostgreSQLDataType::CHAR:
                case PostgreSQLDataType::VARCHAR:
                    output << string(data, dataSize);
                    break;

                case PostgreSQLDataType::DATE:
                    output << readDate(data).dateString();
                    break;

                case PostgreSQLDataType::TIMESTAMPTZ:
                case PostgreSQLDataType::TIMESTAMP:
                    output << readTimestamp(data, timestampFormat ==
                                                  PostgreSQLConnection::TimestampFormat::INT64).dateString();
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

void PostgreSQLConnection::queryFetch(Query* query)
{
    if (!query->active())
    THROW_QUERY_ERROR(query, "Dataset isn't open")

    scoped_lock lock(m_mutex);

    auto* statement = (PostgreSQLStatement*) query->statement();
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

    auto fieldCount = (int) query->fieldCount();
    int dataLength = 0;

    if (fieldCount == 0)
    {
        return;
    }

    DatabaseField* field = nullptr;
    const PGresult* stmt = statement->stmt();
    auto currentRow = (int) statement->currentRow();

    for (int column = 0; column < fieldCount; ++column)
    {
        try
        {
            field = (DatabaseField*) &(*query)[column];
            auto fieldType = (PostgreSQLDataType) field->fieldType();

            dataLength = PQgetlength(stmt, currentRow, column);

            if (dataLength == 0)
            {
                VariantDataType dataType;
                PostgreTypeToCType(fieldType, dataType);

                bool isNull = true;
                if ((int) dataType & ((int) VariantDataType::VAR_STRING | (int) VariantDataType::VAR_TEXT |
                                      (int) VariantDataType::VAR_BUFFER))
                {
                    isNull = PQgetisnull(stmt, currentRow, column) == 1;
                }

                if (isNull)
                {
                    field->setNull(dataType);
                }
                else
                {
                    static array<char, 2> emptyString {};
                    field->setExternalBuffer((uint8_t*) emptyString.data(), 0,
                                             VariantDataType::VAR_STRING); // External string
                }
            }
            else
            {
                char* data = PQgetvalue(stmt, currentRow, column);

                switch (fieldType)
                {
                    case PostgreSQLDataType::BOOLEAN:
                        field->setBool(readBool(data));
                        break;

                    case PostgreSQLDataType::INT2:
                        field->setInteger(readInt2(data));
                        break;

                    case PostgreSQLDataType::OID:
                    case PostgreSQLDataType::INT4:
                        field->setInteger(readInt4(data));
                        break;

                    case PostgreSQLDataType::INT8:
                        field->setInt64(readInt8(data));
                        break;

                    case PostgreSQLDataType::FLOAT4:
                        field->setFloat(readFloat4(data));
                        break;

                    case PostgreSQLDataType::FLOAT8:
                        field->setFloat(readFloat8(data));
                        break;

                    case PostgreSQLDataType::NUMERIC:
                        field->setMoney(readNumericToScaledInteger(data));
                        break;

                    case PostgreSQLDataType::BYTEA:
                        field->setExternalBuffer((uint8_t*) data, (size_t) dataLength,
                                                 VariantDataType::VAR_BUFFER); // External buffer
                        break;

                    case PostgreSQLDataType::DATE:
                        field->setDateTime(readDate(data));
                        break;

                    case PostgreSQLDataType::TIMESTAMPTZ:
                    case PostgreSQLDataType::TIMESTAMP:
                        field->setDateTime(readTimestamp(data, m_timestampsFormat == TimestampFormat::INT64));
                        break;

                    case PostgreSQLDataType::CHAR_ARRAY:
                    case PostgreSQLDataType::INT2_VECTOR:
                    case PostgreSQLDataType::INT2_ARRAY:
                    case PostgreSQLDataType::INT4_ARRAY:
                    case PostgreSQLDataType::TEXT_ARRAY:
                    case PostgreSQLDataType::VARCHAR_ARRAY:
                    case PostgreSQLDataType::INT8_ARRAY:
                    case PostgreSQLDataType::FLOAT4_ARRAY:
                    case PostgreSQLDataType::FLOAT8_ARRAY:
                    case PostgreSQLDataType::TIMESTAMP_ARRAY:
                    case PostgreSQLDataType::TIMESTAMPTZ_ARRAY:
                        decodeArray(data, field, m_timestampsFormat);
                        break;

                    default:
                        field->setExternalBuffer((uint8_t*) data, size_t(dataLength),
                                                 VariantDataType::VAR_STRING); // External string
                        break;
                }
            }
        }
        catch (const Exception& e)
        {
            THROW_QUERY_ERROR(query, "Can't read field " << field->fieldName() << ": " << e.what())
        }
    }
}

void PostgreSQLConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string tablesSQL("SELECT table_schema || '.' || table_name "
                     "FROM information_schema.tables "
                     "WHERE table_schema NOT IN ('information_schema','pg_catalog') ");
    string objectsSQL;
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
        objects.push_back(query[uint32_t(0)].asString());
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
    array<char, 16> mark;
    snprintf(mark.data(), sizeof(mark), "$%i", paramIndex + 1);
    return String(mark.data());
}

static void appendTSV(Buffer& dest, const VariantVector& row)
{
    bool firstValue = true;
    for (const auto& value: row)
    {
        if (firstValue)
        {
            firstValue = false;
        }
        else
        {
            dest.append(char('\t'));
        }
        if (value.isNull())
        {
            dest.append("\\N", 2);
            continue;
        }
        if ((int) value.dataType() &
            ((int) VariantDataType::VAR_BUFFER | (int) VariantDataType::VAR_STRING | (int) VariantDataType::VAR_TEXT))
        {
            dest.append(escapeSQLString(value.asString(), true));
        }
        else
        {
            dest.append(value.asString());
        }
    }
    dest.append(char('\n'));
}

void PostgreSQLConnection::_bulkInsert(const String& tableName, const Strings& columnNames,
                                       const vector<VariantVector>& data)
{
    stringstream sql;
    sql << "COPY " << tableName << "(" << columnNames.join(",") << ") FROM STDIN ";

    PGresult* res = PQexec(m_connect, sql.str().c_str());
    checkError(m_connect, res, "COPY", PGRES_COPY_IN);
    PQclear(res);

    Buffer buffer;
    for (const auto& row: data)
    {
        appendTSV(buffer, row);
    }

    if (PQputCopyData(m_connect, buffer.c_str(), (int) buffer.bytes()) != 1)
    {
        String error = "COPY command send data failed: ";
        error += PQerrorMessage(m_connect);
        throw DatabaseException(error);
    }

    if (PQputCopyEnd(m_connect, nullptr) != 1)
    {
        String error = "COPY command end copy failed: ";
        error += PQerrorMessage(m_connect);
        throw DatabaseException(error);
    }

    res = PQgetResult(m_connect);
    checkError(m_connect, res, "COPY");
    PQclear(res);
}

void PostgreSQLConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    Strings statements = extractStatements(sqlBatch);

    for (const auto& stmt : statements)
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
    RegularExpression matchFunction("^(CREATE|REPLACE) .*FUNCTION", "i");
    RegularExpression matchFunctionBodyStart(R"(AS\s+(\S+)\s*$)", "i");
    RegularExpression matchStatementEnd(R"(;(\s*|\s*--.*)$)");
    RegularExpression matchCommentRow(R"(^\s*--)");

    Strings statements;
    RegularExpression::Groups matches;
    String delimiter;
    stringstream statement;

    bool functionHeader = false;
    bool functionBody = false;
    for (auto row : sqlBatch)
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

void* postgresql_create_connection(const char* connectionString)
{
    auto* connection = new PostgreSQLConnection(connectionString);
    return connection;
}

void postgresql_destroy_connection(void* connection)
{
    delete (PostgreSQLConnection*) connection;
}
