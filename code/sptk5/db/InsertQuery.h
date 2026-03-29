/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/db/Query.h>

namespace sptk {
/**
 * @addtogroup Database Database Support.
 * @{
 */

/**
 * @brief Database query.
 *
 * A Dataset connected to the database to execute database queries. The type of the database depends
 * on the DatabaseConnection object the query is connected to.
 */
class SP_EXPORT InsertQuery : public Query
{
public:
    /**
     * @brief Constructor.
     *
     * You can optionally provide the name of the file and line number where
     * this query is created. This is used to collect statistical information
     * for the query calls. If file and line information is provided, then
     * the call statistics are stored to the database object during the query dtor.
     * @param db               Database connection.
     * @param sql              SQL query, optional.
     * @param idFieldName      Name of auto-incremental field.
     * @param autoPrepare      Whether to automatically prepare the query.
     */
    explicit InsertQuery(const DatabaseConnection& db, const String& sql = "", const String& idFieldName = "id", bool autoPrepare = true);

    /**
     * @brief Return query' SQL.
     * @return SQL.
     */
    [[nodiscard]] String sql() const override;

    /**
     * @brief Sets SQL Query text.
     * If the Query text is different and the db statement was prepared earlier,
     * then the db statement is released and the new one is created.
     * @param _sql             Query SQL.
     */
    void sql(const String& _sql) override;

    /**
     * @brief Executes insert query.
     *
     * Retrieves value of "id" field and sets internal value for id().
     */
    void exec() override;

    /**
     * @brief Executes the query and closes the statement.
     *
     * Query SQL would be set to the new SQL statement.
     * @param newSQL            SQL statement to execute.
     */
    void exec(const String& newSQL) override
    {
        sql(newSQL);
        open();
    }

    /**
     * @brief Get created record id.
     * @return created record id.
     */
    [[nodiscard]] uint64_t id() const
    {
        return m_id;
    }

private:
    uint64_t m_id {0};         ///< The value of the 'id' field in the inserted record.
    String   m_idFieldName;    ///< The name of the auto-incremental field.
    SQuery   m_lastInsertedId; ///< The query retrieving the last inserted id (if needed by connection).

    /**
     * @brief Adjust the MS SQL insert query by.
     * @param sql               Query SQL.
     * @param idFieldName       Auto-incremental field name.
     * @returns Adjusted SQL.
     */
    static String reviewMsSqlQuery(const String& sql, const String& idFieldName);

    /**
     * @brief Adjust the insert query by adding RETURNING id if a connection type allows that.
     * @param connectionType    Database connection type.
     * @param sql               Query SQL.
     * @param idFieldName       Auto-incremental field name.
     * @returns Adjusted SQL.
     */
    static String reviewQuery(DatabaseConnectionType connectionType, const String& sql, const String& idFieldName);
};

} // namespace sptk
