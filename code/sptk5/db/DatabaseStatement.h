/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#include <sptk5/db/QueryParameterList.h>

namespace sptk {

/**
 * @brief Template class for database statements for different database drivers.
 */
template<class Connection, class Statement>
class DatabaseStatement
{
public:
    Connection* connection() const
    {
        return m_connection;
    }

    void statement(Statement* stmt)
    {
        m_statement = stmt;
    }

    Statement* statement() const
    {
        return m_statement;
    }

    /**
     * @brief Constructor.
     * @param connection Connection*, DB connection.
     */
    explicit DatabaseStatement(Connection* connection)
        : m_connection(connection)
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~DatabaseStatement() = default;

    /**
     * @brief Returns current DB statement handle.
     */
    Statement* stmt() const
    {
        return m_statement;
    }

    /**
     * @brief Generates the normalized list of parameters.
     * @param queryParams CParamList&, Standard query parameters.
     */
    virtual void enumerateParams(QueryParameterList& queryParams)
    {
        queryParams.enumerate(m_enumeratedParams);
        m_state.outputParameterCount = 0;

        for (const auto& parameter: m_enumeratedParams)
        {
            if (parameter->isOutput())
                ++m_state.outputParameterCount;
        }
    }

    /**
     * @brief Returns the normalized list of parameters.
     */
    ParamVector& enumeratedParams()
    {
        return m_enumeratedParams;
    }

    /**
     * @brief Returns true if statement uses output parameters.
     */
    [[nodiscard]] size_t outputParameterCount() const
    {
        return m_state.outputParameterCount;
    }

    /**
     * @brief Sets actual parameter values for the statement execution.
     */
    virtual void setParameterValues() = 0;

    /**
     * @brief Executes statement.
     * @param inTransaction bool, True if the statement is executed from transaction.
     */
    virtual void execute(bool inTransaction) = 0;

    /**
     * @brief Closes statement and releases allocated resources.
     */
    virtual void close() = 0;

    /**
     * @brief Fetches next record.
     */
    virtual void fetch() = 0;

    /**
     * @brief Returns true if the recordset is in EOF state.
     */
    [[nodiscard]] bool eof() const
    {
        return m_state.eof;
    }

    /**
     * @brief Returns recordset number of columns.
     */
    [[nodiscard]] unsigned colCount() const
    {
        return m_state.columnCount;
    }

protected:
    /**
     * @brief Statement state type definition.
     */
    struct State
    {
        /**
         * @brief Number of columns is result set.
         */
        unsigned columnCount : 12;

        /**
         * @brief EOF (end of file) flag.
         */
        bool eof : 1;

        /**
         * @brief Transaction in progress flag.
         */
        bool transaction : 1;

        /**
         * @brief Output parameter count.
         */
        unsigned outputParameterCount : 1;
    };

    State& state()
    {
        return m_state;
    }

private:
    Connection* m_connection {nullptr}; ///< DB connection.
    Statement*  m_statement {nullptr};  ///< Statement.
    State       m_state {};             ///< State flags.
    ParamVector m_enumeratedParams;     ///< Enumerated parameters.
};

} // namespace sptk
