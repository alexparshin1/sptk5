/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include "sptk5/db/GroupInsert.h"

using namespace std;
using namespace sptk;

GroupInsert::GroupInsert(PoolDatabaseConnection* connection, const String& tableName, const Strings& columnNames, unsigned groupSize)
    : m_insertQuery(connection, makeInsertSQL(connection->connectionType(), tableName, columnNames, groupSize))
    , m_columnNames(columnNames)
    , m_tableName(tableName)
    , m_groupSize(groupSize)
    , m_connection(connection)
{
}

String GroupInsert::makeInsertSQL(DatabaseConnectionType connectionType, const String& tableName, const Strings& columnNames, unsigned groupSize)
{
    String sql;
    switch (connectionType)
    {
        using enum DatabaseConnectionType;
        case ORACLE:
        case ORACLE_OCI:
            sql = GroupInsert::makeOracleInsertSQL(tableName, columnNames, groupSize);
            break;
        case POSTGRES:
        case MYSQL:
        case MSSQL_ODBC:
            sql = GroupInsert::makeGenericInsertSQL(tableName, columnNames, groupSize);
            break;
        case SQLITE3:
            sql = GroupInsert::makeSqlite3InsertSQL(tableName, columnNames, groupSize);
            break;
        default:
            throw Exception("Unsupported database type");
    }
    return sql;
}

String GroupInsert::makeOracleInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize)
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

String GroupInsert::makeSqlite3InsertSQL(const String& tableName, const Strings& columnNames, unsigned int groupSize)
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

String GroupInsert::makeGenericInsertSQL(const String& tableName, const Strings& columnNames, unsigned int groupSize)
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

void GroupInsert::insertRows(const vector<VariantVector>& rows)
{
    auto fullGroupCount = static_cast<unsigned>(rows.size() / m_groupSize);
    unsigned remainder = rows.size() % m_groupSize;

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
        auto databaseConnectionType = m_connection->connectionType();
        Query insertQuery(m_connection, makeInsertSQL(databaseConnectionType, m_tableName, m_columnNames, remainder));
        insertGroupRows(insertQuery, firstRow, firstRow + remainder);
    }
}

void GroupInsert::insertGroupRows(Query& insertQuery, std::vector<VariantVector>::const_iterator startRow, std::vector<VariantVector>::const_iterator end)
{
    size_t parameterIndex = 0;
    size_t columnCount = startRow->size();
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
