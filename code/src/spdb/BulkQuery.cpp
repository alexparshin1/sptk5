/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-06                                             ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "sptk5/db/BulkQuery.h"

#include "sptk5/Printer.h"
#include <sstream> // libc++

using namespace std;
using namespace sptk;

BulkQuery::BulkQuery(const WPoolDatabaseConnection& connection, const String& tableName, const String& serialColumnName, const Strings& columnNames, const size_t groupSize)
    : m_insertQuery(connection, "")
    , m_deleteQuery(connection, "")
    , m_serialColumnName(serialColumnName)
    , m_columnNames(columnNames)
    , m_tableName(tableName)
    , m_groupSize(groupSize)
    , m_connection(connection)
    , m_lastInsertedIdQuery(connection, "")
{
    if (tableName.empty())
    {
        throw Exception("Table name is empty");
    }
    if (m_groupSize == 0)
    {
        throw Exception("Group size must be greater than zero");
    }
    if (m_serialColumnName.empty() && m_columnNames.empty())
    {
        throw Exception("No primary key column specified and column list is empty");
    }
    const auto conn = connection.lock();
    m_insertQuery.sql(makeInsertSQL(conn->connectionType(), tableName, serialColumnName, columnNames, groupSize,
                                   conn->supportsReturning()));
    m_deleteQuery.sql(makeGenericDeleteSQL(tableName, serialColumnName.empty() ? columnNames[0] : serialColumnName, groupSize));
    m_lastInsertedIdQuery.sql(conn->lastAutoIncrementSql(tableName));
}

String BulkQuery::makeInsertSQL(const DatabaseConnectionType connectionType, const String& tableName, const String& keyColumnName, const Strings& columnNames, const size_t groupSize,
                               const bool supportsReturning)
{
    using enum DatabaseConnectionType;
    String sql;
    switch (connectionType)
    {
        case ORACLE:
        case ORACLE_OCI:
            sql = makeOracleInsertSQL(tableName, columnNames, groupSize);
            break;
        case POSTGRES:
        case MYSQL:
            sql = makeGenericInsertSQL(tableName, columnNames, groupSize);
            break;
        case MSSQL_ODBC:
            sql = makeGenericInsertSQL(tableName, columnNames, groupSize, keyColumnName.empty() ? " " : " OUTPUT Inserted." + keyColumnName);
            break;
        case SQLITE3:
            sql = makeSqlite3InsertSQL(tableName, columnNames, groupSize);
            break;
        default:
            throw Exception("Unsupported database type");
    }

    if (!keyColumnName.empty() && (connectionType == POSTGRES || (connectionType == SQLITE3 && supportsReturning)))
    {
        sql += " RETURNING " + keyColumnName;
    }

    return sql;
}

String BulkQuery::makeOracleInsertSQL(const String& tableName, const Strings& columnNames, const size_t groupSize)
{
    stringstream sql;

    sql << "INSERT INTO " << tableName << "(" << columnNames.join(",") << ")\n";

    for (size_t rowNumber = 0; rowNumber < groupSize; ++rowNumber)
    {
        if (rowNumber > 0)
        {
            sql << "UNION ALL ";
        }
        sql << "SELECT ";
        auto first = true;
        for (const auto& column: columnNames)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                sql << ",";
            }
            sql << ":" << column << "_" << rowNumber;
        }
        sql << " FROM DUAL\n";
    }

    return sql.str();
}

String BulkQuery::makeSqlite3InsertSQL(const String& tableName, const Strings& columnNames, const size_t groupSize)
{
    stringstream sql;

    sql << "INSERT INTO " << tableName << "(" << columnNames.join(", ") << ")" << "\n";
    sql << "     SELECT ";

    auto first = true;
    for (const auto& column: columnNames)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            sql << ", ";
        }
        sql << ":" << column << "_0 AS " << column;
    }
    sql << "\n";

    for (size_t rowNumber = 1; rowNumber < groupSize; ++rowNumber)
    {
        sql << "UNION ALL SELECT ";

        first = true;
        for (const auto& column: columnNames)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                sql << ",";
            }
            sql << ":" << column << "_" << rowNumber;
        }

        sql << "\n";
    }

    return sql.str();
}

String BulkQuery::makeGenericInsertSQL(const String& tableName, const Strings& columnNames, const size_t groupSize, const String& intoAttribute)
{
    stringstream sql;

    sql << "INSERT INTO " << tableName << "(" << columnNames.join(",") << ")\n";
    if (!intoAttribute.empty())
    {
        sql << "            " << intoAttribute << "\n";
    }
    sql << "VALUES\n";

    for (size_t rowNumber = 0; rowNumber < groupSize; ++rowNumber)
    {
        if (rowNumber > 0)
        {
            sql << ",\n";
        }

        sql << "  (";

        auto first = true;
        for (const auto& column: columnNames)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                sql << ",";
            }
            sql << ":" << column << "_" << rowNumber;
        }

        sql << ")";
    }
    return sql.str();
}

String BulkQuery::makeGenericDeleteSQL(const String& tableName, const String& keyColumnName, const size_t groupSize)
{
    stringstream sql;

    sql << "DELETE FROM " << tableName << " WHERE " << keyColumnName << " IN (";

    for (size_t keyNumber = 0; keyNumber < groupSize; ++keyNumber)
    {
        if (constexpr auto keysPerRow = 10;
            keyNumber % keysPerRow == 0)
        {
            sql << "\n  ";
        }

        if (keyNumber > 0)
        {
            sql << ", ";
        }

        sql << ":" << keyColumnName << "_" << keyNumber;
    }

    sql << "\n)\n";

    return sql.str();
}

void BulkQuery::beginInsert(bool& startedTransaction) const
{
    using enum DatabaseConnectionType;

    startedTransaction = false;
    const auto conn = m_connection.lock();
    if (conn->connectionType() == MYSQL)
    {
        // Locked the table until the UNLOCK TABLES command.
        // This method is used to prevent other connections from inserting data at the same time.
        Query lockTableQuery(m_connection, "LOCK TABLES " + m_tableName + " WRITE", false);
        lockTableQuery.exec();
    }
}

void BulkQuery::unlockTables() const
{
    using enum DatabaseConnectionType;
    const auto conn = m_connection.lock();
    if (conn->connectionType() == MYSQL)
    {
        Query unlockTableQuery(m_connection, "UNLOCK TABLES", false);
        unlockTableQuery.exec();
    }
}

bool BulkQuery::reserveInsertIds(const String& tableName, const vector<VariantVector>& rows, vector<int64_t>& insertedIds) const
{
    using enum DatabaseConnectionType;

    const auto conn = m_connection.lock();
    if (const auto connectionType = conn->connectionType();
        connectionType == ORACLE || connectionType == ORACLE_OCI)
    {
        stringstream sqlStream;
        sqlStream << "WITH SERIES (IND) AS (SELECT ROWNUM FROM DUAL CONNECT BY ROWNUM <= " << rows.size() << ")\n"
                  << "SELECT " << conn->tableSequenceName(tableName) << ".nextval FROM SERIES";

        Query query(m_connection, sqlStream.str());
        query.open();
        while (!query.eof())
        {
            insertedIds.push_back(query[0].asInt64());
            query.next();
        }
        query.close();

        return true;
    }
    return false;
}

void BulkQuery::insertRows(const vector<VariantVector>& rows, vector<int64_t>* insertedIds)
{
    using enum DatabaseConnectionType;

    if (m_groupSize == 0)
    {
        throw Exception("Group size must be greater than zero");
    }

    const auto fullGroupCount = rows.size() / m_groupSize;
    const auto remainder = rows.size() % m_groupSize;
    if (insertedIds)
    {
        insertedIds->clear();
        insertedIds->reserve(rows.size());
    }

    auto useReservedIds = false;

    auto   serialColumnIndex = -1;
    size_t reservedIdOffset = 0;
    if (!m_serialColumnName.empty() && insertedIds)
    {
        // For Oracle, reserve auto increment IDs, and set it into insertedIds.
        if ((useReservedIds = reserveInsertIds(m_tableName, rows, *insertedIds)))
        {
            serialColumnIndex = m_columnNames.indexOf(m_serialColumnName);
            if (serialColumnIndex < 0)
            {
                m_columnNames.push_back(m_serialColumnName);
            }
        }
    }

    auto firstRow = rows.begin();

    if (fullGroupCount > 0)
    {
        for (unsigned groupNumber = 0; groupNumber < fullGroupCount; ++groupNumber)
        {
            const span group(firstRow, m_groupSize);
            const auto insertedCount = insertGroupRows(m_insertQuery, group, insertedIds, useReservedIds, serialColumnIndex, reservedIdOffset);
            firstRow += static_cast<long>(insertedCount);
        }
    }

    if (remainder > 0)
    {
        const auto conn = m_connection.lock();
        // Last group
        const span group(firstRow, remainder);
        const auto databaseConnectionType = conn->connectionType();
        Query      insertQuery(m_connection, makeInsertSQL(databaseConnectionType, m_tableName, m_serialColumnName, m_columnNames, remainder,
                                                          conn->supportsReturning()));
        insertGroupRows(insertQuery, group, insertedIds, useReservedIds, serialColumnIndex, reservedIdOffset);
    }
}

void BulkQuery::deleteRows(const VariantVector& keys)
{
    const auto fullGroupCount = static_cast<unsigned>(keys.size() / m_groupSize);
    const auto remainder = keys.size() % m_groupSize;

    auto firstKey = keys.begin();
    if (fullGroupCount > 0)
    {
        for (unsigned groupNumber = 0; groupNumber < fullGroupCount; ++groupNumber)
        {
            const span group(firstKey, m_groupSize);
            deleteGroupRows(m_deleteQuery, group);
            firstKey += static_cast<long>(m_groupSize);
        }
    }

    if (remainder > 0)
    {
        if (m_serialColumnName.empty() && m_columnNames.empty())
        {
            throw Exception("No primary key column specified and column list is empty");
        }

        // Last group
        Query      deleteQuery(m_connection, makeGenericDeleteSQL(m_tableName, m_serialColumnName.empty() ? m_columnNames[0] : m_serialColumnName, remainder));
        const span group(firstKey, remainder);
        deleteGroupRows(deleteQuery, group);
    }
}

void BulkQuery::appendParameterValuesFromRow(const vector<int64_t>* insertedIds, const size_t serialColumnIndex,
                                             size_t& reservedIdOffset, const vector<Variant>::size_type columnCount,
                                             QueryParameterList::iterator& parameterIterator, const vector<Variant>& row)
{
    for (size_t columnNumber = 0; columnNumber < columnCount; ++columnNumber)
    {
        const auto& parameter = *parameterIterator;
        if (columnNumber == serialColumnIndex)
        {
            if (insertedIds)
            {
                *parameter = (*insertedIds)[reservedIdOffset];
            }
            ++reservedIdOffset;
        }
        else
        {
            *parameter = row[columnNumber];
        }
        ++parameterIterator;
    }
}

void BulkQuery::appendParameterValuesFromRows(Query& insertQuery, const span<const VariantVector> rows, vector<int64_t>* insertedIds, const size_t serialColumnIndex, size_t& reservedIdOffset, int64_t& rowCount, const vector<Variant>::size_type rowSize, const vector<Variant>::size_type columnCount)
{
    auto parameterIterator = insertQuery.parameters().begin();
    for (const auto& row: rows)
    {
        if (row.size() != rowSize)
        {
            throw Exception("Row size mismatch");
        }
        appendParameterValuesFromRow(insertedIds, serialColumnIndex, reservedIdOffset, columnCount, parameterIterator, row);
        ++rowCount;
    }
}

size_t BulkQuery::insertGroupRows(Query& insertQuery, const span<const VariantVector> rows,
                                  vector<int64_t>* insertedIds, const bool useReservedIds, const size_t serialColumnIndex, size_t& reservedIdOffset)
{
    using enum DatabaseConnectionType;

    const auto conn = m_connection.lock();
    const auto connectionType = conn->connectionType();
    const auto captureInsertedIds = (!m_serialColumnName.empty() && insertedIds != nullptr) || connectionType == MSSQL_ODBC;
    // SQLite only returns them when the library has RETURNING, which arrived in 3.35 and is absent
    // from the 3.34.1 that Enterprise Linux 9 ships. Without it the branch below asks the connection
    // afterwards, exactly as it already does for MySQL - and that is the safer of the two under
    // concurrency, because the rowids are allocated inside the write, under its lock, where another
    // connection's insert cannot interleave.
    const auto insertReturnsIds = connectionType == POSTGRES || connectionType == MSSQL_ODBC ||
                                  (connectionType == SQLITE3 && conn->supportsReturning());
    const auto sequenceReturnedIds = useReservedIds && (connectionType == ORACLE || connectionType == ORACLE_OCI);

    int64_t    rowCount = 0;
    const auto rowSize = rows.front().size();
    auto       columnCount = rowSize;
    if (columnCount == serialColumnIndex)
    {
        ++columnCount;
    }

    appendParameterValuesFromRows(insertQuery, rows, insertedIds, serialColumnIndex, reservedIdOffset, rowCount, rowSize, columnCount);

    if (captureInsertedIds && !sequenceReturnedIds)
    {
        if (insertReturnsIds)
        {
            insertQuery.open();
            while (!insertQuery.eof())
            {
                if (insertedIds != nullptr)
                {
                    insertedIds->push_back(insertQuery[0].asInt64());
                }
                insertQuery.next();
            }
            insertQuery.close();
        }
        else
        {
            auto startedTransaction = false;

            beginInsert(startedTransaction);

            try
            {
                insertQuery.exec();

                auto lastInsertedId = m_lastInsertedIdQuery.scalar().asInt64();

                unlockTables();

                auto firstInsertedId = lastInsertedId - rowCount + 1;

                if (conn->connectionType() == MYSQL)
                {
                    // A special case for MySQL: multi-row insert returns the first row id
                    firstInsertedId = lastInsertedId;
                    lastInsertedId += rowCount - 1;
                }

                if (insertedIds != nullptr)
                {
                    for (auto insertedId = firstInsertedId; insertedId <= lastInsertedId; ++insertedId)
                    {
                        insertedIds->push_back(insertedId);
                    }
                }
            }
            catch (const Exception&)
            {
                unlockTables();
                if (startedTransaction)
                {
                    conn->rollbackTransaction();
                }
                throw;
            }
        }
    }
    else
    {
        insertQuery.exec();
    }

    return rowCount;
}

void BulkQuery::deleteGroupRows(Query& deleteQuery, const span<const Variant> keys)
{
    size_t parameterIndex = 0;
    for (const auto& key: keys)
    {
        deleteQuery.param(parameterIndex) = key;
        ++parameterIndex;
    }
    deleteQuery.exec();
}
