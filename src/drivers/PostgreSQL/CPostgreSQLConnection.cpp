/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CPostgreSQLConnection.cpp  -  description
                             -------------------
    begin                : Mon Sep 17 2007
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/db/CPostgreSQLConnection.h>
#include <sptk5/db/CDatabaseField.h>
#include <sptk5/db/CQuery.h>

#include "CPostgreSQLParamValues.h"
#include "htonq.h"
#ifndef WIN32
#include <arpa/inet.h>
#endif

#include <string>
#include <stdio.h>
#include <sstream>
#include <stdint.h>

using namespace std;
using namespace sptk;

namespace sptk
{

const CDateTime epochDate(2000, 1, 1);

class CPostgreSQLStatement
{
    PGresult* m_stmt;
    char m_stmtName[20];
    static unsigned index;
    int m_rows;
    int m_cols;
    int m_currentRow;
public:
    CPostgreSQLParamValues m_paramValues;
public:

    CPostgreSQLStatement(bool int64timestamps)
    : m_paramValues(int64timestamps) {
        m_stmt = NULL;
        sprintf(m_stmtName, "S%04i", ++index);
    }

    ~CPostgreSQLStatement() {
        if (m_stmt)
            PQclear(m_stmt);
    }

    void clear() {
        clearRows();
        m_cols = 0;
    }

    void clearRows() {
        if (m_stmt) {
            PQclear(m_stmt);
            m_stmt = 0;
        }

        m_rows = 0;
        m_currentRow = -1;
    }

    void stmt(PGresult* st, unsigned rows, unsigned cols = 99999) {
        if (m_stmt)
            PQclear(m_stmt);

        m_stmt = st;
        m_rows = (int) rows;

        if (cols != 99999)
            m_cols = (int) cols;

        m_currentRow = -1;
    }

    const PGresult* stmt() const {
        return m_stmt;
    }

    string name() const {
        return m_stmtName;
    }

    void fetch() {
        m_currentRow++;
    }

    bool eof() {
        return m_currentRow >= m_rows;
    }

    unsigned currentRow() const {
        return (unsigned) m_currentRow;
    }

    unsigned colCount() const {
        return (unsigned) m_cols;
    }

    unsigned rowCount() const {
        return (unsigned) m_rows;
    }

    const CParamVector& params() const {
        return m_paramValues.m_params;
    }
};

unsigned CPostgreSQLStatement::index;
}

enum CPostgreSQLTimestampFormat { PG_UNKNOWN_TIMESTAMPS, PG_DOUBLE_TIMESTAMPS, PG_INT64_TIMESTAMPS };
static CPostgreSQLTimestampFormat timestampsFormat;

CPostgreSQLConnection::CPostgreSQLConnection(string connectionString) :
    CDatabaseConnection(connectionString)
{
    m_connect = 0;
    m_connType = DCT_POSTGRES;
}

CPostgreSQLConnection::~CPostgreSQLConnection()
{
    try {
        if (m_inTransaction && active())
            rollbackTransaction();

        close();

        while (m_queryList.size()) {
            try {
                CQuery* query = (CQuery*) m_queryList[0];
                query->disconnect();
            } catch (...) {
            }
        }

        m_queryList.clear();
    } catch (...) {
    }
}

static string csParam(string name, string value)
{
    if (!value.empty())
        return name + "=" + value + " ";

    return "";
}

string CPostgreSQLConnection::nativeConnectionString() const
{
    string port;

    if (m_connString.portNumber())
        port = int2string(m_connString.portNumber());

    string result =
        csParam("dbname", m_connString.databaseName()) +
        csParam("host", m_connString.hostName()) +
        csParam("user", m_connString.userName()) +
        csParam("password", m_connString.password()) +
        csParam("port", port);

    return result;
}

void CPostgreSQLConnection::openDatabase(string newConnectionString) THROWS_EXCEPTIONS
{
    if (!active()) {
        m_inTransaction = false;

        if (newConnectionString.length())
            m_connString = newConnectionString;

        m_connect = PQconnectdb(nativeConnectionString().c_str());

        if (PQstatus(m_connect) != CONNECTION_OK) {
            string error = PQerrorMessage(m_connect);
            PQfinish(m_connect);
            m_connect = NULL;
            throw CDatabaseException(error);
        }

        if (timestampsFormat == PG_UNKNOWN_TIMESTAMPS) {
            const char* val = PQparameterStatus(m_connect,"integer_datetimes");
            if (upperCase(val) == "ON")
                timestampsFormat = PG_INT64_TIMESTAMPS;
            else
                timestampsFormat = PG_DOUBLE_TIMESTAMPS;
        }
    }
}

void CPostgreSQLConnection::closeDatabase() THROWS_EXCEPTIONS
{
    for (unsigned i = 0; i < m_queryList.size(); i++) {
        try {
            CQuery* query = (CQuery*) m_queryList[i];
            queryFreeStmt(query);
        } catch (...) {
        }
    }

    PQfinish(m_connect);
    m_connect = NULL;
}

void* CPostgreSQLConnection::handle() const
{
    return m_connect;
}

bool CPostgreSQLConnection::active() const
{
    return m_connect != 0L;
}

void CPostgreSQLConnection::driverBeginTransaction() THROWS_EXCEPTIONS
{
    if (!m_connect)
        open();

    if (m_inTransaction)
        throw CDatabaseException("Transaction already started.");

    PGresult* res = PQexec(m_connect, "BEGIN");

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        string error = "BEGIN command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(res);
        throw CDatabaseException(error);
    }

    PQclear(res);

    m_inTransaction = true;
}

void CPostgreSQLConnection::driverEndTransaction(bool commit) THROWS_EXCEPTIONS
{
    if (!m_inTransaction)
        throw CDatabaseException("Transaction isn't started.");

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
        throw CDatabaseException(error);
    }

    PQclear(res);

    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------

string CPostgreSQLConnection::queryError(const CQuery* query) const
{
    return PQerrorMessage(m_connect);
}

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void CPostgreSQLConnection::queryAllocStmt(CQuery* query)
{
    queryFreeStmt(query);
    querySetStmt(query, new CPostgreSQLStatement(timestampsFormat == PG_INT64_TIMESTAMPS));
}

void CPostgreSQLConnection::queryFreeStmt(CQuery* query)
{
    SYNCHRONIZED_CODE;

    CPostgreSQLStatement* statement = (CPostgreSQLStatement*) query->statement();

    if (statement) {
        if (statement->stmt()) {
            string deallocateCommand = "DEALLOCATE \"" + statement->name() + "\"";
            PGresult* res = PQexec(m_connect, deallocateCommand.c_str());
            ExecStatusType rc = PQresultStatus(res);

            if (rc >= PGRES_BAD_RESPONSE) {
                string error = "DEALLOCATE command failed: ";
                error += PQerrorMessage(m_connect);
                PQclear(res);
                query->logAndThrow("CPostgreSQLConnection::queryFreeStmt", error);
            }

            PQclear(res);
        }

        delete statement;
    }

    querySetStmt(query, 0L);

    querySetPrepared(query, false);
}

void CPostgreSQLConnection::queryCloseStmt(CQuery* query)
{
    SYNCHRONIZED_CODE;

    CPostgreSQLStatement* statement = (CPostgreSQLStatement*) query->statement();
    statement->clearRows();
}

void CPostgreSQLConnection::queryPrepare(CQuery* query)
{
    queryFreeStmt(query);

    SYNCHRONIZED_CODE;

    querySetStmt(query, new CPostgreSQLStatement(timestampsFormat == PG_INT64_TIMESTAMPS));

    CPostgreSQLStatement* statement = (CPostgreSQLStatement*) query->statement();

    CPostgreSQLParamValues& params = statement->m_paramValues;
    params.setParameters(query->params());

    const Oid* paramTypes = params.types();
    unsigned paramCount = params.size();

    PGresult* stmt = PQprepare(m_connect, statement->name().c_str(), query->sql().c_str(), (int) paramCount, paramTypes);

    if (PQresultStatus(stmt) != PGRES_COMMAND_OK) {
        string error = "PREPARE command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(stmt);
        query->logAndThrow("CPostgreSQLConnection::queryPrepare", error);
    }

    PGresult* stmt2 = PQdescribePrepared(m_connect, statement->name().c_str());
    unsigned fieldCount = (unsigned) PQnfields(stmt2);

    if (fieldCount && PQftype(stmt2, 0) == VOIDOID)
        fieldCount = 0;   // VOID result considered as no result

    PQclear(stmt2);

    statement->stmt(stmt, 0, fieldCount);

    querySetPrepared(query, true);
}

void CPostgreSQLConnection::queryUnprepare(CQuery* query)
{
    queryFreeStmt(query);
}

int CPostgreSQLConnection::queryColCount(CQuery* query)
{

    CPostgreSQLStatement* statement = (CPostgreSQLStatement*) query->statement();

    return (int) statement->colCount();
}

void CPostgreSQLConnection::queryBindParameters(CQuery* query)
{
    SYNCHRONIZED_CODE;

    CPostgreSQLStatement* statement = (CPostgreSQLStatement*) query->statement();
    CPostgreSQLParamValues& paramValues = statement->m_paramValues;
    const CParamVector& params = paramValues.params();
    uint32_t paramNumber = 0;

    for (CParamVector::const_iterator ptor = params.begin(); ptor != params.end(); ptor++, paramNumber++) {
        CParam* param = *ptor;
        paramValues.setParameterValue(paramNumber, param);
    }

    int resultFormat = 1;   // Results are presented in binary format

    if (!statement->colCount())
        resultFormat = 0;   // VOID result or NO results, using text format

    PGresult* stmt = PQexecPrepared(m_connect, statement->name().c_str(), (int) paramValues.size(), paramValues.values(),
                                    paramValues.lengths(), paramValues.formats(), resultFormat);

    ExecStatusType rc = PQresultStatus(stmt);

    switch (rc) {
    case PGRES_COMMAND_OK:
        statement->stmt(stmt, 0, 0);
        break;

    case PGRES_TUPLES_OK:
        statement->stmt(stmt, (unsigned) PQntuples(stmt));
        break;

    default: {
        string error = "EXECUTE command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(stmt);
        statement->clear();
        query->logAndThrow("CPostgreSQLConnection::queryBindParameters", error);
    }
    }
}

void CPostgreSQLConnection::PostgreTypeToCType(int postgreType, CVariantType& dataType)
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

void CPostgreSQLConnection::CTypeToPostgreType(CVariantType dataType, Oid& postgreType)
{
    switch (dataType) {
    case VAR_INT:
        postgreType = PG_INT4;
        return;        ///< Integer 4 bytes

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
        throwException("Unsupported SPTK data type: " + int2string(dataType));
    }
}

void CPostgreSQLConnection::queryOpen(CQuery* query)
{
    if (!active())
        open();

    if (query->active())
        return;

    if (!query->statement())
        queryAllocStmt(query);

    if (!query->prepared())
        queryPrepare(query);

    // Bind parameters also executes a query
    queryBindParameters(query);

    //query->fields().clear();

    CPostgreSQLStatement* statement = (CPostgreSQLStatement*) query->statement();
    //if (statement->rowCount() == 0)
    //    return;

    short count = (short) queryColCount(query);

    if (count < 1) {
        //queryCloseStmt(query);
        return;
    } else {
        querySetActive(query, true);

        if (query->fieldCount() == 0) {
            SYNCHRONIZED_CODE;
            // Reading the column attributes
            char columnName[256];
            //long  columnType;
            //CVariantType dataType;
            const PGresult* stmt = statement->stmt();

            for (short column = 0; column < count; column++) {
                strncpy(columnName, PQfname(stmt, column), 255);
                columnName[255] = 0;

                if (columnName[0] == 0)
                    sprintf(columnName, "column%02i", column + 1);

                Oid dataType = PQftype(stmt, column);
                CVariantType fieldType;
                PostgreTypeToCType((int)dataType, fieldType);
                int fieldLength = PQfsize(stmt, column);
                CDatabaseField* field = new CDatabaseField(columnName, column, (int) dataType, fieldType, fieldLength);
                query->fields().push_back(field);
            }
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

static inline bool readBool(char* data)
{
    return (bool) *data;
}

static inline int16_t readInt2(char* data)
{
    return (int16_t) ntohs(*(uint16_t*) data);
}

static inline int32_t readInt4(char* data)
{
    return (int32_t) ntohl(*(uint32_t*) data);
}

static inline int64_t readInt8(char* data)
{
    return (int64_t) ntohq(*(uint64_t*) data);
}

static inline float readFloat4(char* data)
{
    int32_t v = (int32_t) ntohl(*(uint32_t*) data);
    return *(float*) (void*) &v;
}

static inline double readFloat8(char* data)
{
    int64_t v = (int64_t) ntohq(*(uint64_t*) data);
    return *(double*) (void*) &v;
}

static inline double readDate(char* data)
{
    int32_t dt = (int32_t) ntohl(*(uint32_t*) data);
    return dt + (int32_t) epochDate;
}

static inline double readTimestamp(char* data, bool integerTimestamps)
{
    int64_t v = (int64_t) ntohq(*(uint64_t*) data);
    double dt = (double) epochDate;
    if (integerTimestamps) {
        // time is in usecs
        dt += v / 1000000.0 / 3600.0 / 24.0;
    } else {
        // time is in secs
        dt += *(double*) (void*) &v / 3600.0 / 24.0;
    }
    return dt;
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
static inline CMoneyData readNumericToScaledInteger(char* v)
{
    int16_t ndigits = (int16_t) ntohs(*(uint16_t*) v);
    int16_t weight = (int16_t) ntohs(*(uint16_t*) (v + 2));
    int16_t sign = (int16_t) ntohs(*(uint16_t*) (v + 4));
    uint16_t dscale = ntohs(*(uint16_t*)(v+6));

    v += 8;
    int64_t value = 0;
    int64_t divider = 1;

    int scale = 0;
    if (weight < 0) {
        for (int i = 0; i < -(weight + 1); i++) {
            divider *= 10000;
            scale += 4;
        }
    }

    int16_t digitWeight = weight;
    for (int i = 0; i < ndigits; i++, v += 2, digitWeight--) {
        int16_t digit = (int16_t) ntohs(*(int16_t*) v);

        value = value * 10000 + digit;
        if (digitWeight < 0)
            scale += 4;
    }

    while (scale < dscale - 4) {
        value *= 10000;
        scale += 4;
    }

    switch (scale - dscale) {
        case -3: value *= 1000; break;
        case -2: value *= 100; break;
        case -1: value *= 10; break;
        case 1: value /= 10; break;
        case 2: value /= 100; break;
        case 3: value /= 1000; break;
    }

    scale = dscale;

    if (sign)
        value = -value;

    CMoneyData moneyData = {value, uint8_t(dscale) };

    return moneyData;
}


static void decodeArray(char* data, CDatabaseField* field)
{
    struct PGArrayHeader {
        uint32_t dimensionNumber;
        uint32_t hasNull;
        uint32_t elementType;
    };

    struct PGArrayDimension {
        uint32_t elementCount;
        uint32_t lowerBound;
    };

    PGArrayHeader* arrayHeader = (PGArrayHeader*) data;
    arrayHeader->dimensionNumber = ntohl(arrayHeader->dimensionNumber);
    arrayHeader->hasNull = ntohl(arrayHeader->hasNull);
    arrayHeader->elementType = ntohl(arrayHeader->elementType);
    data += sizeof(PGArrayHeader);

    PGArrayDimension* dimensions = (PGArrayDimension*) data;
    data += arrayHeader->dimensionNumber * sizeof(PGArrayDimension);

    stringstream output;
    for (size_t dim = 0; dim < arrayHeader->dimensionNumber; dim++) {
        PGArrayDimension* dimension = dimensions + dim;
        dimension->elementCount = htonl(dimension->elementCount);
        dimension->lowerBound = htonl(dimension->lowerBound);
        output << "{";
        for (size_t element = 0; element < dimension->elementCount; element++) {
            if (element)
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
                    CDateTime dt = readDate(data);
                    output << dt.dateString();
                    break;
                }

                case PG_TIMESTAMPTZ:
                case PG_TIMESTAMP: {
                    CDateTime dt = readTimestamp(data, timestampsFormat == PG_INT64_TIMESTAMPS);
                    output << dt.dateString();
                    break;
                }

                default:
                    throw CDatabaseException("Unsupported array element type");
            }
            data += dataSize;
        }
        output << "}";
    }
    field->setString(output.str());
}

void CPostgreSQLConnection::queryFetch(CQuery* query)
{
    if (!query->active())
        query->logAndThrow("CPostgreSQLConnection::queryFetch", "Dataset isn't open");

    SYNCHRONIZED_CODE;

    CPostgreSQLStatement* statement = (CPostgreSQLStatement*) query->statement();

    statement->fetch();

    if (statement->eof()) {
        querySetEof(query, true);
        return;
    }

    int fieldCount = (int) query->fieldCount();
    int dataLength = 0;

    if (!fieldCount)
        return;

    CDatabaseField* field = 0;
    const PGresult* stmt = statement->stmt();
    int currentRow = statement->currentRow();

    for (int column = 0; column < fieldCount; column++) {
        try {
            field = (CDatabaseField*) & (*query)[(int) column];
            short fieldType = (short) field->fieldType();

            bool isNull = false;
            dataLength = PQgetlength(stmt, currentRow, column);

            if (!dataLength) {
                if (fieldType & (VAR_STRING | VAR_TEXT | VAR_BUFFER))
                    isNull = PQgetisnull(stmt, currentRow, column);
                else
                    isNull = true;

                if (isNull)
                    field->setNull();
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
                    field->setExternalString(data, size_t(dataLength));
                    break;

                case PG_BYTEA:
                    field->setExternalBuffer(data, size_t(dataLength));
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
            query->logAndThrow("CPostgreSQLConnection::queryFetch",
                               "Can't read field " + field->fieldName() + ": " + string(e.what()));
        }
    }
}

void CPostgreSQLConnection::objectList(CDbObjectType objectType, CStrings& objects) THROWS_EXCEPTIONS
{
    string tablesSQL("SELECT table_schema || '.' || table_name "
                     "FROM information_schema.tables "
                     "WHERE table_schema NOT IN ('information_schema','pg_catalog') ");
    string objectsSQL;
    objects.clear();

    switch (objectType) {
    case DOT_PROCEDURES:
        objectsSQL = "SELECT DISTINCT routine_schema || '.' || routine_name "
                     "FROM information_schema.routines "
                     "WHERE routine_schema NOT IN ('information_schema','pg_catalog')";
        break;

    case DOT_TABLES:
        objectsSQL = tablesSQL + "AND table_type = 'BASE TABLE'";
        break;

    case DOT_VIEWS:
        objectsSQL = tablesSQL + "AND table_type = 'VIEW'";
        break;
    }

    CQuery query(this, objectsSQL);
    query.open();

    while (!query.eof()) {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }

    query.close();
}

std::string CPostgreSQLConnection::driverDescription() const
{
    return "PostgreSQL";
}

std::string CPostgreSQLConnection::paramMark(unsigned paramIndex)
{
    char mark[16];
    sprintf(mark, "$%i", paramIndex + 1);
    return mark;
}

void CPostgreSQLConnection::bulkInsert(std::string tableName, const CStrings& columnNames, const CStrings& data, std::string format) THROWS_EXCEPTIONS
{
    string      sql = "COPY " + tableName + "(" + columnNames.asString(",") + ") FROM STDIN " + format;
    PGresult*   res = PQexec(m_connect, sql.c_str());

    ExecStatusType rc = PQresultStatus(res);
    if (rc < 0) {
        string error = "COPY command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(res);
        throw CDatabaseException(error);
    }
    PQclear(res);

    CBuffer buffer;
    for (CStrings::const_iterator itor = data.begin(); itor != data.end(); itor++) {
        buffer.append(*itor);
        buffer.append('\n');
    }

    if (PQputCopyData(m_connect, buffer.c_str(), (int) buffer.bytes()) != 1) {
        string error = "COPY command send data failed: ";
        error += PQerrorMessage(m_connect);
        throw CDatabaseException(error);
    }

    if (PQputCopyEnd(m_connect, NULL) != 1) {
        string error = "COPY command end copy failed: ";
        error += PQerrorMessage(m_connect);
        throw CDatabaseException(error);
    }
}

void* postgresql_create_connection(const char* connectionString)
{
    CPostgreSQLConnection* connection = new CPostgreSQLConnection(connectionString);
    return connection;
}

void  postgresql_destroy_connection(void* connection)
{
    delete (CPostgreSQLConnection*) connection;
}
