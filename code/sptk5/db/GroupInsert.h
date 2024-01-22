/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#pragma once

#include <sptk5/db/Query.h>

namespace sptk {

class GroupInsert
{
public:
    GroupInsert(PoolDatabaseConnection* connection, const String& tableName, const Strings& columnNames, unsigned groupSize);

    void insertRows(const std::vector<VariantVector>& rows);

private:
    Query m_insertQuery;
    Strings m_columnNames;
    String m_tableName;
    unsigned m_groupSize;
    PoolDatabaseConnection* m_connection;

    [[nodiscard]] static String makeInsertSQL(DatabaseConnectionType connectionType, const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeOracleInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize);
    static void insertGroupRows(Query& insertQuery, std::vector<VariantVector>::const_iterator startRow, std::vector<VariantVector>::const_iterator end);
};

} // namespace sptk
