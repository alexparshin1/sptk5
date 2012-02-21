/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CPostgreSQLDatabase.cpp  -  description
                             -------------------
    begin                : Mon Sep 17 2007
    copyright            : (C) 2008-2012 by Alexey Parshin. All rights reserved.
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

#include <sptk5/db/CPostgreSQLDatabase.h>
#include <sptk5/db/CDatabaseField.h>
#include <sptk5/db/CQuery.h>
#include <sptk5/db/CParams.h>

#include <string>
#include <stdio.h>
#include <arpa/inet.h>

#include <iostream>

#if HAVE_POSTGRESQL == 1

#define PG_FETCH_BINARY_RESULTS

// This is copied from the server headers. We assume that it wouldn't change in the future..
#define VOIDOID			2278

using namespace std;
using namespace sptk;

namespace sptk {
    
    /// @brief PostgreSQL-specific database field
    class CPostgreSQLField : public CDatabaseField {
        friend class CPostgreSQLDatabase;
    public:
        /// @brief Constructor
        /// @param std::string fieldName, field name
        /// @param fieldColumn int, field column number
        /// @param fieldType int, native field type
        /// @param dataType CVariantType, SPTK field data type
        /// @param fieldSize int, maximum field size in bytes
        CPostgreSQLField(std::string fieldName, int fieldColumn, int fieldType, CVariantType dataType, int fieldSize)
        : CDatabaseField(fieldName, fieldColumn, fieldType, dataType, fieldSize, 4) {}
    };
    
    class CParamValues {
        friend class CPostgreSQLStatement;
    protected:
        unsigned       m_size;
        unsigned       m_count;
        const char**   m_values;
        int*           m_lengths;
        int*           m_formats;
        Oid*           m_types;
        CParamVector   m_params;
    public:
        CParamValues() {
            m_count = 0;
            m_size = 0;
            m_values  = NULL;
            m_lengths = NULL;
            m_formats = NULL;
            m_types   = NULL;
            resize(16);
        }
        
        ~CParamValues() {
            if (m_size) {
                free(m_values);
                free(m_lengths);
                free(m_formats);
                free(m_types);
            }
        }
        
        void reset() {
            m_count = 0;
        }
        
        void resize(unsigned sz) {
            if (sz > m_size) {
                m_size = sz;
                m_values  = (const char**) realloc(m_values,  m_size * sizeof(const char*));
                m_lengths = (int*)         realloc(m_lengths, m_size * sizeof(int));
                m_formats = (int*)         realloc(m_formats, m_size * sizeof(int));
                m_types   = (Oid*)         realloc(m_types,   m_size * sizeof(Oid));
            }
        }
        
        void print() {
            cout << m_count << " parameters:" << endl;
            for (unsigned i = 0; i < m_count; i++) {
                cout << hex << (void*)m_values[i] << ", len=" << m_lengths[i] << ", format=" << m_formats[i] << ", type=" << m_types[i] 
                        << " (" << m_params[i]->name() << ")" << endl;
            }
        }
        
        void setParameters(CParamList& params);
                
        void setParameterValue(unsigned number, const void* value, unsigned sz) {
            m_values[number] = (const char*) value;
            m_lengths[number] = sz;
        }
        
        void setParameterSize(unsigned number, unsigned sz) {
            m_lengths[number] = sz;
        }
        
        unsigned size() const               { return m_count;   }
        const char* const* values() const   { return m_values;  }
        const int* lengths() const          { return m_lengths; }
        const int* formats() const          { return m_formats; }
        const Oid* types() const            { return m_types;   }
        const CParamVector& params() const  { return m_params;  }
    };

    class CPostgreSQLStatement {
        PGresult*         m_stmt;
        char              m_stmtName[20];
        static unsigned   index;
        int               m_rows;
	int               m_cols;
        int               m_currentRow;
    public:
        CParamValues      m_paramValues;
    public:
        CPostgreSQLStatement() {
            m_stmt = NULL;
            sprintf(m_stmtName, "S%04i", ++index);
        }
        
        ~CPostgreSQLStatement() {
            if (m_stmt)       PQclear(m_stmt);
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
        
        void stmt(PGresult* st, unsigned rows, unsigned cols=99999) {
            if (m_stmt)
                PQclear(m_stmt);
            m_stmt = st;
            m_rows = rows;
	    if (cols != 99999)
	        m_cols = cols;
            m_currentRow = -1;
        }
        
        const PGresult* stmt() const        { return m_stmt; }
        string   name() const               { return m_stmtName; }
        void     fetch()                    { m_currentRow++; }
        bool     eof()                      { return m_currentRow >= m_rows; }
        unsigned currentRow() const         { return m_currentRow; }
        unsigned colCount() const           { return m_cols; }
        unsigned rowCount() const           { return m_rows; }
        
        const CParamVector& params() const  { return m_paramValues.m_params;  }
    };
    
    unsigned CPostgreSQLStatement::index;
    const CDateTime epochDate(2000,1,1);
}


static uint64_t htonq(uint64_t val) {
#if WORDS_BIGENDIAN == 1
    return val;
#else
    uint64_t result;
    int32_t* src = (int32_t *)(void *)&val;
    int32_t* dst = (int32_t *)(void *)&result;
    dst[0] = htonl(src[1]);
    dst[1] = htonl(src[0]);
    return result;
#endif
}

static uint64_t ntohq(uint64_t val) {
#if WORDS_BIGENDIAN == 1
    return val;
#else
    uint64_t result;
    int32_t* src = (int32_t *)(void *)&val;
    int32_t* dst = (int32_t *)(void *)&result;
    dst[0] = htonl(src[1]);
    dst[1] = htonl(src[0]);
    return result;
#endif
}

static void inline htonq_inplace(uint64_t* in,uint64_t* out) {
#if WORDS_BIGENDIAN == 1
    return;
#else
    int32_t* src = (int32_t *)(void *)in;
    int32_t* dst = (int32_t *)(void *)out;
    dst[1] = htonl(src[0]);
    dst[0] = htonl(src[1]);
#endif
}

void CParamValues::setParameters(CParamList& params) {
    params.enumerate(m_params);
    m_count = m_params.size();
    resize(m_count);
    for (unsigned i = 0; i < m_count; i++) {
        CParam* param = m_params[i];
        CVariantType ptype = param->dataType();
        CPostgreSQLDatabase::CTypeToPostgreType(ptype, m_types[i]);
      
        if (ptype & (VAR_INT|VAR_INT64|VAR_FLOAT|VAR_BUFFER|VAR_DATE|VAR_DATE_TIME)) {
            m_formats[i] = 1; // Binary format
        } else
            m_formats[i] = 0; // Text format
        m_values[i] = param->conversionBuffer(); // This is a default. For VAR_STRING, VAR_TEXT, VAR_BUFFER and it would be replaced later
        
        switch (ptype) {
             case VAR_BOOL:
                m_lengths[i] = sizeof(bool);
                break;

             case VAR_INT:
                m_lengths[i] = sizeof(int32_t);
                break;

             case VAR_DATE:
                m_lengths[i] = sizeof(int32_t);
                break;

             case VAR_FLOAT:
             case VAR_DATE_TIME:
                m_lengths[i] = sizeof(double);
                break;

             case VAR_INT64:
                m_lengths[i] = sizeof(int64_t);
                break;
            default:
                m_lengths[i] = 0;
                break;
        }
    }
}

CPostgreSQLDatabase::CPostgreSQLDatabase(string connectionString)
: CDatabase(connectionString) {
    m_connect = 0;
}

CPostgreSQLDatabase::~CPostgreSQLDatabase() {
    try {
        if (m_inTransaction && active())
            rollbackTransaction();
        close();
        while (m_queryList.size()) {
            try {
                CQuery *query = (CQuery *)m_queryList[0];
                query->disconnect();
            } catch (...) {}}
        m_queryList.clear();
    } catch (...) {}
}

void CPostgreSQLDatabase::openDatabase(const string newConnectionString) throw(CException) {
    if (!active()) {
        m_inTransaction = false;
        if (newConnectionString.length())
            m_connString = newConnectionString;
        
        m_connect = PQconnectdb(m_connString.c_str());
        if (PQstatus(m_connect) != CONNECTION_OK) {
            string error = PQerrorMessage(m_connect);
            PQfinish(m_connect);
            m_connect = NULL;
            throw CException(error);
        }
    }
}

void CPostgreSQLDatabase::closeDatabase() throw(CException) {
    for (unsigned i = 0; i < m_queryList.size(); i++) {
        try {
            CQuery *query = (CQuery *)m_queryList[i];
            queryFreeStmt(query);
        } catch (...) {}
    }
    PQfinish(m_connect);
    m_connect = NULL;
}

void* CPostgreSQLDatabase::handle() const {
    return m_connect;
}

bool CPostgreSQLDatabase::active() const {
    return m_connect != 0L;
}

void CPostgreSQLDatabase::driverBeginTransaction() throw(CException) {
    if (!m_connect)
        open();
    
    if (m_inTransaction)
        throw CException("Transaction already started.");
    
    PGresult* res = PQexec(m_connect, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        string error = "BEGIN command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(res);
        throw CException(error);
    }
    PQclear(res);
    
    m_inTransaction = true;
}

void CPostgreSQLDatabase::driverEndTransaction(bool commit) throw(CException) {
    if (!m_inTransaction)
        throw CException("Transaction isn't started.");
    
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
        throw CException(error);
    }
    PQclear(res);
    
    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------
static inline bool successful(int ret) {
    return ret == PGRES_COMMAND_OK;
}

string CPostgreSQLDatabase::queryError(const CQuery *query) const {
    return PQerrorMessage(m_connect);
}

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void CPostgreSQLDatabase::queryAllocStmt(CQuery *query) {
    queryFreeStmt(query);
    querySetStmt(query, new CPostgreSQLStatement);
}

void CPostgreSQLDatabase::queryFreeStmt(CQuery *query) {
    CPostgreSQLLock lock(this);
    
    CPostgreSQLStatement* statement = (CPostgreSQLStatement*)query->statement();
    if (statement) {
        if (statement->stmt()) {
            string deallocateCommand = "DEALLOCATE \"" + statement->name() + "\"";
            PGresult* res = PQexec(m_connect, deallocateCommand.c_str());
            ExecStatusType rc = PQresultStatus(res);
            if (rc >= PGRES_BAD_RESPONSE) {
                string error = "DEALLOCATE command failed: ";
                error += PQerrorMessage(m_connect);
                PQclear(res);
                query->logAndThrow("CPostgreSQLDatabase::queryFreeStmt", error);
            }
            PQclear(res);
        }
        delete statement;
    }
    querySetStmt(query, 0L);
    
    querySetPrepared(query, false);
}

void CPostgreSQLDatabase::queryCloseStmt(CQuery *query) {
    CPostgreSQLLock lock(this);
    
    CPostgreSQLStatement* statement = (CPostgreSQLStatement*)query->statement();
    statement->clearRows();
}

void CPostgreSQLDatabase::queryPrepare(CQuery *query) {
    queryFreeStmt(query);

    CPostgreSQLLock lock(this);
    
    querySetStmt(query, new CPostgreSQLStatement);
    
    CPostgreSQLStatement* statement = (CPostgreSQLStatement*)query->statement();
    
    CParamValues& params = statement->m_paramValues;
    params.setParameters(query->params());
    
    const Oid* paramTypes = params.types();
    unsigned   paramCount = params.size();

    PGresult* stmt = PQprepare(m_connect, statement->name().c_str(), query->sql().c_str(), paramCount, paramTypes);
    if (PQresultStatus(stmt) != PGRES_COMMAND_OK) {
        string error = "PREPARE command failed: ";
        error += PQerrorMessage(m_connect);
        PQclear(stmt);
        query->logAndThrow("CPostgreSQLDatabase::queryPrepare", error);
    }
    
    PGresult* stmt2 = PQdescribePrepared(m_connect, statement->name().c_str());
    unsigned fieldCount = PQnfields(stmt2);
    if (fieldCount && PQftype(stmt2,0) == VOIDOID)
        fieldCount = 0;   // VOID result considered as no result
    PQclear(stmt2);
	
    statement->stmt(stmt, 0, fieldCount);
    
    querySetPrepared(query, true);
}

void CPostgreSQLDatabase::queryUnprepare(CQuery *query) {
    queryFreeStmt(query);
}

int CPostgreSQLDatabase::queryColCount(CQuery *query) {
    
    CPostgreSQLStatement* statement = (CPostgreSQLStatement*)query->statement();
    
    return statement->colCount();
}

void CPostgreSQLDatabase::queryBindParameters(CQuery *query) {
    CPostgreSQLLock lock(this);
    static const char* booleanTrue = "t";
    static const char* booleanFalse = "f";
    
    CPostgreSQLStatement* statement = (CPostgreSQLStatement*)query->statement();
    CParamValues&         paramValues = statement->m_paramValues;
    const CParamVector&   params = paramValues.params();
    uint32_t paramNumber = 0;
    for (CParamVector::const_iterator ptor = params.begin(); ptor != params.end(); ptor++, paramNumber++) {
        CParam *param = *ptor;
        CVariantType ptype = param->dataType();
            
        if (param->isNull())
            paramValues.setParameterValue(paramNumber, 0, 0);
        else {
            switch (ptype) {
                case VAR_BOOL:
                    if (param->asBool())
                        paramValues.setParameterValue(paramNumber, booleanTrue, 1);
                    else
                        paramValues.setParameterValue(paramNumber, booleanFalse, 1);
                    break;
                    
                case VAR_INT: {
                    int32_t* bufferToSend = (int32_t*) param->conversionBuffer();
                    *bufferToSend = htonl(param->getInteger());
                }
                break;

                case VAR_DATE: {
                    int32_t* bufferToSend = (int32_t*) param->conversionBuffer();
                    *bufferToSend = htonl((int32_t)param->getDateTime() - (int32_t) epochDate);
                }
                break;

                case VAR_DATE_TIME: {
                    double dt = (param->getDateTime() - epochDate) * 3600 * 24;
                    //int64_t* bufferToSend = (int64_t*) param->conversionBuffer();
                    //*bufferToSend = htonq(*(int64_t*)(void*)&dt);
                    htonq_inplace((uint64_t*) &dt,(uint64_t*) param->conversionBuffer());
                }
                break;

                case VAR_INT64: {
                    int64_t* bufferToSend = (int64_t*) param->conversionBuffer();
                    *bufferToSend = htonq(param->getInt64());
                }
                break;

                case VAR_FLOAT: {
                    int64_t* bufferToSend = (int64_t*) param->conversionBuffer();
                    *bufferToSend = htonq(*(int64_t*)param->dataBuffer());
                }
                break;

                case VAR_STRING:
                case VAR_TEXT:
                    //paramValues.setParameterValue(paramNumber, param->getString(), 0);
                    //break;
                    
                case VAR_BUFFER:
                    paramValues.setParameterValue(paramNumber, param->getString(), param->dataSize());
                    break;
                default:
                    query->logAndThrow("CPostgreSQLDatabase::queryBindParameters","Unsupported type of parameter "+int2string(paramNumber));
            }
        }
    }

#ifdef PG_FETCH_BINARY_RESULTS
    int resultFormat = 1;   // Results are presented in binary format
#else
    int resultFormat = 0;   // Results are presented in text format
#endif

    if (!statement->colCount())
        resultFormat = 0;   // VOID result or NO results, using text format
            
    PGresult* stmt = PQexecPrepared(
            m_connect,
            statement->name().c_str(),
            paramValues.size(),
            paramValues.values(),
            paramValues.lengths(),
            paramValues.formats(),
            resultFormat
            );
    
    ExecStatusType rc = PQresultStatus(stmt);
    switch (rc) {
        case PGRES_COMMAND_OK:
            statement->stmt(stmt, 0, 0);
            break;
        case PGRES_TUPLES_OK:
            statement->stmt(stmt, PQntuples(stmt));
            break;
        default: {
            string error = "EXECUTE command failed: ";
            error += PQerrorMessage(m_connect);
            PQclear(stmt);
            statement->clear();
            query->logAndThrow("CPostgreSQLDatabase::queryBindParameters",error);
        }
    }
}

void CPostgreSQLDatabase::PostgreTypeToCType(int postgreType, CVariantType& dataType) {
    switch (postgreType) {
        case PG_BOOL:      dataType = VAR_BOOL;      return;
        case PG_OID:
        case PG_INT2:
        case PG_INT4:      dataType = VAR_INT;       return;
        case PG_INT8:      dataType = VAR_INT64;     return;
        case PG_NUMERIC:
        case PG_FLOAT4:
        case PG_FLOAT8:    dataType = VAR_FLOAT;     return;
        case PG_BYTEA:     dataType = VAR_BUFFER;    return;
        case PG_DATE:      dataType = VAR_DATE;      return;
        case PG_TIME:
        case PG_TIMESTAMP: dataType = VAR_DATE_TIME; return;
        default:           dataType = VAR_STRING;    return;
    }
}

void CPostgreSQLDatabase::CTypeToPostgreType(CVariantType dataType, Oid& postgreType) {
    switch (dataType) {
        case VAR_INT:     postgreType = PG_INT4;    return;        ///< Integer 4 bytes
        case VAR_FLOAT:
        case VAR_MONEY:   postgreType = PG_FLOAT8;  return;        ///< Floating-point (double)
        case VAR_STRING:
        case VAR_TEXT:    postgreType = PG_VARCHAR; return;        ///< Varchar
        case VAR_BUFFER:  postgreType = PG_BYTEA;   return;        ///< Bytea
        case VAR_DATE:
        case VAR_DATE_TIME: postgreType = PG_TIMESTAMP; return;    ///< Timestamp
        case VAR_INT64:   postgreType = PG_INT8; return;           ///< Integer 8 bytes
        case VAR_BOOL:    postgreType = PG_BOOL; return;           ///< Boolean
        default:          throwException("Unsupported SPTK data type: " + int2string(dataType));
    }
}

void CPostgreSQLDatabase::queryOpen(CQuery *query) {
    if (!active()) open();
    
    if (query->active()) return;
    
    if (!query->statement())
        queryAllocStmt(query);
    
    if (!query->prepared())
        queryPrepare(query);
    
    // Bind parameters also executes a query
    queryBindParameters(query);
    
    //query->fields().clear();
    
    CPostgreSQLStatement* statement = (CPostgreSQLStatement*)query->statement();
    //if (statement->rowCount() == 0)
    //    return;
    
    short count = queryColCount(query);
    if (count < 1) {
        //queryCloseStmt(query);
        return;
    } else {
        querySetActive(query, true);
        if (query->fieldCount() == 0) {
           CPostgreSQLLock lock(this);
           // Reading the column attributes
           char  columnName[256];
           //long  columnType;
           //CVariantType dataType;
           const PGresult* stmt = statement->stmt();
           for (short column = 0; column < count; column++) {
               strncpy(columnName, PQfname(stmt, column), 255);
               columnName[255] = 0;
               if (columnName[0] == 0)
                   sprintf(columnName, "column%02i", column);
               Oid dataType = PQftype(stmt, column);
               CVariantType fieldType;
               PostgreTypeToCType(dataType, fieldType);
               int fieldLength = PQfsize(stmt, column);
               CPostgreSQLField* field = new CPostgreSQLField(columnName, column, dataType, fieldType, fieldLength);
               query->fields().push_back(field);
           }
        }
    }
    
    querySetEof(query, statement->eof());
    
    queryFetch(query);
}

// Converts internal NUMERIC Postgresql binary to long double
static long double numericBinaryToLongDouble(const char* v) {
    int16_t ndigits = ntohs(*(int16_t*)v);
    int16_t weight  = ntohs(*(int16_t*)(v+2));
    int16_t sign    = ntohs(*(int16_t*)(v+4));
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
	int16_t digit = ntohs(*(int16_t*)v);
	if (i <= weight)
	    value = value * 10000 + digit;
	else {
	    decValue = decValue * 10000 + digit;
	    divider *= 10000;
	}
    }
    long double finalValue = value + decValue / (long double)(divider);
    if (sign)
	finalValue = -finalValue;
    return finalValue;
}        

void CPostgreSQLDatabase::queryFetch(CQuery *query) {
    if (!query->active())
        query->logAndThrow("CPostgreSQLDatabase::queryFetch","Dataset isn't open");
    
    CPostgreSQLLock lock(this);
    
    CPostgreSQLStatement* statement = (CPostgreSQLStatement*) query->statement();
    
    statement->fetch();
    if (statement->eof()) {
        querySetEof(query, true);
        return;
    }
    
    int fieldCount = query->fieldCount();
    int dataLength = 0;
    
    if (!fieldCount)
        return;
    
    CPostgreSQLField*    field = 0;
    const PGresult*   stmt = statement->stmt();
    int               currentRow = statement->currentRow();
    for (int column = 0; column < fieldCount; column++) {
        try {
            field = (CPostgreSQLField*) & (*query)[(int)column];
            short fieldType = (short) field->fieldType();
	    
            bool  isNull = false;
            dataLength = PQgetlength(stmt, currentRow, column);
            if (!dataLength) {
                if (fieldType & (VAR_STRING|VAR_TEXT|VAR_BUFFER))
                    isNull = PQgetisnull(stmt, currentRow, column);
                else
                    isNull = true;
                if (isNull)
                    field->setNull();
                else
                    field->setExternalString("", 0);
            } else {
                char* data = PQgetvalue(stmt, currentRow, column);

#ifdef PG_FETCH_BINARY_RESULTS
                switch (fieldType) {
                    
                    case PG_BOOL:
                        field->setBool((bool)*data);
                        dataLength = sizeof(bool);
                        break;
                        
                    case PG_INT2:
                        field->setInteger( ntohs(*(int16_t *)data) );
                        break;
                        
                    case PG_OID:
                    case PG_INT4:
                        field->setInteger( ntohl(*(int32_t *)data) );
                        break;
                        
                    case PG_INT8:
                        field->setInt64( ntohq(*(int64_t *)data) );
                        break;
                        
                   case PG_FLOAT4: {
                        int32_t v = ntohl(*(int32_t *)data);
                        field->setFloat( *(float *)(void *)&v );
                        break;
                   }
                        
                   case PG_FLOAT8: {
                        int64_t v = ntohq(*(int64_t *)data);
                        field->setFloat( *(double *)(void *)&v );
                        break;
                   }
                        
                    case PG_NUMERIC:
                        field->setFloat( numericBinaryToLongDouble(data) );
                        break;
                        
                    default:
                        field->setExternalString(data, dataLength);
                        break;
                        
                    case PG_BYTEA:
                        field->setExternalBuffer(data, dataLength);
                        break;
                        
                   case PG_DATE: {
                        int32_t dt = ntohl(*(int32_t *)data);
                        field->setDateTime( dt + (int32_t) epochDate );
                        break;
                   }
                        
                   case PG_TIME: 
                   case PG_TIMESTAMPTZ: 
                   case PG_TIMESTAMP: {
                        int64_t v = ntohq(*(int64_t *)data);
                        double val = (double)epochDate + *(double *)(void *)&v / 3600.0 / 24.0;
                        field->setDateTime(val);
                        break;
                   }
                }
#else
                switch (fieldType) {
                    case PG_BOOL:
                        field->setBool(*data == 't');
                        break;
                        
                    case PG_INT2:
                    case PG_OID:
                    case PG_INT4:
                        field->setInteger( atoi(data) );
                        break;
                        
                    case PG_INT8:
                        field->setInt64( atoll(data) );
                        break;
                        
                    case PG_FLOAT4:
                    case PG_FLOAT8:
                    case PG_NUMERIC:
                        field->setFloat( atof(data) );
                        break;
                        
                    default:
                        field->setString(data, dataLength);
                        break;
                        
                    case PG_BYTEA:
                        field->setBuffer(data, dataLength);
                        break;
                        
                    case PG_TIMESTAMP:
                        field->setDateTime(data);
                        break;
                }
#endif
                field->dataSize(dataLength);
            }

        } catch (exception& e) {
            query->logAndThrow("CPostgreSQLDatabase::queryFetch","Can't read field "+field->fieldName()+": "+string(e.what()));
        }
    }
}

void CPostgreSQLDatabase::objectList(CDbObjectType objectType, CStrings& objects) throw(std::exception) {
    string tablesSQL("SELECT table_schema || '.' || table_name "
            "FROM information_schema.tables "
            "WHERE table_schema NOT IN ('information_schema','pg_catalog') ");
    string objectsSQL;
    objects.clear();
    switch (objectType) {
        case DOT_PROCEDURES:  objectsSQL = "SELECT DISTINCT routine_schema || '.' || routine_name "
                "FROM information_schema.routines "
                "WHERE routine_schema NOT IN ('information_schema','pg_catalog')";
        break;
        case DOT_TABLES:      objectsSQL = tablesSQL + "AND table_type = 'BASE TABLE'"; break;
        case DOT_VIEWS:       objectsSQL = tablesSQL + "AND table_type = 'VIEW'"; break;
        default:              return; // no information about objects of other types
    }
    CQuery query(this, objectsSQL);
    query.open();
    while (!query.eof()) {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }
    query.close();
}

std::string CPostgreSQLDatabase::driverDescription() const {
    return "PostgreSQL";
}

std::string CPostgreSQLDatabase::paramMark(unsigned paramIndex) {
    char mark[16];
    sprintf(mark, "$%i", paramIndex+1);
    return mark;
}

#endif
