/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CFirebirdStatement.h  -  description
                             -------------------
    begin                : Wed Jul 24 2013
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#include <sptk5/db/CFirebirdConnection.h>
#include <sptk5/db/CFirebirdStatement.h>
#include <sptk5/CFieldList.h>
#include <sptk5/CBuffer.h>
#include <math.h>

using namespace std;
using namespace sptk;

class CFirebirdStatementField: public CDatabaseField {
    XSQLVAR&        m_sqlvar;
public:
    // Callback variables
    ISC_SHORT       m_cbNull;
    ISC_QUAD        m_blob_id;
    double          m_numericScale;

public:
    CFirebirdStatementField(std::string fieldName, int fieldColumn, int fieldType, CVariantType dataType, int fieldSize, XSQLVAR& sqlvar) :
        CDatabaseField(fieldName, fieldColumn, fieldType & 0xFFFE, dataType, fieldSize),
        m_sqlvar(sqlvar), m_cbNull(0), m_numericScale(1)
    {
        setInt64(0);
        m_sqlvar.sqldata = (ISC_SCHAR*) &getInt64();
        fieldType &= 0xFFFE;
        switch (fieldType) {
            case SQL_LONG:
            case SQL_INT64:
                if (sqlvar.sqlsubtype != 1) {
                    break;
                }
            case SQL_FLOAT:
            case SQL_D_FLOAT:
            case SQL_DOUBLE:
                m_numericScale = exp10(sqlvar.sqlscale);
                break;
            case SQL_TEXT:
            case SQL_BLOB:
            case SQL_VARYING:
                setBuffer(NULL,fieldSize + 1);
                m_sqlvar.sqldata = (ISC_SCHAR*) getBuffer();
                break;
        }
        m_sqlvar.sqlind = &m_cbNull;
    }

    void setDataSize(uint32_t sz)
    {
        dataSize(sz);
    }
    
    void clearNull()
    {
        m_dataType &= ~VAR_NULL;
    }
};


CFirebirdStatement::CFirebirdStatement(CFirebirdConnection* connection, string sql)
: CDatabaseStatement<CFirebirdConnection,isc_stmt_handle>(connection)
{
    m_statement = new isc_stmt_handle;
    *m_statement = 0;
}

CFirebirdStatement::~CFirebirdStatement()
{
    if (*m_statement)
        isc_dsql_free_statement(m_status_vector, m_statement, DSQL_drop);
    delete m_statement;
}

void CFirebirdStatement::dateTimeToFirebirdDate(struct tm& firebirdDate, CDateTime timestamp, CVariantType timeType)
{
    short year, month, day, hour, minute, second, msecond;
    memset(&firebirdDate, 0, sizeof(firebirdDate));
    if (timeType == VAR_DATE) {
        // Date only
        timestamp.decodeDate(&year, &month, &day);
        firebirdDate.tm_year = year - 1900;
        firebirdDate.tm_mon = month - 1;
        firebirdDate.tm_mday = day;
    } else {
        // Time only
        timestamp.decodeTime(&hour, &minute, &second, &msecond);
        firebirdDate.tm_hour = hour;
        firebirdDate.tm_min = minute;
        firebirdDate.tm_sec = second;
    }
}

void CFirebirdStatement::firebirdDateToDateTime(CDateTime& timestamp, const struct tm& firebirdData)
{
    timestamp = CDateTime(firebirdData.tm_year + 1900, short(firebirdData.tm_mon + 1), short(firebirdData.tm_mday),
                short(firebirdData.tm_hour), short(firebirdData.tm_min), short(firebirdData.tm_sec));
}

void CFirebirdStatement::enumerateParams(CParamList& queryParams)
{
    CDatabaseStatement::enumerateParams(queryParams);
    XSQLDA* psqlda = &m_paramBuffers.sqlda();
    isc_dsql_describe_bind(m_status_vector, m_statement, 1, psqlda);
    m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
    if (psqlda->sqln != psqlda->sqld) {
        m_paramBuffers.resize(psqlda->sqld);
        isc_dsql_describe_bind(m_status_vector, m_statement, 1, psqlda);
        m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
    }
}

CVariantType CFirebirdStatement::firebirdTypeToVariantType(int firebirdType, int firebirdSubtype)
{
    switch (firebirdType) {

    case SQL_SHORT:
        return VAR_INT;

    case SQL_LONG:
        if (firebirdSubtype == 1)
            return VAR_FLOAT;
        return VAR_INT;

    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_D_FLOAT:
        return VAR_FLOAT;

    case SQL_BLOB:
        return VAR_BUFFER;

    case SQL_TYPE_DATE:
        return VAR_DATE;

    case SQL_TIMESTAMP:
    case SQL_TYPE_TIME:
        return VAR_DATE_TIME;

    case SQL_INT64:
        if (firebirdSubtype == 1)
            return VAR_FLOAT;
        return VAR_INT64;

    case SQL_TEXT:
        if (firebirdSubtype == 1)
            return VAR_BUFFER;
        return VAR_STRING;
        
    case SQL_VARYING:
        // Anything we don't know about - treat as string
    default:
        return VAR_STRING;
    }
}

int CFirebirdStatement::variantTypeToFirebirdType(CVariantType dataType)
{
    switch (dataType) {

    case VAR_NONE:
        return SQL_NULL;

    case VAR_INT:
        return SQL_LONG;

    case VAR_FLOAT:
    case VAR_MONEY:
        return SQL_DOUBLE;

    case VAR_TEXT:
    case VAR_BUFFER:
        return SQL_BLOB;

    case VAR_DATE:
        return SQL_TYPE_DATE;
        
    case VAR_DATE_TIME:
        return SQL_TIMESTAMP;

    case VAR_INT64:
        return SQL_INT64;

    case VAR_STRING:
        // Anything we don't know about - treat as string
    default:
        return SQL_VARYING;
    }
}

void CFirebirdStatement::setParameterValues()
{
    unsigned        paramCount = m_enumeratedParams.size();
    struct tm       firebirdDateTime;
    ISC_TIMESTAMP*  pts;
    for (unsigned paramIndex = 0; paramIndex < paramCount; paramIndex++) {
        CParam*     param = m_enumeratedParams[paramIndex];
        XSQLVAR&    sqlvar = m_paramBuffers[paramIndex];

        if (param->isNull())
            *sqlvar.sqlind = 1;
        else
            *sqlvar.sqlind = 0;
        
        switch (param->dataType()) {
            
            case VAR_NONE:
                sqlvar.sqltype = SQL_TEXT + 1;
                sqlvar.sqllen = 0;
                sqlvar.sqldata = (ISC_SCHAR*) "";
                param->setNull();
                break;

            case VAR_BOOL:
                sqlvar.sqltype = SQL_SHORT + 1;
                sqlvar.sqllen = sizeof(short);
                sqlvar.sqldata = (ISC_SCHAR*) &param->getInteger();
                break;
                
            case VAR_INT:
                sqlvar.sqltype = SQL_LONG + 1;
                sqlvar.sqllen = sizeof(int32_t);
                sqlvar.sqldata = (ISC_SCHAR*) &param->getInteger();
                break;
                
            case VAR_FLOAT:
            case VAR_MONEY:
                sqlvar.sqltype = SQL_DOUBLE + 1;
                sqlvar.sqllen = sizeof(double);
                sqlvar.sqldata = (ISC_SCHAR*) &param->getFloat();
                break;
                
            case VAR_INT64:
                sqlvar.sqltype = SQL_INT64 + 1;
                sqlvar.sqllen = sizeof(int64_t);
                sqlvar.sqldata = (ISC_SCHAR*) &param->getInt64();
                break;
                
            case VAR_STRING:
                sqlvar.sqltype = SQL_TEXT + 1;
                sqlvar.sqllen = param->dataSize();
                sqlvar.sqldata = (ISC_SCHAR*) param->getBuffer();
                break;

            case VAR_TEXT:
            case VAR_BUFFER:
                sqlvar.sqltype = SQL_BLOB + 1;
                sqlvar.sqllen = sizeof(ISC_QUAD);
                sqlvar.sqldata = (ISC_SCHAR*) param->conversionBuffer();
                createBLOB((ISC_QUAD*)sqlvar.sqldata, param);
                break;
                
            case VAR_DATE:
                sqlvar.sqltype = SQL_TYPE_DATE + 1;
                sqlvar.sqllen = sizeof(ISC_DATE);
                sqlvar.sqldata = (ISC_SCHAR*) param->conversionBuffer();
                dateTimeToFirebirdDate(firebirdDateTime, param->getDate(), VAR_DATE);
                isc_encode_sql_date(&firebirdDateTime, (ISC_DATE*)sqlvar.sqldata);
                break;
                
            case VAR_DATE_TIME:
                sqlvar.sqltype = SQL_TIMESTAMP + 1;
                sqlvar.sqllen = sizeof(ISC_TIMESTAMP);
                sqlvar.sqldata = (ISC_SCHAR*) param->conversionBuffer();
                pts = (ISC_TIMESTAMP*) sqlvar.sqldata;
                
                dateTimeToFirebirdDate(firebirdDateTime, param->getDateTime(), VAR_DATE);
                isc_encode_sql_date(&firebirdDateTime, &pts->timestamp_date);
                
                dateTimeToFirebirdDate(firebirdDateTime, param->getDateTime(), VAR_DATE_TIME);
                isc_encode_sql_time(&firebirdDateTime, &pts->timestamp_time);
                break;
                
            default:
            {
                char buffer[256];
                sprintf(buffer, "Unsupported Firebird type %i", sqlvar.sqltype & 0xFFFE);
                throw CDatabaseException(buffer);
            }
        }
    }
}

void CFirebirdStatement::CFirebirdStatement::prepare(const string& sql)
{
    if (!m_statement || !*m_statement) {
        isc_db_handle db = m_connection->connection();
        isc_dsql_allocate_statement(m_status_vector, &db, m_statement);
        m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
    }
    
    if (!m_connection->m_transaction) 
        m_connection->driverBeginTransaction();
    
    isc_dsql_prepare(m_status_vector, &m_connection->m_transaction, m_statement, 0, sql.c_str(), SQL_DIALECT_CURRENT, NULL);
    m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
}

void CFirebirdStatement::execute(bool)
{
    if (!m_connection->m_transaction) 
        m_connection->driverBeginTransaction();
    
    m_state.eof = false;
    isc_dsql_execute(m_status_vector, &m_connection->m_transaction, m_statement, 1, &m_paramBuffers.sqlda());
    m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);

    // Allocate result buffers
    XSQLDA* osqlda = &m_outputBuffers.sqlda();
    isc_dsql_describe(m_status_vector, m_statement, 1, osqlda);
    if (osqlda->sqln < osqlda->sqld) {
        m_outputBuffers.resize(osqlda->sqld);
        isc_dsql_describe(m_status_vector, m_statement, 1, osqlda);
    }
            
    m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
    
    m_state.columnCount = osqlda->sqld;
}

void CFirebirdStatement::bindResult(CFieldList& fields)
{
    fields.clear();

    char columnName[256];
    for (int columnIndex = 0; columnIndex < m_state.columnCount; columnIndex++) {
        XSQLVAR& sqlvar = m_outputBuffers[columnIndex];
        ISC_SHORT type = sqlvar.sqltype;
        
        if (type == SQL_TEXT && sqlvar.sqlsubtype)
            type = SQL_BLOB;

        strncpy(columnName, sqlvar.aliasname, sizeof(columnName));
        columnName[sizeof(columnName)-1] = 0;
        if (columnName[0] == 0)
            sprintf(columnName, "column_%02i", columnIndex + 1);

        CVariantType fieldType = firebirdTypeToVariantType(sqlvar.sqltype, sqlvar.sqlsubtype);
        unsigned fieldLength = sqlvar.sqllen;
        fields.push_back(new CFirebirdStatementField(columnName, columnIndex, sqlvar.sqltype, fieldType, fieldLength, sqlvar));
    }
}

isc_blob_handle CFirebirdStatement::createBLOB(ISC_QUAD* blob_id, CParam* param) throw(CDatabaseException)
{
    isc_db_handle   db = m_connection->connection();
    isc_blob_handle blob_handle = 0;

    isc_create_blob2(
        m_status_vector,
        &db,
        &m_connection->m_transaction,
        &blob_handle,
        blob_id,
        0, // Blob Parameter Buffer length = 0; no filter will be used
        NULL // NULL Blob Parameter Buffer, since no filter will be used
    );
    m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);

    const char *segment = param->getBuffer();
    size_t remaining = param->dataSize();
    while (remaining)
    {
        size_t segmentSize = remaining > 16384 ? 16384 : remaining;
        isc_put_segment(
            m_status_vector,
            &blob_handle,
            segmentSize,
            segment
        );
        m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
        remaining -= segmentSize;
        segment += segmentSize;
    }
    isc_close_blob(m_status_vector, &blob_handle);
    m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
    
    return blob_handle;
}

size_t CFirebirdStatement::fetchBLOB(ISC_QUAD* blob_id, CDatabaseField* field) throw(CDatabaseException)
{
    isc_db_handle   db = m_connection->connection();
    
    if (blob_id->gds_quad_low == 0 && blob_id->gds_quad_high == 0)
        return 0;
    
    isc_blob_handle blob_handle = 0;
    isc_open_blob2(m_status_vector, &db, &m_connection->m_transaction, &blob_handle, blob_id, 0, NULL);
    m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
    
    size_t  dataLength = 0;
    size_t  fragmentSize = 8192;
    
    field->checkSize(fragmentSize);
    
    while (true) {
        char* currentFragment = (char*) field->getBuffer() + dataLength;
        unsigned short  fragmentLength = 0;
        isc_get_segment(m_status_vector, &blob_handle, &fragmentLength, fragmentSize, currentFragment);
        if (fragmentLength == 0)
            break;
        dataLength += fragmentLength;
        field->checkSize(dataLength + fragmentSize);
    }

    isc_close_blob(m_status_vector, &blob_handle);
    
    field->setDataSize(dataLength);
    
    return field->dataSize();
}

void CFirebirdStatement::fetchResult(CFieldList& fields)
{
    struct tm       times;
    uint32_t        fieldCount = fields.size();
    
    for (uint32_t fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++) {
        CFirebirdStatementField*    field = (CFirebirdStatementField*) &fields[fieldIndex];
        XSQLVAR&                    sqlvar = m_outputBuffers[fieldIndex];
        if (*sqlvar.sqlind) {
            field->setNull();
            continue;
        }
        field->clearNull();
        switch (sqlvar.sqltype & 0xFFFE) {
            case SQL_BLOB:
                fetchBLOB((ISC_QUAD*)sqlvar.sqldata, field);
                break;
                    
            // Date and time types
            case SQL_TYPE_DATE:
                isc_decode_sql_date((ISC_DATE*)sqlvar.sqldata, &times);
                field->setDate(CDateTime(times.tm_year + 1900, times.tm_mon + 1, times.tm_mday));
                break;
                        
            case SQL_TYPE_TIME:
                isc_decode_sql_time((ISC_TIME*)sqlvar.sqldata, &times);
                field->setDateTime(CDateTime(times.tm_year + 1900, times.tm_mon + 1, times.tm_mday, times.tm_hour, times.tm_min, times.tm_sec));
                break;
                        
            case SQL_TIMESTAMP:
                isc_decode_timestamp((ISC_TIMESTAMP*)sqlvar.sqldata, &times);
                field->setDateTime(CDateTime(times.tm_year + 1900, times.tm_mon + 1, times.tm_mday, times.tm_hour, times.tm_min, times.tm_sec));
                break;
                        
            case SQL_SHORT:
                field->setInteger(*(short*) sqlvar.sqldata);
                break;
                        
            case SQL_LONG:
                if (sqlvar.sqlsubtype == 1)
                    field->setFloat(*(int32_t*) sqlvar.sqldata * field->m_numericScale);
                else
                    field->setInteger(*(int32_t*) sqlvar.sqldata);
                break;
                        
            case SQL_INT64:
                if (sqlvar.sqlsubtype == 1)
                    field->setFloat(*(int64_t*) sqlvar.sqldata * field->m_numericScale);
                else
                    field->setInt64(*(int64_t*) sqlvar.sqldata);
                break;
                        
            case SQL_FLOAT:
                field->setFloat(*(float*) sqlvar.sqldata * field->m_numericScale);
                break;
                        
            case SQL_DOUBLE:
            case SQL_D_FLOAT:
                field->setFloat(*(double*) sqlvar.sqldata * field->m_numericScale);
                break;
                        
            case SQL_TEXT:
                {
                    int pos = sqlvar.sqllen - 1;
                    while (sqlvar.sqldata[pos] == ' ' && pos >= 0)
                        pos--;
                    pos++;
                    sqlvar.sqldata[pos] = 0;
                    break;
                }
                
            case SQL_VARYING:
                {
                    size_t len = *(uint16_t*) sqlvar.sqldata;
                    field->setString(sqlvar.sqldata + 2, len);
                }
                break;
                        
            default:
                {
                    char buffer[256];
                    sprintf(buffer, "Unsupported Firebird type %i", sqlvar.sqltype & 0xFFFE);
                    throw CDatabaseException(buffer);
                }
        }
    }
}

void CFirebirdStatement::close()
{
    isc_dsql_free_statement(m_status_vector, m_statement, DSQL_close);
    m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
}

void CFirebirdStatement::fetch()
{
    ISC_STATUS retcode = isc_dsql_fetch(m_status_vector, m_statement, 1, &m_outputBuffers.sqlda());
    if (retcode == 100)
        m_state.eof = true;
    else {
        m_connection->checkStatus(m_status_vector, __FILE__, __LINE__);
        m_state.eof = false;
    }
}
