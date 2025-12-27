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

#include <sptk5/db/OracleOciConnection.h>
#include <sptk5/db/OracleOciStatement.h>

using namespace std;
using namespace sptk;
using namespace ocilib;

OracleOciStatement::OracleOciStatement(OracleOciConnection* connection, const string& sql)
    : DatabaseStatement<OracleOciConnection, ocilib::Statement>(connection)
    , m_ociConnection(connection->m_connection)
    , m_ociStatement(make_shared<Statement>(*connection->connection()))
    , m_sql(sql)
{
    statement(m_ociStatement.get());
    state().columnCount = 0;
    state().eof = true;
    state().transaction = false;
    state().outputParameterCount = 0;
}

OracleOciStatement::~OracleOciStatement()
{
    if (statement() != nullptr)
    {
        m_ociStatement.reset();
    }
}

void OracleOciStatement::bindParameters()
{
    if (m_parameterBinding.empty())
    {
        unsigned    parameterIndex = 1;
        const auto* stmt = statement();

        m_outputParamIndex.clear();

        for (const auto& parameterPtr: enumeratedParams())
        {
            QueryParameter&  parameter = *parameterPtr;
            VariantDataType& paramDataType = parameter.binding().m_dataType;

            paramDataType = parameter.dataType();
            const auto paramMark = ":" + to_string(parameterIndex);

            const auto paramBuffer = make_shared<OracleOciParameterBuffer>(paramDataType, m_ociConnection);
            if (!parameter.isOutput())
            {
                paramBuffer->bind(*stmt, paramMark, BindInfo::BindDirectionValues::In);
            }
            else
            {
                paramBuffer->bindOutput(*stmt, paramMark);
                m_outputParamIndex.push_back(parameterIndex);
            }
            m_parameterBinding.push_back(paramBuffer);

            ++parameterIndex;
        }
    }
}

void OracleOciStatement::setParameterValues()
{
    unsigned    parameterIndex = 1;
    const auto* stmt = statement();

    for (const auto& parameterPtr: enumeratedParams())
    {
        QueryParameter&  parameter = *parameterPtr;
        VariantDataType& paramDataType = parameter.binding().m_dataType;

        paramDataType = parameter.dataType();
        const auto paramMark = ":" + to_string(parameterIndex);

        m_parameterBinding[parameterIndex - 1]->setValue(parameter);

        if (!parameter.isOutput())
        {
            stmt->GetBind(paramMark).SetDataNull(parameter.isNull());
        }

        ++parameterIndex;
    }
}

void OracleOciStatement::execute(bool inTransaction)
{
    state().eof = true;
    state().columnCount = 0;

    statement()->ExecutePrepared();
    auto resultSet = statement()->GetResultset();
    if (resultSet)
    {
        state().eof = false;
        state().columnCount = resultSet.GetColumnCount();
    }
}

void OracleOciStatement::close()
{
    // Nothing to do.
}

void OracleOciStatement::fetch()
{
    auto resultSet = m_ociStatement->GetResultset();
    if (resultSet)
    {
        state().eof = !resultSet.Next();
    }
}

void OracleOciStatement::enumerateParams(QueryParameterList& queryParams)
{
    DatabaseStatement::enumerateParams(queryParams);
    m_parameterBinding.clear();
}
