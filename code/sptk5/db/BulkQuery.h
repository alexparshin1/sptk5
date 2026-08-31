/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#pragma once

#include <sptk5/db/Query.h>
#include <span>

namespace sptk {
/**
 * @addtogroup database Database Classes.
 * @{.
 */

/**
 * @brief Bulk query class.
 *
 * Creates bulk insert and bulk delete queries for the specified table.
 */
class SP_EXPORT BulkQuery
{
public:
    /**
     * @brief Constructor.
     * @param connection        Database connection.
     * @param tableName         Table name.
     * @param serialColumnName  Autoincrement column name.
     * @param columnNames       Column names.
     * @param groupSize         Group size.
     */
    BulkQuery(const WPoolDatabaseConnection& connection, const String& tableName, const String& serialColumnName, const Strings& columnNames, size_t groupSize);

    /**
     * @brief Insert rows into the table.
     * @param rows          Data to insert.
     * @param insertedIds   The optional inserted values for the autoincrement column (if it isn't empty).
     */
    void insertRows(const std::vector<std::vector<Variant>>& rows, std::vector<int64_t>* insertedIds = nullptr);

    /**
     * @brief Delete rows from the table.
     * @param keys              Keys to delete.
     */
    void deleteRows(const VariantVector& keys);

    /**
     *
     * @param insertedIds       Vector for inserted IDs.
     * @param serialColumnIndex The index of the serial column in the row.
     * @param reservedIdOffset  The offset for reserved IDs.
     * @param columnCount       The total number of columns in the row.
     * @param parameterIterator Iterator for query parameters.
     * @param row               The row data.
     */
    static void appendParameterValuesFromRow(const std::vector<int64_t>* insertedIds, size_t serialColumnIndex, size_t& reservedIdOffset, std::size_t columnCount, QueryParameterList::iterator& parameterIterator, const std::vector<Variant>& row);
    static void appendParameterValuesFromRows(Query& insertQuery, std::span<const std::vector<Variant>> rows, std::vector<int64_t>* insertedIds, size_t serialColumnIndex, size_t& reservedIdOffset, int64_t& rowCount, std::size_t rowSize, std::size_t columnCount);

private:
    Query                   m_insertQuery;         ///< Insert query.
    Query                   m_deleteQuery;         ///< Delete query.
    String                  m_serialColumnName;    ///< Auto increment column name (optional, can be empty).
    Strings                 m_columnNames;         ///< Column names.
    String                  m_tableName;           ///< Table name.
    size_t                  m_groupSize;           ///< Insert or delete record group size.
    WPoolDatabaseConnection m_connection;          ///< Database connection.
    Query                   m_lastInsertedIdQuery; ///< Query that retrieves the last inserted id.

    [[nodiscard]] static String makeInsertSQL(DatabaseConnectionType connectionType, const String& tableName, const String& keyColumnName, const Strings& columnNames, size_t groupSize,
                                             bool supportsReturning);
    [[nodiscard]] static String makeOracleInsertSQL(const String& tableName, const Strings& columnNames, size_t groupSize);
    [[nodiscard]] static String makeGenericInsertSQL(const String& tableName, const Strings& columnNames, size_t groupSize, const String& intoAttribute = "");
    [[nodiscard]] static String makeSqlite3InsertSQL(const String& tableName, const Strings& columnNames, size_t groupSize);
    [[nodiscard]] static String makeGenericDeleteSQL(const String& tableName, const String& keyColumnName, size_t groupSize);

    void        beginInsert(bool& startedTransaction) const;
    void        unlockTables() const;
    bool        reserveInsertIds(const String& tableName, const std::vector<std::vector<Variant>>& rows, std::vector<int64_t>& insertedIds) const;
    size_t      insertGroupRows(Query& insertQuery, std::span<const std::vector<Variant>> rows,
                                std::vector<int64_t>* insertedIds, bool useReservedIds, size_t serialColumnIndex, size_t& reservedIdOffset);
    static void deleteGroupRows(Query& insertQuery, std::span<const Variant> keys);
};

} // namespace sptk
