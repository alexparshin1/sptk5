/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        CFirebirdStatement.h - description                    ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CFIREBIRDSTATEMENT_H__
#define __CFIREBIRDSTATEMENT_H__

#include <ibase.h>

#include <list>
#include <string>
#include <stdio.h>
#include <sys/time.h>

#include <sptk5/db/CDatabaseField.h>
#include <sptk5/db/CDatabaseStatement.h>

namespace sptk
{

class CFirebirdConnection;

/// @brief Firebird-specific bind buffers
class CFirebirdBindBuffers
{
    size_t  m_size;     ///< Buffer count
    XSQLDA* m_sqlda;    ///< Buffers structure
    short*  m_cbNulls;  ///< Null flags (callback)
public:
    /// @brief Constructor
    CFirebirdBindBuffers()
    : m_size(0), m_sqlda(NULL), m_cbNulls(NULL)
    {
        resize(16);
    }

    /// @brief Destructor
    ~CFirebirdBindBuffers()
    {
        free(m_sqlda);
        free(m_cbNulls);
    }
    
    /// @brief Returns buffer structure
    XSQLDA& sqlda()
    {
        return *m_sqlda;
    }
    
    /// @brief Resize bind buffers
    /// @param size size_t, New size of buffer array
    void resize(size_t size)
    {
        if (size < 1)
            size = 1;
        if (size > 1024)
            size = 1024;
        m_size = size;
        m_sqlda = (XSQLDA *) realloc(m_sqlda, XSQLDA_LENGTH(m_size));
        m_sqlda->version = SQLDA_VERSION1;
        m_sqlda->sqln = (ISC_SHORT) m_size;
        m_cbNulls = (short*) realloc(m_cbNulls, size * sizeof(short));
        short* cbNull = m_cbNulls;
        for (unsigned i = 0; i < m_size; i++, cbNull++)
            m_sqlda->sqlvar[i].sqlind = cbNull;
    }
    
    /// @brief Returns individual buffer
    XSQLVAR& operator [] (size_t index)
    {
        return m_sqlda->sqlvar[index];
    }
    
    /// @brief Returns number of buffers
    size_t size() const
    {
        return m_size;
    }
};

/// @brief Firebird SQL statement
class CFirebirdStatement : public CDatabaseStatement<CFirebirdConnection,isc_stmt_handle>
{
    CFirebirdBindBuffers    m_outputBuffers;        ///< Output result buffers
    CFirebirdBindBuffers    m_paramBuffers;         ///< Parameter buffers
    ISC_STATUS              m_status_vector[20];    ///< Execution result
    CBuffer                 m_blobData;             ///< BLOB fetch buffer
    
public:

    /// @brief Translates Firebird native type to CVariant type
    /// @param firebirdType int, Firebird native type
    /// @param firebirdSubtype int, Firebird native subtype
    /// @returns CVariant type
    static VariantType firebirdTypeToVariantType(int firebirdType, int firebirdSubtype);

    /// @brief Translates CDateTime to Firebird time
    /// @param firebirdDate tm&, Firebird time
    /// @param timestamp CDateTime, Timestamp
    /// @param timeType CVariantType, Time type, VAR_DATE or VAR_DATETIME
    static void dateTimeToFirebirdDate(struct tm& firebirdDate, CDateTime timestamp, VariantType timeType);

    /// @brief Translates Firebird time to CDateTime
    /// @param timestamp CDateTime&, Timestamp
    /// @param firebirdDate const tm&, Firebird time
    static void firebirdDateToDateTime(CDateTime& timestamp, const struct tm& firebirdDate);

    /// @brief Creates new BLOB from parameter data
    /// @param blob_id ISC_QUAD*, Firebird-specific BLOB id
    /// @param param CParam*, BLOB field
    isc_blob_handle createBLOB(ISC_QUAD* blob_id, CParam* param) THROWS_EXCEPTIONS;
    
    /// @brief Fetches BLOB data during fetch of query results
    /// @param blob_id ISC_QUAD*, Firebird-specific BLOB id
    /// @param field CDatabaseField*, BLOB field
    size_t fetchBLOB(ISC_QUAD* blob_id, CDatabaseField* field) THROWS_EXCEPTIONS;
    
public:
    /// @brief Constructor
    /// @param connection Connection*, Firebird connection
    /// @param sql std::string, SQL statement
    CFirebirdStatement(CFirebirdConnection* connection, std::string sql);

    /// @brief Destructor
    virtual ~CFirebirdStatement();

    /// @brief Generates normalized list of parameters
    /// @param queryParams CParamList&, Standard query parameters
    void enumerateParams(CParamList& queryParams);

    /// @brief Sets actual parameter values for the statement execution
    void setParameterValues();

    /// @brief Prepares Firebird statement
    /// @param sql const std::string, statement SQL
    void prepare(const std::string& sql);

    /// @brief Executes statement
    void execute(bool);

    /// @brief Binds statement result metadata to query fields
    /// @param fields CFieldList&, query fields (if any)
    void bindResult(CFieldList& fields);

    /// @brief Fetches statement result metadata to query fields
    /// @param fields CFieldList&, query fields (if any)
    void fetchResult(CFieldList& fields);

    /// @brief Closes statement and releases allocated resources
    void close();

    /// @brief Fetches next record
    void fetch();
};

}

#endif

