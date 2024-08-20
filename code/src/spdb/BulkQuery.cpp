/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "sptk5/db/BulkQuery.h"

using namespace std;
using namespace sptk;

BulkQuery::BulkQuery(PoolDatabaseConnection* connection, const String& tableName, const Strings& columnNames, unsigned groupSize)
    : m_insertQuery(connection, makeInsertSQL(connection->connectionType(), tableName, columnNames, groupSize))
    , m_deleteQuery(connection, makeGenericDeleteSQL(tableName, columnNames[0], groupSize))
    , m_columnNames(columnNames)
    , m_tableName(tableName)
    , m_groupSize(groupSize)
    , m_connection(connection)
{
}

String BulkQuery::makeInsertSQL(DatabaseConnectionType connectionType, const String& tableName, const Strings& columnNames, unsigned groupSize)
{
    String sql;
    switch (connectionType)
    {
        using enum DatabaseConnectionType;
        case ORACLE:
        case ORACLE_OCI:
            sql = BulkQuery::makeOracleInsertSQL(tableName, columnNames, groupSize);
            break;
        case POSTGRES:
        case MYSQL:
        case MSSQL_ODBC:
            sql = BulkQuery::makeGenericInsertSQL(tableName, columnNames, groupSize);
            break;
        case SQLITE3:
            sql = BulkQuery::makeSqlite3InsertSQL(tableName, columnNames, groupSize);
            break;
        default:
            throw Exception("Unsupported database type");
    }
    return sql;
}

String BulkQuery::makeOracleInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize)
{
    stringstream sql;

    sql << "INSERT ALL\n";

    for (size_t rowNumber = 0; rowNumber < groupSize; ++rowNumber)
    {
        sql << "INTO " << tableName << "(" << columnNames.join(",") << ") VALUES(";
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
        sql << ")\n";
    }
    sql << "SELECT * FROM DUAL\n";
    return sql.str();
}

String BulkQuery::makeSqlite3InsertSQL(const String& tableName, const Strings& columnNames, unsigned int groupSize)
{
    stringstream sql;

    sql << "INSERT INTO " << tableName << "\n";
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
        if (keyNumber % 10 == 0)
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

void BulkQuery::insertRows(const vector<VariantVector>& rows)
{
    const auto fullGroupCount = static_cast<unsigned>(rows.size() / m_groupSize);
    const unsigned remainder = rows.size() % m_groupSize;

    auto firstRow = rows.begin();
    if (fullGroupCount > 0)
    {
        for (unsigned groupNumber = 0; groupNumber < fullGroupCount; ++groupNumber)
        {
            insertGroupRows(m_insertQuery, firstRow, firstRow + m_groupSize);
            firstRow += m_groupSize;
        }
    }

    if (remainder > 0)
    {
        // Last group
        const auto databaseConnectionType = m_connection->connectionType();
        Query insertQuery(m_connection, makeInsertSQL(databaseConnectionType, m_tableName, m_columnNames, remainder));
        insertGroupRows(insertQuery, firstRow, firstRow + remainder);
    }
}

void BulkQuery::deleteRows(const VariantVector& keys)
{
    const auto fullGroupCount = static_cast<unsigned>(keys.size() / m_groupSize);
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

void BulkQuery::insertGroupRows(Query& insertQuery, std::vector<VariantVector>::const_iterator startRow, std::vector<VariantVector>::const_iterator end)
{
    size_t parameterIndex = 0;
    const size_t columnCount = startRow->size();
    for (auto row = startRow; row != end; ++row)
    {
        for (size_t columnNumber = 0; columnNumber < columnCount; ++columnNumber)
        {
            insertQuery.param(parameterIndex) = (*row)[columnNumber];
            ++parameterIndex;
        }
    }
    insertQuery.exec();
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
