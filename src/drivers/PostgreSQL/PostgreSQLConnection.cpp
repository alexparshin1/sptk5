/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPostgreSQLConnection.cpp - description                ║
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

#include "PostgreSQLParamValues.h"
#include "htonq.h"
#include <iomanip>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

namespace sptk {

    const DateTime epochDate(2000, 1, 1);
    const long daysSinceEpoch = chrono::duration_cast<chrono::hours>(epochDate.timePoint().time_since_epoch()).count() / 24;
    const int64_t microsecondsSinceEpoch = chrono::duration_cast<chrono::microseconds>(epochDate.timePoint().time_since_epoch()).count();

    class PostgreSQLStatement
    {
        PGresult* m_stmt;
        char m_stmtName[20];
        static unsigned index;
        int m_rows;
        int m_cols;
        int m_currentRow;
    public:
        PostgreSQLParamValues m_paramValues;
    public:

        PostgreSQLStatement(bool int64timestamps, bool prepared)
            : m_stmt(nullptr), m_stmtName(), m_rows(0), m_cols(0), m_currentRow(0), m_paramValues(int64timestamps)
        {
            if (prepared)
                snprintf(m_stmtName, sizeof(m_stmtName), "S%04u", ++index);
        }

        ~PostgreSQLStatement()
        {
            if (m_stmt != nullptr)
                PQclear(m_stmt);
        }

        void clear()
        {
            clearRows();
            m_cols = 0;
        }

        void clearRows()
        {
            if (m_stmt != nullptr) {
                PQclear(m_stmt);
                m_stmt = nullptr;
            }

            m_rows = 0;
            m_currentRow = -1;
        }

        void stmt(PGresult* st, unsigned rows, unsigned cols = 99999)
        {
            if (m_stmt != nullptr)
                PQclear(m_stmt);

            m_stmt = st;
            m_rows = (int) rows;

            if (cols != 99999)
                m_cols = (int) cols;

            m_currentRow = -1;
        }

        const PGresult* stmt() const
        {
            return m_stmt;
        }

        string name() const
        {
            return m_stmtName;
        }

        void fetch()
        {
            m_currentRow++;
        }

        bool eof()
        {
            return m_currentRow >= m_rows;
        }

        unsigned currentRow() const
        {
            return (unsigned) m_currentRow;
        }

        unsigned colCount() const
        {
            return (unsigned) m_cols;
        }

    };

    unsigned PostgreSQLStatement::index;

} // namespace sptk

enum PostgreSQLTimestampFormat
{
    PG_UNKNOWN_TIMESTAMPS, PG_DOUBLE_TIMESTAMPS, PG_INT64_TIMESTAMPS
};
static PostgreSQLTimestampFormat timestampsFormat;

PostgreSQLConnection::PostgreSQLConnection(const String& connectionString)
: PoolDatabaseConnection(connectionString)
{
    m_connect = nullptr;
    m_connType = DCT_POSTGRES;
}

PostgreSQLConnection::~PostgreSQLConnection()
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
    } catch (...) {
    }
}

static string csParam(const string& name, const string& value)
{
    if (!value.empty())
        return name + "=" + value + " ";
    return "";
}

String PostgreSQLConnection::nativeConnectionString() const
{
    String port;

    if (m_connString.portNumber() != 0)
        port = int2string(m_connString.portNumber());

    string result =
        csParam("dbname", m_connString.databaseName()) +
        csParam("host", m_connString.hostName()) +
        csParam("user", m_connString.userName()) +
        csParam("password", m_connString.password()) +
        csParam("port", port);

    return result;
}

void PostgreSQLConnection::_openDatabase(const String& newConnectionString)
{
    if (!active()) {
        m_inTransaction = false;

        if (!newConnectionString.empty())
            m_connString = DatabaseConnectionString(newConnectionString);

        m_connect = PQconnectdb(nativeConnectionString().c_str());

        if (PQstatus(m_connect) != CONNECTION_OK) {
            String error = PQerrorMessage(m_connect);
            PQfinish(m_connect);
            m_connect = nullptr;
            throw DatabaseException(error);
        }

        if (timestampsFormat == PG_UNKNOWN_TIMESTAMPS) {
            const char* val = PQparameterStatus(m_connect, "integer_datetimes");
            if (upperCase(val) == "ON")
                timestampsFormat = PG_INT64_TIMESTAMPS;
            else
                timestampsFormat = PG_DOUBLE_TIMESTAMPS;
        }
    }
}

void PostgreSQLConnection::closeDatabase()
{
    for (auto query: m_queryList) {
        try {
            queryFreeStmt(query);
        } catch (...) {
        }
    }

    PQfinish(m_connect);
    m_connect = nullptr;
}

void* PostgreSQLConnection::handle() const
{
    return m_connect;
}

bool PostgreSQLConnection::active() const
{
    return m_connect != nullptr;
}

void PostgreSQLConnection::driverBeginTransaction()
{
    if (m_connect == nullptr)
        open();

    if (m_inTransaction)
        throw DatabaseException("Transaction already started.");

    PGresult* res = PQexec(m_connect, "BEGIN");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        string error = "BEGIN command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(res);
        throw DatabaseException(error);
    }

    PQclear(res);

    m_inTransaction = true;
}

void PostgreSQLConnection::driverEndTransaction(bool commit)
{
    if (!m_inTransaction)
        throw DatabaseException("Transaction isn't started.");

    string action;

    if (commit)
        action = "COMMIT";
    else
        action = "ROLLBACK";

    PGresult* res = PQexec(m_connect, action.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        string error = action + " command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(res);
        throw DatabaseException(error);
    }

    PQclear(res);

    m_inTransaction = false;
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
    querySetStmt(query, new PostgreSQLStatement(timestampsFormat == PG_INT64_TIMESTAMPS, query->autoPrepare()));
}

void PostgreSQLConnection::queryFreeStmt(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto statement = (PostgreSQLStatement*) query->statement();

    if (statement != nullptr) {
        if (statement->stmt() != nullptr) {
            if (!statement->name().empty()) {
                string deallocateCommand = "DEALLOCATE \"" + statement->name() + "\"";
                PGresult* res = PQexec(m_connect, deallocateCommand.c_str());
                ExecStatusType rc = PQresultStatus(res);
                if (rc >= PGRES_BAD_RESPONSE) {
                    string error = "DEALLOCATE command failed: ";
                    error += PQerrorMessage(m_connect);
                    PQclear(res);
                    THROW_QUERY_ERROR(query, error);
                }
                PQclear(res);
            }
        }

        delete statement;
    }

    querySetStmt(query, nullptr);

    querySetPrepared(query, false);
}

void PostgreSQLConnection::queryCloseStmt(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto statement = (PostgreSQLStatement*) query->statement();
    statement->clearRows();
}

void PostgreSQLConnection::queryPrepare(Query* query)
{
    queryFreeStmt(query);

    lock_guard<mutex> lock(m_mutex);

    querySetStmt(query, new PostgreSQLStatement(timestampsFormat == PG_INT64_TIMESTAMPS, query->autoPrepare()));

    auto statement = (PostgreSQLStatement*) query->statement();

    PostgreSQLParamValues& params = statement->m_paramValues;
    params.setParameters(query->params());

    const Oid* paramTypes = params.types();
    unsigned paramCount = params.size();

    PGresult* stmt = PQprepare(m_connect, statement->name().c_str(), query->sql().c_str(), (int) paramCount,
                               paramTypes);

    if (PQresultStatus(stmt) != PGRES_COMMAND_OK) {
        string error = "PREPARE command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(stmt);
        THROW_QUERY_ERROR(query, error);
    }

    PGresult* stmt2 = PQdescribePrepared(m_connect, statement->name().c_str());
    auto fieldCount = (unsigned) PQnfields(stmt2);

    if (fieldCount != 0 && PQftype(stmt2, 0) == VOIDOID)
        fieldCount = 0;   // VOID result considered as no result

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
    auto statement = (PostgreSQLStatement*) query->statement();

    return (int) statement->colCount();
}

void PostgreSQLConnection::queryBindParameters(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto statement = (PostgreSQLStatement*) query->statement();
    PostgreSQLParamValues& paramValues = statement->m_paramValues;
    const CParamVector& params = paramValues.params();
    uint32_t paramNumber = 0;

    for (auto ptor = params.begin(); ptor != params.end(); ++ptor, paramNumber++) {
        QueryParameter* param = *ptor;
        paramValues.setParameterValue(paramNumber, param);
    }

    int resultFormat = 1;   // Results are presented in binary format

    if (statement->colCount() == 0)
        resultFormat = 0;   // VOID result or NO results, using text format

    PGresult* stmt = PQexecPrepared(m_connect, statement->name().c_str(), (int) paramValues.size(),
                                    paramValues.values(),
                                    paramValues.lengths(), paramValues.formats(), resultFormat);

    ExecStatusType rc = PQresultStatus(stmt);

    string error;
    switch (rc) {
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

    if (!error.empty()) {
        PQclear(stmt);
        statement->clear();
        THROW_QUERY_ERROR(query, error);
    }
}

void PostgreSQLConnection::queryExecDirect(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto statement = (PostgreSQLStatement*) query->statement();
    PostgreSQLParamValues& paramValues = statement->m_paramValues;
    const CParamVector& params = paramValues.params();
    uint32_t paramNumber = 0;

    for (auto ptor = params.begin(); ptor != params.end(); ++ptor, paramNumber++) {
        QueryParameter* param = *ptor;
        paramValues.setParameterValue(paramNumber, param);
    }

    int resultFormat = 1;   // Results are presented in binary format
    PGresult* stmt = PQexecParams(m_connect, query->sql().c_str(), (int) paramValues.size(), paramValues.types(),
                                  paramValues.values(),
                                  paramValues.lengths(), paramValues.formats(), resultFormat);

    ExecStatusType rc = PQresultStatus(stmt);

    string error;
    switch (rc) {
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

    if (!error.empty()) {
        PQclear(stmt);
        statement->clear();
        THROW_QUERY_ERROR(query, error);
    }
}

void PostgreSQLConnection::PostgreTypeToCType(int postgreType, VariantType& dataType)
{
    switch (postgreType) {
        case PG_BOOL:
            dataType = VAR_BOOL;
            return;

        case PG_OID:
        case PG_INT2:
        case PG_INT4:
            dataType = VAR_INT;
            return;

        case PG_INT8:
            dataType = VAR_INT64;
            return;

        case PG_NUMERIC:
        case PG_FLOAT4:
        case PG_FLOAT8:
            dataType = VAR_FLOAT;
            return;

        case PG_BYTEA:
            dataType = VAR_BUFFER;
            return;

        case PG_DATE:
            dataType = VAR_DATE;
            return;

        case PG_TIME:
        case PG_TIMESTAMP:
            dataType = VAR_DATE_TIME;
            return;

        default:
            dataType = VAR_STRING;
            return;
    }
}

void PostgreSQLConnection::CTypeToPostgreType(VariantType dataType, Oid& postgreType, const String& paramName)
{
    switch (dataType) {
        case VAR_INT:
            postgreType = PG_INT4;
            return;        ///< Integer 4 bytes

        case VAR_MONEY:
            postgreType = PG_FLOAT8;
            return;        ///< Floating-point (double)

        case VAR_FLOAT:
            postgreType = PG_FLOAT8;
            return;        ///< Floating-point (double)

        case VAR_STRING:
        case VAR_TEXT:
            postgreType = PG_VARCHAR;
            return;        ///< Varchar

        case VAR_BUFFER:
            postgreType = PG_BYTEA;
            return;        ///< Bytea

        case VAR_DATE:
        case VAR_DATE_TIME:
            postgreType = PG_TIMESTAMP;
            return;        ///< Timestamp

        case VAR_INT64:
            postgreType = PG_INT8;
            return;        ///< Integer 8 bytes

        case VAR_BOOL:
            postgreType = PG_BOOL;
            return;           ///< Boolean

        default:
            throw DatabaseException("Unsupported parameter type(" + to_string(dataType) + ") for parameter '" + paramName + "'");
    }
}

void PostgreSQLConnection::queryOpen(Query* query)
{
    if (!active())
        open();

    if (query->active())
        return;

    if (query->statement() == nullptr)
        queryAllocStmt(query);

    if (query->autoPrepare()) {
        if (!query->prepared())
            queryPrepare(query);
        queryBindParameters(query);
    } else
        queryExecDirect(query);

    auto statement = (PostgreSQLStatement*) query->statement();

    auto count = (short) queryColCount(query);
    if (count < 1) {
        //queryCloseStmt(query);
        return;
    }

    querySetActive(query, true);

    if (query->fieldCount() == 0) {
        lock_guard<mutex> lock(m_mutex);
        // Reading the column attributes
        const PGresult* stmt = statement->stmt();

        stringstream columnName;
        columnName.fill('0');
        for (short column = 0; column < count; column++) {
            columnName.str(PQfname(stmt, column));

            if (columnName.str().empty())
                columnName << "column" << setw(2) << (column + 1);

            Oid dataType = PQftype(stmt, column);
            VariantType fieldType;
            PostgreTypeToCType((int) dataType, fieldType);
            int fieldLength = PQfsize(stmt, column);
            //int mod = PQfmod(stmt, column);
            DatabaseField* field = new DatabaseField(columnName.str(), column, (int) dataType, fieldType, fieldLength);
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
    return (int16_t) ntohs(*(uint16_t*) data);
}

static inline int32_t readInt4(const char* data)
{
    return (int32_t) ntohl(*(uint32_t*) data);
}

static inline int64_t readInt8(const char* data)
{
    return (int64_t) ntohq(*(uint64_t*) data);
}

static inline float readFloat4(const char* data)
{
    auto v = (int32_t) ntohl(*(uint32_t*) data);
    return *(float*) (void*) &v;
}

static inline double readFloat8(const char* data)
{
    auto v = (int64_t) ntohq(*(uint64_t*) data);
    return *(double*) (void*) &v;
}

static inline DateTime readDate(const char* data)
{
    auto dt = (int32_t) ntohl(*(uint32_t*) data);
    return epochDate + chrono::hours(dt * 24);
}

static inline DateTime readTimestamp(const char* data, bool integerTimestamps)
{
    auto v = (int64_t) ntohq(*(uint64_t*) data);

    if (integerTimestamps) {
        // time is in usecs
        return epochDate + chrono::microseconds(v);
    }

    double seconds = *(double*) (void*) &v;
    DateTime ts = epochDate + chrono::microseconds((int64_t) seconds * 1000000);
    return ts;
}

// Converts internal NUMERIC Postgresql binary to long double
/*
static inline long double readNumericToFloat(char* v)
{
    int16_t ndigits = ntohs(*(int16_t*) v);
    int16_t weight = ntohs(*(int16_t*) (v + 2));
    int16_t sign = ntohs(*(int16_t*) (v + 4));
    //int16_t dscale  = ntohs(*(int16_t*)(v+6));

    v += 8;
    int64_t value = 0;
    int64_t decValue = 0;
    int64_t divider = 1;

    if (weight < 0) {
        for (int i = 0; i < -(weight + 1); i++)
            divider *= 10000;
        weight = -1;
    }

    for (int i = 0; i < ndigits; i++, v += 2) {
        int16_t digit = ntohs(*(int16_t*) v);

        if (i <= weight)
            value = value * 10000 + digit;
        else {
            decValue = decValue * 10000 + digit;
            divider *= 10000;
        }
    }

    long double finalValue = value + decValue / (long double) (divider);

    if (sign)
        finalValue = -finalValue;

    return finalValue;
}
*/

// Converts internal NUMERIC Postgresql binary to long double
static inline MoneyData readNumericToScaledInteger(const char* v)
{
    auto ndigits = (int16_t) ntohs(*(uint16_t*) v);
    auto weight = (int16_t) ntohs(*(uint16_t*) (v + 2));
    auto sign = (int16_t) ntohs(*(uint16_t*) (v + 4));
    uint16_t dscale = ntohs(*(uint16_t*) (v + 6));

    v += 8;
    int64_t value = 0;

    int scale = 0;
    if (weight < 0) {
        for (int i = 0; i < -(weight + 1); i++)
            scale += 4;
    }

    int16_t digitWeight = weight;
    for (int i = 0; i < ndigits; i++, v += 2, digitWeight--) {
        auto digit = (int16_t) ntohs(*(uint16_t*) v);

        value = value * 10000 + digit;
        if (digitWeight < 0)
            scale += 4;
    }

    while (scale < dscale - 4) {
        value *= 10000;
        scale += 4;
    }

    switch (scale - dscale) {
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
        value = -value;

    MoneyData moneyData = {value, uint8_t(dscale)};

    return moneyData;
}


static void decodeArray(const char* data, DatabaseField* field)
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

    auto arrayHeader = (PGArrayHeader*) data;
    arrayHeader->dimensionNumber = ntohl(arrayHeader->dimensionNumber);
    arrayHeader->hasNull = ntohl(arrayHeader->hasNull);
    arrayHeader->elementType = ntohl(arrayHeader->elementType);
    data += sizeof(PGArrayHeader);

    auto dimensions = (PGArrayDimension*) data;
    data += arrayHeader->dimensionNumber * sizeof(PGArrayDimension);

    stringstream output;
    for (size_t dim = 0; dim < arrayHeader->dimensionNumber; dim++) {
        PGArrayDimension* dimension = dimensions + dim;
        dimension->elementCount = htonl(dimension->elementCount);
        dimension->lowerBound = htonl(dimension->lowerBound);
        output << "{";
        for (size_t element = 0; element < dimension->elementCount; element++) {
            if (element != 0)
                output << ",";

            uint32_t dataSize = ntohl(*(uint32_t*) data);
            data += sizeof(uint32_t);

            switch (arrayHeader->elementType) {
                case PG_INT2:
                    output << readInt2(data);
                    break;

                case PG_INT4:
                    output << readInt4(data);
                    break;

                case PG_INT8:
                    output << readInt8(data);
                    break;

                case PG_FLOAT4:
                    output << readFloat4(data);
                    break;

                case PG_FLOAT8:
                    output << readFloat8(data);
                    break;

                case PG_TEXT:
                case PG_CHAR:
                case PG_VARCHAR:
                    output << string(data, dataSize);
                    break;

                case PG_DATE: {
                    DateTime dt = readDate(data);
                    output << dt.dateString();
                    break;
                }

                case PG_TIMESTAMPTZ:
                case PG_TIMESTAMP: {
                    DateTime dt = readTimestamp(data, timestampsFormat == PG_INT64_TIMESTAMPS);
                    output << dt.dateString();
                    break;
                }

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
        THROW_QUERY_ERROR(query, "Dataset isn't open");

    lock_guard<mutex> lock(m_mutex);

    auto statement = (PostgreSQLStatement*) query->statement();

    statement->fetch();

    if (statement->eof()) {
        querySetEof(query, true);
        return;
    }

    auto fieldCount = (int) query->fieldCount();
    int dataLength = 0;

    if (fieldCount == 0)
        return;

    DatabaseField* field = nullptr;
    const PGresult* stmt = statement->stmt();
    int currentRow = statement->currentRow();

    for (int column = 0; column < fieldCount; column++) {
        try {
            field = (DatabaseField*) &(*query)[column];
            auto fieldType = (short) field->fieldType();

            dataLength = PQgetlength(stmt, currentRow, column);

            if (dataLength == 0) {
                bool isNull = true;
                if ((fieldType & (VAR_STRING | VAR_TEXT | VAR_BUFFER)) != 0)
                    isNull = PQgetisnull(stmt, currentRow, column) == 1;

                if (isNull)
                    field->setNull(VAR_NONE);
                else
                    field->setExternalString("", 0);
            } else {
                char* data = PQgetvalue(stmt, currentRow, column);

                switch (fieldType) {

                    case PG_BOOL:
                        field->setBool(readBool(data));
                        break;

                    case PG_INT2:
                        field->setInteger(readInt2(data));
                        break;

                    case PG_OID:
                    case PG_INT4:
                        field->setInteger(readInt4(data));
                        break;

                    case PG_INT8:
                        field->setInt64(readInt8(data));
                        break;

                    case PG_FLOAT4:
                        field->setFloat(readFloat4(data));
                        break;

                    case PG_FLOAT8:
                        field->setFloat(readFloat8(data));
                        break;

                    case PG_NUMERIC:
                        field->setMoney(readNumericToScaledInteger(data));
                        break;

                    default:
                        field->setExternalString(data, dataLength);
                        break;

                    case PG_BYTEA:
                        field->setExternalBuffer(data, (size_t) dataLength);
                        break;

                    case PG_DATE:
                        field->setDateTime(readDate(data));
                        break;

                    case PG_TIMESTAMPTZ:
                    case PG_TIMESTAMP:
                        field->setDateTime(readTimestamp(data, timestampsFormat == PG_INT64_TIMESTAMPS));
                        break;

                    case PG_CHAR_ARRAY:
                    case PG_INT2_VECTOR:
                    case PG_INT2_ARRAY:
                    case PG_INT4_ARRAY:
                    case PG_TEXT_ARRAY:
                    case PG_VARCHAR_ARRAY:
                    case PG_INT8_ARRAY:
                    case PG_FLOAT4_ARRAY:
                    case PG_FLOAT8_ARRAY:
                    case PG_TIMESTAMP_ARRAY:
                    case PG_TIMESTAMPTZ_ARRAY:
                        decodeArray(data, field);
                        break;

                }
            }

        } catch (exception& e) {
            THROW_QUERY_ERROR(query, "Can't read field " << field->fieldName() << ": " << e.what());
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

    switch (objectType) {
        case DOT_FUNCTIONS:
            objectsSQL =
                "SELECT DISTINCT routine_schema || '.' || routine_name "
                    "FROM information_schema.routines "
                    "WHERE routine_schema NOT IN ('information_schema','pg_catalog') "
                    "AND routine_type = 'FUNCTION'";
            break;

        case DOT_PROCEDURES:
            objectsSQL =
                "SELECT DISTINCT routine_schema || '.' || routine_name "
                    "FROM information_schema.routines "
                    "WHERE routine_schema NOT IN ('information_schema','pg_catalog') "
                    "AND routine_type = 'PROCEDURE'";
            break;

        case DOT_TABLES:
            objectsSQL = tablesSQL + "AND table_type = 'BASE TABLE'";
            break;

        case DOT_VIEWS:
            objectsSQL = tablesSQL + "AND table_type = 'VIEW'";
            break;

        case DOT_DATABASES:
            objectsSQL =
                "SELECT datname FROM pg_database WHERE datname NOT IN ('postgres','template0','template1')";
            break;
    }

    Query query(this, objectsSQL);
    query.open();

    while (!query.eof()) {
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
    char mark[16];
    snprintf(mark, sizeof(mark), "$%i", paramIndex + 1);
    return string(mark);
}

void PostgreSQLConnection::_bulkInsert(const String& tableName, const Strings& columnNames, const Strings& data,
                                       const String& format)
{
    stringstream sql;
    sql << "COPY " << tableName << "(" << columnNames.asString(",") << ") FROM STDIN " << format;

    PGresult* res = PQexec(m_connect, sql.str().c_str());

    ExecStatusType rc = PQresultStatus(res);
    if (rc >= PGRES_BAD_RESPONSE) {
        string error = "COPY command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(res);
        throw DatabaseException(error);
    }
    PQclear(res);

    Buffer buffer;
    for (auto& row: data) {
        buffer.append(row);
        buffer.append(char('\n'));
    }

    if (PQputCopyData(m_connect, buffer.c_str(), (int) buffer.bytes()) != 1) {
        string error = "COPY command send data failed: ";
        error += PQerrorMessage(m_connect);
        throw DatabaseException(error);
    }

    if (PQputCopyEnd(m_connect, nullptr) != 1) {
        string error = "COPY command end copy failed: ";
        error += PQerrorMessage(m_connect);
        throw DatabaseException(error);
    }
}

void PostgreSQLConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    RegularExpression matchFunction("^(CREATE|REPLACE) .*FUNCTION", "i");
    RegularExpression matchFunctionBodyStart("AS\\s+(\\S+)\\s*$", "i");
    RegularExpression matchStatementEnd(";(\\s*|\\s*--.*)$");
    RegularExpression matchCommentRow("^\\s*--");

    Strings statements, matches;
    String delimiter;
    stringstream statement;

    bool functionHeader = false;
    bool functionBody = false;
    for (auto row : sqlBatch) {
        if (!functionHeader && !functionBody) {
            row = row.trim();
            if (row.empty() || matchCommentRow.matches(row))
                continue;
        }
        if (!functionHeader) {
            if (matchFunction.m(row, matches)) {
                functionHeader = true;
                statement << row << "\n";
                continue;
            }
        }

        if (functionHeader && !functionBody && matchFunctionBodyStart.m(row, matches)) {
            functionBody = true;
            functionHeader = false;
            delimiter = matches[0];
            statement << row << "\n";
            continue;
        }

        if (functionBody && row.find(delimiter) != string::npos) {
            delimiter = "";
            functionBody = false;
        }

        if (!functionBody) {
            if (matchStatementEnd.m(row, matches)) {
                statement << row;
                statements.push_back(statement.str());
                statement.str("");
                continue;
            }
        }

        statement << row << "\n";
    }

    if (!trim(statement.str()).empty())
        statements.push_back(statement.str());

    for (auto& stmt : statements) {
        try {
            Query query(this, stmt);
            query.exec();
        }
        catch (const exception& e) {
            if (errors != nullptr)
                errors->push_back(e.what());
            else
                throw;
        }
    }
}

void* postgresql_create_connection(const char* connectionString)
{
    PostgreSQLConnection* connection = new PostgreSQLConnection(connectionString);
    return connection;
}

void postgresql_destroy_connection(void* connection)
{
    delete (PostgreSQLConnection*) connection;
}
