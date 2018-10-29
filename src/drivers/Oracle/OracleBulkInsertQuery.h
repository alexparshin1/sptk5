/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       COracleBulkInsertQuery.h - description                 ║
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

#ifndef __CORACLEBULKINSERTQUERY_H__
#define __CORACLEBULKINSERTQUERY_H__

#include <sptk5/db/Query.h>
#include <sptk5/db/OracleConnection.h>
#include <sptk5/Exception.h>

namespace sptk {

/// @addtogroup Database Database Support
/// @{

/// @brief Oracle bulk insert query
///
/// This class is dedicated for internal use only
class SP_EXPORT OracleBulkInsertQuery : public Query
{
    friend class OracleConnection;

    size_t              m_recordCount;      ///< Inserted record count
    size_t              m_recordNumber;     ///< Current record number
    size_t              m_batchSize;        ///< Batch size
    bool                m_lastIteration;    ///< Last iteration
    QueryColumnTypeSizeMap  m_columnTypeSizes;  ///< Column type sizes

protected:
    /// @brief Constructor
    /// @param db DatabaseConnection, the database to connect to, optional
    /// @param sql std::string, the SQL query text to use, optional
    /// @param recordCount size_t, number of records to insert
    OracleBulkInsertQuery(PoolDatabaseConnection *db, const std::string& sql, size_t recordCount, const QueryColumnTypeSizeMap& columnTypeSizes);

    /// @brief Destructor
    ~OracleBulkInsertQuery() override = default;

public:
    /// @brief Executes next iteration of bulk insert
    void execNext();

    size_t batchSize() const { return m_batchSize; }
    bool lastIteration() const { return m_lastIteration; }
    const QueryColumnTypeSizeMap columnTypeSizes() const { return m_columnTypeSizes; }
};
/// @}
}
#endif
