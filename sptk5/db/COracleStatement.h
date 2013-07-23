/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          COracleStatement.h  -  description
                             -------------------
    begin                : Sun Jul 14 2013
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CORACLESTATEMENT_H__
#define __CORACLESTATEMENT_H__

#include <occi.h>

#include <list>
#include <string>
#include <stdio.h>

#include <sptk5/db/CParamList.h>

namespace sptk
{

class COracleConnection;

class COracleStatement
{
public:
    typedef oracle::occi::Connection Connection;
    typedef oracle::occi::Statement Statement;
    typedef oracle::occi::ResultSet ResultSet;
    typedef oracle::occi::MetaData MetaData;
private:
    COracleConnection*  m_connection;           ///< Oracle connection
    Statement*          m_statement;            ///< Main statement
    Statement*          m_createClobStatement;  ///< Statement for creating CLOBs
    Statement*          m_createBlobStatement;  ///< Statement for creating BLOBs
    ResultSet*          m_resultSet;            ///< Result set (if returned by statement)
    CParamVector        m_enumeratedParams;     ///< Enumerated parameters
    struct
    {
        unsigned    columnCount:12;         ///< Number of columns is result set
        bool        eof:1;                  ///< EOF (end of file) flag
        bool        transaction:1;          ///< Transaction in progress flag
        unsigned    outputParameterCount:1; ///< Output parameter count
    } m_state;                              ///< State flags

    /// @brief Sets character data to a CLOB parameter
    /// @param parameterIndex uint32_t, Parameter index
    /// @param data unsigned char*, Character data buffer
    /// @param dataSize uint32_t, Character data size
    void setClobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize);

    /// @brief Sets binary data to a BLOB parameter
    /// @param parameterIndex uint32_t, Parameter index
    /// @param data unsigned char*, Binary data buffer
    /// @param dataSize uint32_t, Binary data size
    void setBlobParameter(uint32_t parameterIndex, unsigned char* data, uint32_t dataSize);

public:
    /// @brief Constructor
    /// @param connection Connection*, Oracle connection
    /// @param sql std::string, SQL statement
    COracleStatement(COracleConnection* connection, std::string sql);

    /// @brief Destructor
    ~COracleStatement();

    /// @brief Returns statement handle
    Statement* stmt() const
    {
        return m_statement;
    }

    /// @brief Generates normalized list of parameters
    /// @param queryParams CParamList&, Standard query parameters
    void enumerateParams(CParamList& queryParams);

    /// @brief Returns normalized list of parameters
    CParamVector& enumeratedParams()
    {
        return m_enumeratedParams;
    }

    /// @brief Returns true if statement uses output parameters
    bool outputParameterCount() const
    {
        return m_state.outputParameterCount;
    }

    /// @brief Sets actual parameter values for the statement execution
    void setParameterValues();

    /// @brief Executes statement
    /// @param inTransaction bool, True if statement is executed from transaction
    void execute(bool inTransaction);

    /// @brief Closes statement and releases allocated resources
    void close();

    /// @brief Fetches next record
    void fetch()
    {
        if (m_resultSet) {
            m_state.eof = (m_resultSet->next() == ResultSet::END_OF_FETCH);
        }
    }

    bool eof() const
    {
        return m_state.eof;
    }

    unsigned colCount() const
    {
        return m_state.columnCount;
    }

    ResultSet* resultSet()
    {
        return m_resultSet;
    }
};

}

#endif