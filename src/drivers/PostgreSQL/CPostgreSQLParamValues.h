/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPostgreSQLParamValues.h - description                 ║
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

#ifndef __CPOSTGRESQLPARAMVALUES_H__
#define __CPOSTGRESQLPARAMVALUES_H__

#include <sptk5/db/PostgreSQLConnection.h>
#include <sptk5/db/QueryParameterList.h>
#include "pgtypes.h"

namespace sptk {

    class CPostgreSQLParamValues {
        friend class CPostgreSQLStatement;
    protected:
        unsigned       m_size;
        unsigned       m_count;
        const char**   m_values;
        int*           m_lengths;
        int*           m_formats;
        Oid*           m_types;
        CParamVector   m_params;
        bool           m_int64timestamps;
    public:
        CPostgreSQLParamValues(bool int64timestamps) {
            m_count = 0;
            m_size = 0;
            m_values  = NULL;
            m_lengths = NULL;
            m_formats = NULL;
            m_types   = NULL;
            resize(16);
            m_int64timestamps = int64timestamps;
        }

        ~CPostgreSQLParamValues() {
            if (m_size) {
                free(m_values);
                free(m_lengths);
                free(m_formats);
                free(m_types);
            }
        }

        void reset() {
            m_count = 0;
        }

        void resize(unsigned sz) {
            if (sz > m_size) {
                m_size = sz;
                m_values  = (const char**) realloc(m_values,  m_size * sizeof(const char*));
                m_lengths = (int*)         realloc(m_lengths, m_size * sizeof(int));
                m_formats = (int*)         realloc(m_formats, m_size * sizeof(int));
                m_types   = (Oid*)         realloc(m_types,   m_size * sizeof(Oid));
            }
        }

        void setParameters(QueryParameterList& params);

        void setParameterValue(unsigned paramIndex, const void* value, unsigned sz, int32_t format, PG_DATA_TYPE dataType) {
            m_values[paramIndex] = (const char*) value;
            m_lengths[paramIndex] = (int) sz;
            m_formats[paramIndex] = format;
            m_types[paramIndex] = dataType;
        }

        void setParameterValue(unsigned paramIndex, QueryParameter* param) THROWS_EXCEPTIONS;

        unsigned size() const               { return m_count;   }
        const char* const* values() const   { return m_values;  }
        const int* lengths() const          { return m_lengths; }
        const int* formats() const          { return m_formats; }
        const Oid* types() const            { return m_types;   }
        const CParamVector& params() const  { return m_params;  }
    };

    extern const DateTime epochDate;

}

#endif
