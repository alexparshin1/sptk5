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

class BulkQuery
{
public:
    BulkQuery(PoolDatabaseConnection* connection, const String& tableName, const Strings& columnNames, unsigned groupSize);

    void insertRows(const std::vector<VariantVector>& rows);
    void deleteRows(const VariantVector& keys);

private:
    Query m_insertQuery;
    Query m_deleteQuery;
    Strings m_columnNames;
    String m_tableName;
    unsigned m_groupSize;
    PoolDatabaseConnection* m_connection;

    [[nodiscard]] static String makeInsertSQL(DatabaseConnectionType connectionType, const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeOracleInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeGenericInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeSqlite3InsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeGenericDeleteSQL(const String& tableName, const String& keyColumnName, unsigned int groupSize);
    static void insertGroupRows(Query& insertQuery, std::vector<VariantVector>::const_iterator startRow, std::vector<VariantVector>::const_iterator end);
    static void deleteGroupRows(Query& insertQuery, VariantVector::const_iterator startKey, VariantVector::const_iterator end);
};

} // namespace sptk
