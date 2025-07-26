/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "sptk5/db/BulkQuery.h"

#include "sptk5/Printer.h"

using namespace std;
using namespace sptk;

BulkQuery::BulkQuery(PoolDatabaseConnection* connection, const String& tableName, const String& serialColumnName, const Strings& columnNames, unsigned groupSize)
    : m_insertQuery(connection, makeInsertSQL(connection->connectionType(), tableName, serialColumnName, columnNames, groupSize))
    , m_deleteQuery(connection, makeGenericDeleteSQL(tableName, columnNames[0], groupSize))
    , m_serialColumnName(serialColumnName)
    , m_columnNames(columnNames)
    , m_tableName(tableName)
    , m_groupSize(groupSize)
    , m_connection(connection)
    , m_lastInsertedIdQuery(connection, connection->lastAutoIncrementSql(tableName))
{
}

String BulkQuery::makeInsertSQL(DatabaseConnectionType connectionType, const String& tableName, const String& keyColumnName, const Strings& columnNames, unsigned groupSize)
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
        case MSSQL_ODBC:
            sql = makeGenericInsertSQL(tableName, columnNames, groupSize);
            break;
        case SQLITE3:
            sql = makeSqlite3InsertSQL(tableName, columnNames, groupSize);
            break;
        default:
            throw Exception("Unsupported database type");
    }

    if (!keyColumnName.empty() && (connectionType == POSTGRES || connectionType == SQLITE3))
    {
        sql += " RETURNING " + keyColumnName;
    }

    return sql;
}

String BulkQuery::makeOracleInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize)
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
        bool first = true;
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

String BulkQuery::makeSqlite3InsertSQL(const String& tableName, const Strings& columnNames, unsigned int groupSize)
{
    stringstream sql;

    sql << "INSERT INTO " << tableName << "(" << columnNames.join(", ") << ")" << "\n";
    sql << "     SELECT ";

    bool first = true;
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

String BulkQuery::makeGenericInsertSQL(const String& tableName, const Strings& columnNames, unsigned int groupSize)
{
    stringstream sql;

    sql << "INSERT INTO " << tableName << "(" << columnNames.join(",") << ")\n";
    sql << "VALUES\n";

    for (size_t rowNumber = 0; rowNumber < groupSize; ++rowNumber)
    {
        if (rowNumber > 0)
        {
            sql << ",\n";
        }

        sql << "  (";

        bool first = true;
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

String BulkQuery::makeGenericDeleteSQL(const String& tableName, const String& keyColumnName, unsigned int groupSize)
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
    switch (m_connection->connectionType())
    {
        case MYSQL: {
            // Table is locked until the UNLOCK TABLES command.
            Query lockTableQuery(m_connection, "LOCK TABLES " + m_tableName + " WRITE", false);
            lockTableQuery.exec();
        }
        break;

        default:
            break;
    }
}

void BulkQuery::commitInsert() const
{
    using enum DatabaseConnectionType;
    switch (m_connection->connectionType())
    {
        case MYSQL: {
            Query unlockTableQuery(m_connection, "UNLOCK TABLES", false);
            unlockTableQuery.exec();
        }
        break;

        default:
            break;
    }
}

bool BulkQuery::reserveInsertIds(const String& tableName, const vector<VariantVector>& rows, vector<int64_t>& insertedIds)
{
    using enum DatabaseConnectionType;

    string       sequenceName = m_connection->tableSequenceName(tableName);
    stringstream sqlStream;

    switch (m_connection->connectionType())
    {
        case ORACLE:
        case ORACLE_OCI:
            sqlStream << "WITH SERIES (IND) AS (SELECT ROWNUM FROM DUAL CONNECT BY ROWNUM <= " << rows.size() << ")"
                      << "SELECT " << m_connection->tableSequenceName(tableName) << ".nextval FROM SERIES";
            break;
        default:
            return false;
    }

    const auto sql = sqlStream.str();
    if (!sql.empty())
    {
        Query query(m_connection, sql);
        query.open();
        while (!query.eof())
        {
            insertedIds.push_back(query[0].asInt64());
            query.next();
        }
        query.close();
    }

    return true;
}

vector<int64_t> BulkQuery::insertRows(const vector<VariantVector>& rows)
{
    using enum DatabaseConnectionType;

    const auto     fullGroupCount = static_cast<unsigned>(rows.size() / m_groupSize);
    const unsigned remainder = rows.size() % m_groupSize;

    vector<int64_t> insertedIds;
    insertedIds.reserve(rows.size());

    bool useReservedIds = false;

    int    serialColumnIndex = -1;
    size_t reservedIdOffset = 0;
    if (!m_serialColumnName.empty())
    {
        // For several DB types, reserve auto increment IDs, and set it in rows
        // and insertedIds.
        useReservedIds = reserveInsertIds(m_tableName, rows, insertedIds);
        serialColumnIndex = m_columnNames.indexOf(m_serialColumnName);
        if (serialColumnIndex < 0)
        {
            m_columnNames.push_back(m_serialColumnName);
        }
    }

    auto firstRow = rows.begin();

    if (fullGroupCount > 0)
    {
        for (unsigned groupNumber = 0; groupNumber < fullGroupCount; ++groupNumber)
        {
            auto insertedCount = insertGroupRows(m_insertQuery, firstRow, firstRow + m_groupSize, insertedIds, useReservedIds, serialColumnIndex, reservedIdOffset);
            firstRow += insertedCount;
        }
    }

    if (remainder > 0)
    {
        // Last group
        const auto databaseConnectionType = m_connection->connectionType();
        Query      insertQuery(m_connection, makeInsertSQL(databaseConnectionType, m_tableName, m_serialColumnName, m_columnNames, remainder));
        insertGroupRows(insertQuery, firstRow, firstRow + remainder, insertedIds, useReservedIds, serialColumnIndex, reservedIdOffset);
    }

    return insertedIds;
}

void BulkQuery::deleteRows(const VariantVector& keys)
{
    const auto     fullGroupCount = static_cast<unsigned>(keys.size() / m_groupSize);
    const unsigned remainder = keys.size() % m_groupSize;

    auto firstKey = keys.begin();
    if (fullGroupCount > 0)
    {
        for (unsigned groupNumber = 0; groupNumber < fullGroupCount; ++groupNumber)
        {
            deleteGroupRows(m_deleteQuery, firstKey, firstKey + m_groupSize);
            firstKey += m_groupSize;
        }
    }

    if (remainder > 0)
    {
        // Last group
        Query deleteQuery(m_connection, makeGenericDeleteSQL(m_tableName, m_columnNames[0], remainder));
        deleteGroupRows(deleteQuery, firstKey, firstKey + remainder);
    }
}

size_t BulkQuery::insertGroupRows(Query& insertQuery, vector<VariantVector>::const_iterator startRow, vector<VariantVector>::const_iterator end,
                                  vector<int64_t>& insertedIds, bool useReservedIds, size_t serialColumnIndex, size_t& reservedIdOffset)
{
    using enum DatabaseConnectionType;

    auto connectionType = m_connection->connectionType();
    bool captureInsertedIds = !m_serialColumnName.empty();
    bool insertReturnsIds = connectionType == POSTGRES || connectionType == SQLITE3;
    bool sequenceReturnedIds = useReservedIds && (connectionType == ORACLE || connectionType == ORACLE_OCI);

    size_t rowCount = 0;
    size_t parameterIndex = 0;
    size_t columnCount = startRow->size();
    if (columnCount == serialColumnIndex)
    {
        ++columnCount;
    }

    for (auto row = startRow; row != end; ++row)
    {
        for (size_t columnNumber = 0; columnNumber < columnCount; ++columnNumber)
        {
            if (columnNumber == serialColumnIndex)
            {
                insertQuery.param(parameterIndex) = insertedIds[reservedIdOffset];
                ++reservedIdOffset;
            }
            else
            {
                insertQuery.param(parameterIndex) = (*row)[columnNumber];
            }
            ++parameterIndex;
        }
        ++rowCount;
    }

    if (captureInsertedIds && insertReturnsIds)
    {
        if (insertReturnsIds)
        {
            insertQuery.open();
            while (!insertQuery.eof())
            {
                insertedIds.push_back(insertQuery[0].asInt64());
                ++parameterIndex;
                insertQuery.next();
            }
            insertQuery.close();
        }
    }
    else
    {
        bool startedTransaction = false;
        try
        {
            if (captureInsertedIds && !sequenceReturnedIds)
            {
                beginInsert(startedTransaction);

                insertQuery.exec();

                auto lastInsertedId = m_lastInsertedIdQuery.scalar().asInt64();

                commitInsert();

                auto firstInsertedId = lastInsertedId - rowCount + 1;

                if (m_connection->connectionType() == DatabaseConnectionType::MYSQL)
                {
                    // A special case for MySQL: multi-row insert returns the first row id
                    firstInsertedId = lastInsertedId;
                    lastInsertedId += rowCount - 1;
                }

                for (int64_t insertedId = firstInsertedId; insertedId <= lastInsertedId; ++insertedId)
                {
                    insertedIds.push_back(insertedId);
                }
            }
            else
            {
                insertQuery.exec();
            }
        }
        catch (const Exception& e)
        {
            if (startedTransaction)
            {
                m_connection->rollbackTransaction();
            }
            CERR("BulkQuery::insertGroupRows : " << hex << " " << this << e.what());
        }
    }

    return rowCount;
}

void BulkQuery::deleteGroupRows(Query& deleteQuery, VariantVector::const_iterator startKey, VariantVector::const_iterator end)
{
    size_t parameterIndex = 0;
    for (auto key = startKey; key != end; ++key)
    {
        deleteQuery.param(parameterIndex) = *key;
        ++parameterIndex;
    }
    deleteQuery.exec();
}
