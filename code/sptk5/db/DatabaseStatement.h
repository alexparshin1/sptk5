/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseStatement.h - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_DATABASESTATEMENT_H__
#define __SPTK_DATABASESTATEMENT_H__

#include <sptk5/db/QueryParameterList.h>

namespace sptk
{

/**
 * @brief Template class for database statements for different database drivers
 */
template <class Connection, class Statement> class DatabaseStatement
{
protected:
    /**
     * DB connection
     */
    Connection*     m_connection;

    /**
     * Statement
     */
    Statement*      m_statement;

    /**
     * Enumerated parameters
     */
    CParamVector    m_enumeratedParams;

    /**
     * Statement state type definition
     */
    typedef struct {
        /**
         * Number of columns is result set
         */
        unsigned    columnCount:12;

        /**
         * EOF (end of file) flag
         */
        bool        eof:1;

        /**
         * Transaction in progress flag
         */
        bool        transaction:1;

        /**
         * Output parameter count
         */
        unsigned    outputParameterCount:1;
    } State;

    /**
     * State flags
     */
    State m_state;


public:
    /**
     * @brief Constructor
     * @param connection Connection*, DB connection
     */
    DatabaseStatement(Connection* connection)
    : m_connection(connection), m_statement(nullptr), m_state({})
    {}

    /**
     * @brief Destructor
     */
    virtual ~DatabaseStatement()
    {}

    /**
     * @brief Returns current DB statement handle
     */
    Statement* stmt() const
    {
        return m_statement;
    }

    /**
     * @brief Generates normalized list of parameters
     * @param queryParams CParamList&, Standard query parameters
     */
    virtual void enumerateParams(QueryParameterList& queryParams)
    {
        queryParams.enumerate(m_enumeratedParams);
        m_state.outputParameterCount = 0;

        for (auto parameter: m_enumeratedParams) {
            if (parameter->isOutput())
                m_state.outputParameterCount++;
        }
    }

    /**
     * @brief Returns normalized list of parameters
     */
    CParamVector& enumeratedParams()
    {
        return m_enumeratedParams;
    }

    /**
     * @brief Returns true if statement uses output parameters
     */
    bool outputParameterCount() const
    {
        return m_state.outputParameterCount;
    }

    /**
     * @brief Sets actual parameter values for the statement execution
     */
    virtual void setParameterValues() = 0;

    /**
     * @brief Executes statement
     * @param inTransaction bool, True if statement is executed from transaction
     */
    virtual void execute(bool inTransaction) = 0;

    /**
     * @brief Closes statement and releases allocated resources
     */
    virtual void close() = 0;

    /**
     * @brief Fetches next record
     */
    virtual void fetch() = 0;

    /**
     * @brief Returns true if recordset is in EOF state
     */
    bool eof() const
    {
        return m_state.eof;
    }

    /**
     * @brief Returns recordset number of columns
     */
    unsigned colCount() const
    {
        return m_state.columnCount;
    }
};

}

#endif

