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

BulkQuery::BulkQuery(PoolDatabaseConnection* connection, const String& tableName, const String& keyColumnName, const Strings& columnNames, unsigned groupSize)
    : m_insertQuery(connection, makeInsertSQL(connection->connectionType(), tableName, keyColumnName, columnNames, groupSize))
    , m_deleteQuery(connection, makeGenericDeleteSQL(tableName, columnNames[0], groupSize))
    , m_keyColumnName(keyColumnName)
    , m_columnNames(columnNames)
    , m_tableName(tableName)
    , m_groupSize(groupSize)
    , m_connection(connection)
    , m_lastInsertedIdQuery(connection)
{
    String sequenceName;
    if (connection->connectionType() == DatabaseConnectionType::ORACLE || connection->connectionType() == DatabaseConnectionType::ORACLE_OCI)
    {
        stringstream getSequenceSql;
        getSequenceSql << "SELECT DATA_DEFAULT AS sequence_name\n"
                       << "  FROM ALL_TAB_COLUMNS\n"
                       << " WHERE DATA_DEFAULT is not null\n"
                       << "   AND owner = (SELECT sys_context('USERENV', 'CURRENT_USER') FROM dual) "
                       << "   AND TABLE_NAME='" << tableName.toUpperCase() << "'\n";
        Query getSequenceName(connection, getSequenceSql.str());
        sequenceName = getSequenceName.scalar().asString().toUpperCase().replace("\\.NEXTVAL", "");
    }
    m_lastInsertedIdQuery.sql(connection->lastAutoIncrementSql(tableName, sequenceName));
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

vector<uint64_t> BulkQuery::insertRows(const vector<VariantVector>& rows)
{
    const auto     fullGroupCount = static_cast<unsigned>(rows.size() / m_groupSize);
    const unsigned remainder = rows.size() % m_groupSize;

    vector<uint64_t> insertedIds;

    if (!m_keyColumnName.empty())
    {
        insertedIds.reserve(rows.size());
    }

    auto firstRow = rows.begin();
    if (fullGroupCount > 0)
    {
        for (unsigned groupNumber = 0; groupNumber < fullGroupCount; ++groupNumber)
        {
            insertGroupRows(m_insertQuery, firstRow, firstRow + m_groupSize, insertedIds);
            firstRow += m_groupSize;
        }
    }

    if (remainder > 0)
    {
        // Last group
        const auto databaseConnectionType = m_connection->connectionType();
        Query      insertQuery(m_connection, makeInsertSQL(databaseConnectionType, m_tableName, m_keyColumnName, m_columnNames, remainder));
        insertGroupRows(insertQuery, firstRow, firstRow + remainder, insertedIds);
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

void BulkQuery::insertGroupRows(Query& insertQuery, vector<VariantVector>::const_iterator startRow, vector<VariantVector>::const_iterator end, vector<uint64_t>& insertedIds)
{
    using enum DatabaseConnectionType;

    size_t       rowCount = 0;
    size_t       parameterIndex = 0;
    const size_t columnCount = startRow->size();
    for (auto row = startRow; row != end; ++row)
    {
        for (size_t columnNumber = 0; columnNumber < columnCount; ++columnNumber)
        {
            insertQuery.param(parameterIndex) = (*row)[columnNumber];
            ++parameterIndex;
        }
        ++rowCount;
    }

    auto connectionType = m_connection->connectionType();
    if (!m_keyColumnName.empty() && (connectionType == POSTGRES || connectionType == SQLITE3))
    {
        insertQuery.open();
        while (!insertQuery.eof())
        {
            insertedIds.push_back(static_cast<uint64_t>(insertQuery[0].asInt64()));
            ++parameterIndex;
            insertQuery.next();
        }
        insertQuery.close();
    }
    else
    {
        insertQuery.exec();
        auto lastInsertedId = m_lastInsertedIdQuery.scalar().asInt64();

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
