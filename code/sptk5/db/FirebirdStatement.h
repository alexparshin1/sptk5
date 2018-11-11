/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        FirebirdStatement.h - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __SPTK_FIREBIRDSTATEMENT_H__
#define __SPTK_FIREBIRDSTATEMENT_H__

#include <ibase.h>

#include <list>
#include <string>
#include <stdio.h>
#include <sys/time.h>

#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/DatabaseStatement.h>

namespace sptk
{

/**
 * @brief Firebird-specific bind buffers
 */
class FirebirdBindBuffers
{
    /**
     * Buffer count
     */
    size_t  m_size;

    /**
     * Buffers structure
     */
    XSQLDA* m_sqlda;

    /**
     * Null flags (callback)
     */
    short*  m_cbNulls;

public:
    /**
     * @brief Constructor
     */
    FirebirdBindBuffers()
    : m_size(0), m_sqlda(NULL), m_cbNulls(NULL)
    {
        resize(16);
    }

    /**
     * @brief Destructor
     */
    ~FirebirdBindBuffers()
    {
        free(m_sqlda);
        free(m_cbNulls);
    }

    /**
     * @brief Returns buffer structure
     */
    XSQLDA& sqlda()
    {
        return *m_sqlda;
    }

    /**
     * @brief Resize bind buffers
     * @param size size_t, New size of buffer array
     */
    void resize(size_t size)
    {
        if (size < 1)
            size = 1;
        if (size > 1024)
            size = 1024;
        m_size = size;

        XSQLDA *newptr = (XSQLDA *) realloc(m_sqlda, XSQLDA_LENGTH(m_size));
        if (newptr == nullptr)
            throw Exception("Can't allocate memory for Firebird statement");
        m_sqlda = newptr;

        m_sqlda->version = SQLDA_VERSION1;
        m_sqlda->sqln = (ISC_SHORT) m_size;

        short* nptr = (short*) realloc(m_cbNulls, size * sizeof(short));
        if (nptr == nullptr)
            throw Exception("Can't allocate memory for Firebird statement");
        m_cbNulls = nptr;

        short* cbNull = m_cbNulls;
        for (unsigned i = 0; i < m_size; i++, cbNull++)
            m_sqlda->sqlvar[i].sqlind = cbNull;
    }

    /**
     * @brief Returns individual buffer
     */
    XSQLVAR& operator [] (size_t index)
    {
        return m_sqlda->sqlvar[index];
    }

    /**
     * @brief Returns number of buffers
     */
    size_t size() const
    {
        return m_size;
    }
};

/**
 * @brief Firebird SQL statement
 */
class FirebirdStatement : public DatabaseStatement<FirebirdConnection,isc_stmt_handle>
{
    /**
     * Output result buffers
     */
    FirebirdBindBuffers    m_outputBuffers;

    /**
     * Parameter buffers
     */
    FirebirdBindBuffers    m_paramBuffers;

    /**
     * Execution result
     */
    ISC_STATUS              m_status_vector[20];

public:

    /**
     * @brief Translates Firebird native type to CVariant type
     * @param firebirdType int, Firebird native type
     * @param firebirdSubtype int, Firebird native subtype
     * @returns CVariant type
     */
    static VariantType firebirdTypeToVariantType(int firebirdType, int firebirdSubtype);

    /**
     * @brief Translates DateTime to Firebird time
     * @param firebirdDate tm&, Firebird time
     * @param timestamp DateTime, Timestamp
     * @param timeType VariantType, Time type, VAR_DATE or VAR_DATETIME
     */
    static void dateTimeToFirebirdDate(struct tm& firebirdDate, DateTime timestamp, VariantType timeType);

    /**
     * @brief Translates Firebird time to DateTime
     * @param timestamp DateTime&, Timestamp
     * @param firebirdDate const tm&, Firebird time
     */
    static void firebirdDateToDateTime(DateTime& timestamp, const struct tm& firebirdDate);

    /**
     * @brief Creates new BLOB from parameter data
     * @param blob_id ISC_QUAD*, Firebird-specific BLOB id
     * @param param QueryParameter*, BLOB field
     */
    isc_blob_handle createBLOB(ISC_QUAD* blob_id, QueryParameter* param);

    /**
     * @brief Fetches BLOB data during fetch of query results
     * @param blob_id ISC_QUAD*, Firebird-specific BLOB id
     * @param field DatabaseField*, BLOB field
     */
    size_t fetchBLOB(ISC_QUAD* blob_id, DatabaseField* field);

    /**
     * @brief Constructor
     * @param connection Connection*, Firebird connection
     * @param sql std::string, SQL statement
     */
    FirebirdStatement(FirebirdConnection* connection, std::string sql);

    /**
     * @brief Destructor
     */
    virtual ~FirebirdStatement();

    /**
     * @brief Generates normalized list of parameters
     * @param queryParams QueryParameterList&, Standard query parameters
     */
    void enumerateParams(QueryParameterList& queryParams);

    /**
     * @brief Sets actual parameter values for the statement execution
     */
    void setParameterValues();

    /**
     * @brief Prepares Firebird statement
     * @param sql const std::string, statement SQL
     */
    void prepare(const std::string& sql);

    /**
     * @brief Executes statement
     */
    void execute(bool);

    /**
     * @brief Binds statement result metadata to query fields
     * @param fields CFieldList&, query fields (if any)
     */
    void bindResult(FieldList& fields);

    /**
     * @brief Fetches statement result metadata to query fields
     * @param fields CFieldList&, query fields (if any)
     */
    void fetchResult(FieldList& fields);

    /**
     * @brief Closes statement and releases allocated resources
     */
    void close();

    /**
     * @brief Fetches next record
     */
    void fetch();
};

}

#endif

