/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-05-04                                             ║
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

#include <sptk5/db/DatabaseConnectionPool.h>

using namespace std;
using namespace sptk;

const String AutoDatabaseConnection::s_invalidConnectionMessage {"DB connection not active"};

AutoDatabaseConnection::AutoDatabaseConnection(DatabaseConnectionPool& connectionPool)
    : m_connectionPool(connectionPool)
{
    m_connection = m_connectionPool.createConnection();
    if (m_connection == nullptr)
    {
        throw Exception("Failed to create database connection");
    }
}

AutoDatabaseConnection::~AutoDatabaseConnection()
{
    if (m_connection != nullptr)
    {
        try
        {
            if (m_connection->active())
            {
                m_connection->close();
            }
            m_connectionPool.releaseConnection(m_connection);
        }
        catch (const Exception& e)
        {
            CERR(e.what());
        }
    }
}

SPoolDatabaseConnection AutoDatabaseConnection::connection() const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    return m_connection;
}

void AutoDatabaseConnection::open(const String& connectionString) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->open(connectionString);
}

void AutoDatabaseConnection::close() const
{
    if (active())
    {
        m_connection->close();
    }
}

bool AutoDatabaseConnection::active() const
{
    return m_connection != nullptr && m_connection->active();
}

const DatabaseConnectionString& AutoDatabaseConnection::connectionString() const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    return m_connection->connectionString();
}

DatabaseConnectionType AutoDatabaseConnection::connectionType() const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    return m_connection->connectionType();
}

String AutoDatabaseConnection::driverDescription() const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    return m_connection->driverDescription();
}

void AutoDatabaseConnection::beginTransaction() const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->beginTransaction();
}

void AutoDatabaseConnection::commitTransaction() const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->commitTransaction();
}

void AutoDatabaseConnection::rollbackTransaction() const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->rollbackTransaction();
}

void AutoDatabaseConnection::objectList(const DatabaseObjectType objectType, Strings& objects) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->objectList(objectType, objects);
}

void AutoDatabaseConnection::bulkInsert(const String& tableName, const String& autoIncrementColumnName, const Strings& columnNames,
                                        std::vector<VariantVector>& data, std::vector<int64_t>& insertedIds, const size_t groupSize) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->bulkInsert(tableName, autoIncrementColumnName, columnNames, data, groupSize, insertedIds);
}

void AutoDatabaseConnection::bulkInsert(const String& tableName, const Strings& columnNames,
                                        std::vector<VariantVector>& data, const size_t groupSize) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->bulkInsert(tableName, columnNames, data, groupSize);
}

void AutoDatabaseConnection::bulkDelete(const String& tableName, const String& keyColumnName, const VariantVector& keys) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->bulkDelete(tableName, keyColumnName, keys);
}

[[maybe_unused]] void AutoDatabaseConnection::executeBatchFile(const String& batchFileName, Strings* errors) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->executeBatchFile(batchFileName, errors);
}

void AutoDatabaseConnection::executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    m_connection->executeBatchSQL(batchSQL, errors);
}

String AutoDatabaseConnection::tableSequenceName(const String& tableName, const String& /*sequenceName*/) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    return m_connection->tableSequenceName(tableName);
}

String AutoDatabaseConnection::lastAutoIncrementSql(const String& tableName, const String& /*sequenceName*/) const
{
    if (!m_connection)
    {
        throw Exception(s_invalidConnectionMessage);
    }
    return m_connection->lastAutoIncrementSql(tableName);
}
