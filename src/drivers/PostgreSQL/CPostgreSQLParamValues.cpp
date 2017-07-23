/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPostgreSQLParamValues.cpp - description               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include "CPostgreSQLParamValues.h"
#include "htonq.h"

using namespace sptk;

void CPostgreSQLParamValues::setParameters(QueryParameterList& params) {
    params.enumerate(m_params);
    m_count = (unsigned) m_params.size();
    resize(m_count);
    for (unsigned i = 0; i < m_count; i++) {
        QueryParameter* param = m_params[i];
        VariantType ptype = param->dataType();
        PostgreSQLConnection::CTypeToPostgreType(ptype, m_types[i]);

        if ((ptype & (VAR_INT|VAR_INT64|VAR_FLOAT|VAR_BUFFER|VAR_DATE|VAR_DATE_TIME)) != 0) {
            m_formats[i] = 1; // Binary format
        } else
            m_formats[i] = 0; // Text format
        m_values[i] = param->conversionBuffer(); // This is a default. For VAR_STRING, VAR_TEXT, VAR_BUFFER and it would be replaced later

        switch (ptype) {
             case VAR_BOOL:
                m_lengths[i] = sizeof(bool);
                break;

             case VAR_INT:
                m_lengths[i] = sizeof(int32_t);
                break;

             case VAR_DATE:
             case VAR_DATE_TIME:
                m_lengths[i] = sizeof(int64_t);
                break;

             case VAR_FLOAT:
                m_lengths[i] = sizeof(double);
                break;

             case VAR_INT64:
                m_lengths[i] = sizeof(int64_t);
                break;
            default:
                m_lengths[i] = 0;
                break;
        }
    }
}

void CPostgreSQLParamValues::setParameterValue(unsigned paramIndex, QueryParameter* param) THROWS_EXCEPTIONS
{
    VariantType ptype = param->dataType();

    if (param->isNull())
        setParameterValue(paramIndex, nullptr, 0, 0, PG_VARCHAR);
    else {
        static const char* booleanTrue = "t";
        static const char* booleanFalse = "f";

        switch (ptype) {
            case VAR_BOOL:
                if (param->asBool())
                    setParameterValue(paramIndex, booleanTrue, 1, 0, PG_VARCHAR);
                else
                    setParameterValue(paramIndex, booleanFalse, 1, 0, PG_VARCHAR);
                break;

            case VAR_INT: {
                auto bufferToSend = (uint32_t*) param->conversionBuffer();
                *bufferToSend = htonl((uint32_t) param->getInteger());
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int32_t), 1, PG_INT4);
            }
            break;

            case VAR_DATE: {
                if (m_int64timestamps) {
                    int64_t dt = int64_t(double(param->getDateTime() - epochDate) + 1E-6) * 3600 * 24 * 1000000;
                    htonq_inplace((uint64_t*) &dt,(uint64_t*) param->conversionBuffer());
                } else {
                double dt = int32_t(double(param->getDateTime() - epochDate) + 1E-6) * 3600 * 24;
                htonq_inplace((uint64_t*) &dt,(uint64_t*) param->conversionBuffer());
                }
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1, PG_TIMESTAMP);
            }
            break;

            case VAR_DATE_TIME: {
                if (m_int64timestamps) {
                    int64_t dt = int64_t((param->getDateTime() - epochDate) * 3600 * 24 * 1000000);
                    htonq_inplace((uint64_t*) &dt,(uint64_t*) param->conversionBuffer());
                } else {
                double dt = (param->getDateTime() - epochDate) * 3600 * 24;
                htonq_inplace((uint64_t*) &dt,(uint64_t*) param->conversionBuffer());
                }
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1, PG_TIMESTAMP);
            }
            break;

            case VAR_INT64: {
                auto bufferToSend = (uint64_t*) param->conversionBuffer();
                *bufferToSend = htonq((uint64_t)param->getInt64());
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1, PG_INT8);
            }
            break;

            case VAR_MONEY: {
                double value = param->asFloat();
                auto bufferToSend = (uint64_t*) param->conversionBuffer();
                *bufferToSend = htonq(*(uint64_t*)&value);
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1, PG_FLOAT8);
            }
            break;

            case VAR_FLOAT: {
                auto bufferToSend = (uint64_t*) param->conversionBuffer();
                *bufferToSend = htonq(*(uint64_t*)param->dataBuffer());
                setParameterValue(paramIndex, param->conversionBuffer(), sizeof(int64_t), 1, PG_FLOAT8);
            }
            break;

            case VAR_STRING:
            case VAR_TEXT:
            case VAR_BUFFER:
                setParameterValue(paramIndex, param->getString(), (unsigned) param->dataSize(), 0, PG_VARCHAR);
                break;

            default:
                throw DatabaseException("Unsupported type of parameter " + int2string(paramIndex));
        }
    }
}

