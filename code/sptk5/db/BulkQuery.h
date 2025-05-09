/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#pragma once

#include <sptk5/db/Query.h>

namespace sptk {
/**
 * @addtogroup database Database Classes
 * @{
 */

/**
 * @brief Bulk query class
 *
 * Creates bulk insert and bulk delete queries for the specified table.
 */
class SP_EXPORT BulkQuery
{
public:
    /**
     * @brief Constructor
     * @param connection        Database connection
     * @param tableName         Table name
     * @param columnNames       Column names
     * @param groupSize         Group size
     */
    BulkQuery(PoolDatabaseConnection* connection, const String& tableName, const Strings& columnNames, unsigned groupSize);

    /**
     * Insert rows into the table
     * @param rows              Data to insert
     */
    void insertRows(const std::vector<VariantVector>& rows);

    /**
     * Delete rows from the table
     * @param keys              Keys to delete
     */
    void deleteRows(const VariantVector& keys);

private:
    Query                   m_insertQuery; ///< Insert query
    Query                   m_deleteQuery; ///< Delete query
    Strings                 m_columnNames; ///< Column names
    String                  m_tableName;   ///< Table name
    unsigned                m_groupSize;   ///< Insert or delete record group size
    PoolDatabaseConnection* m_connection;  ///< Database connection

    [[nodiscard]] static String makeInsertSQL(DatabaseConnectionType connectionType, const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeOracleInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeGenericInsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeSqlite3InsertSQL(const String& tableName, const Strings& columnNames, unsigned groupSize);
    [[nodiscard]] static String makeGenericDeleteSQL(const String& tableName, const String& keyColumnName, unsigned int groupSize);
    static void insertGroupRows(Query& insertQuery, std::vector<VariantVector>::const_iterator startRow, std::vector<VariantVector>::const_iterator end);
    static void deleteGroupRows(Query& insertQuery, VariantVector::const_iterator startKey, VariantVector::const_iterator end);
};

} // namespace sptk
