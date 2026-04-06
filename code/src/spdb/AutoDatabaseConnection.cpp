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
    if (m_connection.expired())
    {
        throw Exception("Failed to create database connection");
    }
}

AutoDatabaseConnection::~AutoDatabaseConnection()
{
    try
    {
        const auto connection = m_connection.lock();
        if (connection)
        {
            if (connection->active())
            {
                connection->close();
            }
            m_connectionPool.releaseConnection(connection);
        }
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

WPoolDatabaseConnection AutoDatabaseConnection::connection() const
{
    return acquireConnection();
}

void AutoDatabaseConnection::open(const String& connectionString) const
{
    const auto connection = acquireConnection();
    connection->open(connectionString);
}

void AutoDatabaseConnection::close() const
{
    const auto connection = acquireConnection();
    if (connection->active())
    {
        connection->close();
    }
}

bool AutoDatabaseConnection::active() const
{
    const auto connection = m_connection.lock();
    return connection != nullptr && connection->active();
}

const DatabaseConnectionString& AutoDatabaseConnection::connectionString() const
{
    return m_connectionPool;
}

DatabaseConnectionType AutoDatabaseConnection::connectionType() const
{
    const auto connection = acquireConnection();
    return connection->connectionType();
}

String AutoDatabaseConnection::driverDescription() const
{
    const auto connection = acquireConnection();
    return connection->driverDescription();
}

void AutoDatabaseConnection::beginTransaction() const
{
    const auto connection = acquireConnection();
    connection->beginTransaction();
}

void AutoDatabaseConnection::commitTransaction() const
{
    const auto connection = acquireConnection();
    connection->commitTransaction();
}

void AutoDatabaseConnection::rollbackTransaction() const
{
    const auto connection = acquireConnection();
    connection->rollbackTransaction();
}

void AutoDatabaseConnection::objectList(const DatabaseObjectType objectType, Strings& objects) const
{
    const auto connection = acquireConnection();
    connection->objectList(objectType, objects);
}

void AutoDatabaseConnection::bulkInsert(const String& tableName, const String& autoIncrementColumnName, const Strings& columnNames,
                                        std::vector<VariantVector>& data, std::vector<int64_t>& insertedIds, const size_t groupSize) const
{
    const auto connection = acquireConnection();
    connection->bulkInsert(tableName, autoIncrementColumnName, columnNames, data, groupSize, insertedIds);
}

void AutoDatabaseConnection::bulkInsert(const String& tableName, const Strings& columnNames,
                                        std::vector<VariantVector>& data, const size_t groupSize) const
{
    const auto connection = acquireConnection();
    connection->bulkInsert(tableName, columnNames, data, groupSize);
}

void AutoDatabaseConnection::bulkDelete(const String& tableName, const String& keyColumnName, const VariantVector& keys) const
{
    const auto connection = acquireConnection();
    connection->bulkDelete(tableName, keyColumnName, keys);
}

[[maybe_unused]] void AutoDatabaseConnection::executeBatchFile(const String& batchFileName, Strings* errors) const
{
    const auto connection = acquireConnection();
    connection->executeBatchFile(batchFileName, errors);
}

void AutoDatabaseConnection::executeBatchSQL(const sptk::Strings& batchSQL, Strings* errors) const
{
    const auto connection = acquireConnection();
    connection->executeBatchSQL(batchSQL, errors);
}

String AutoDatabaseConnection::tableSequenceName(const String& tableName, const String& /*sequenceName*/) const
{
    const auto connection = acquireConnection();
    return connection->tableSequenceName(tableName);
}

String AutoDatabaseConnection::lastAutoIncrementSql(const String& tableName, const String& /*sequenceName*/) const
{
    const auto connection = acquireConnection();
    return connection->lastAutoIncrementSql(tableName);
}
