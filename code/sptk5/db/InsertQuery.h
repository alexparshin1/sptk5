/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
 * @addtogroup Database Database Support
 * @{
 */

/**
 * Database query
 *
 * A Dataset connected to the database to execute a database queries. The type of the database depends
 * on the DatabaseConnection object query is connected to.
 */
class SP_EXPORT InsertQuery : public Query
{
public:
    /**
     * Constructor
     *
     * You can optionally provide the name of the file and line number where
     * this query is created. This is used to collect statistical information
     * for the query calls. If file and line information is provided, then
     * calls statistics is stored to the database object during the query dtor.
     * @param db               Database connection
     * @param sql              SQL query, optional
     * @param idFieldName      Name of auto-incremental field
     */
    explicit InsertQuery(const DatabaseConnection& db, const String& sql = "", const String& idFieldName = "id");

    /**
     * Return query' SQL
     * @return SQL
     */
    String sql() const override;

    /**
     * Sets SQL Query text.
     * If the Query text is not the same and the db statement was prepared earlier
     * then the db statement is released and new one is created.
     * @param _sql             Query SQL
     */
    void sql(const String& _sql) override;

    /**
     * Executes insert query.
     *
     * Retrieves value of "id" field and sets internal value for id().
     */
    void exec() override;

    /**
     * Executes the query and closes the statement.
     *
     * Query SQL would be set to the new SQL statement
     * @param newSQL            SQL statement to execute
     */
    void exec(const String& newSQL) override
    {
        sql(newSQL);
        open();
    }

    /**
     * Get created record id
     * @return created record id
     */
    uint64_t id() const
    {
        return m_id;
    }

private:
    uint64_t m_id {0};         ///< The value of 'id' field in inserted record
    String   m_idFieldName;    ///< The name of auto-incremental field
    SQuery   m_lastInsertedId; ///< The query retrieving last inserted id (if needed by connection)

    /**
     * Adjust insert query by adding RETURNING id if connection type allows that
     * @param connectionType    Database connection type
     * @param sql               Query SQL
     * @param idFieldName       Auto-incremental field name
     * @returns Adjusted SQL
     */
    static String reviewQuery(DatabaseConnectionType connectionType, const String& sql, const String& idFieldName);
};

} // namespace sptk
