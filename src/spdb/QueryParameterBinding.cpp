/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CParamBinding.cpp - description                        ║
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

#include <sptk5/db/QueryParameterBinding.h>
//#include <sptk5/string_ext.h>

using namespace std;
using namespace sptk;

void QueryParameterBinding::reset(bool isOutput)
{
    m_stmt = nullptr;
    m_dataType = VAR_NONE;
    m_size = 0;
    m_buffer = nullptr;
    m_output = isOutput;
}

bool QueryParameterBinding::check(void* stmt, VariantType type, uint32_t size, void* buffer)
{
    bool changed = true;

    if (m_stmt != stmt) {
        m_stmt = stmt;
        changed = false;
    }

    if (m_dataType != type) {
        m_dataType = type;
        changed = false;
    }

    if (m_size != size) {
        m_size = size;
        changed = false;
    }

    if (m_buffer != buffer) {
        m_buffer = buffer;
        changed = false;
    }

    return changed;
}
