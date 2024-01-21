/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#include <sptk5/db/OracleOciGroupInsert.h>

using namespace std;
using namespace sptk;

OracleOciGroupInsert::OracleOciGroupInsert(OracleOciConnection* connection, const String& tableName, const Strings& columnNames, unsigned groupSize)
    : m_insertQuery(connection, makeInsertSQL(tableName, columnNames, groupSize))
    , m_columnNames(columnNames)
    , m_tableName(tableName)
    , m_groupSize(groupSize)
    , m_connection(connection)
{
}

String OracleOciGroupInsert::makeInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize)
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

void OracleOciGroupInsert::insertRows(const vector<VariantVector>& rows)
{
    unsigned fullGroupCount = rows.size() / m_groupSize;
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
        Query insertQuery(m_connection, makeInsertSQL(m_tableName, m_columnNames, remainder));
        insertGroupRows(insertQuery, firstRow, firstRow + remainder);
    }
}

void OracleOciGroupInsert::insertGroupRows(Query& insertQuery, std::vector<VariantVector>::const_iterator startRow, std::vector<VariantVector>::const_iterator end)
{
    size_t parameterIndex = 0;
    size_t columnCount = startRow->size();
    for (auto row = startRow; row != end; ++row)
    {
        for (size_t columnNumber = 0; columnNumber < columnCount; ++columnNumber)
        {
            insertQuery.param(parameterIndex++) = (*row)[columnNumber];
        }
    }
    insertQuery.exec();
}
