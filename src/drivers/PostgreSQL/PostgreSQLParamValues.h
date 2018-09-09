/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPostgreSQLParamValues.h - description                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __CPOSTGRESQLPARAMVALUES_H__
#define __CPOSTGRESQLPARAMVALUES_H__

#include "pgtypes.h"
#include <sptk5/db/PostgreSQLConnection.h>
#include <sptk5/db/QueryParameterList.h>

namespace sptk {

    class PostgreSQLParamValues
    {
        friend class PostgreSQLStatement;
    protected:
        size_t                      m_count;
        std::vector<const char*>    m_values;
        std::vector<int>            m_lengths;
        std::vector<int>            m_formats;
        std::vector<Oid>            m_types;
        CParamVector                m_params;
        bool                        m_int64timestamps;
    public:
        explicit PostgreSQLParamValues(bool int64timestamps)
        : m_count(0)
        {
            resize(16);
            m_int64timestamps = int64timestamps;
        }

        void reset()
        {
            m_count = 0;
        }

        void resize(size_t sz)
        {
            m_values.resize(sz);
            m_lengths.resize(sz);
            m_formats.resize(sz);
            m_types.resize(sz);
        }

        void setParameters(QueryParameterList& params);

        void setParameterValue(unsigned paramIndex, const void* value, unsigned sz, int32_t format, PG_DATA_TYPE dataType)
        {
            m_values[paramIndex] = (const char*) value;
            m_lengths[paramIndex] = (int) sz;
            m_formats[paramIndex] = format;
            m_types[paramIndex] = dataType;
        }

        void setParameterValue(unsigned paramIndex, QueryParameter* param);

        unsigned size() const               { return (unsigned) m_count;   }
        const char* const* values() const   { return &m_values[0]; }
        const int* lengths() const          { return &m_lengths[0]; }
        const int* formats() const          { return &m_formats[0]; }
        const Oid* types() const            { return &m_types[0]; }
        const CParamVector& params() const  { return m_params;  }
    };

    extern const DateTime epochDate;
    extern const long     daysSinceEpoch;
    extern const int64_t  microsecondsSinceEpoch;

} // namespace sptk

#endif
