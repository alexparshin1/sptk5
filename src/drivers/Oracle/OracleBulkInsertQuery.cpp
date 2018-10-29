/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       COracleBulkInsertQuery.cpp - description               ║
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

#include "OracleBulkInsertQuery.h"

using namespace std;
using namespace sptk;

OracleBulkInsertQuery::OracleBulkInsertQuery(PoolDatabaseConnection *db, const std::string& sql, size_t recordCount, const QueryColumnTypeSizeMap& columnTypeSizes)
: Query(db, sql), m_recordCount(recordCount), m_recordNumber(0), m_batchSize(2), m_lastIteration(false), m_columnTypeSizes(columnTypeSizes)
{
    m_bulkMode = true;
}

void OracleBulkInsertQuery::execNext()
{
    m_recordNumber++;
    m_lastIteration = (m_recordNumber == m_recordCount || (m_recordNumber % m_batchSize) == 0);
    exec();
}
