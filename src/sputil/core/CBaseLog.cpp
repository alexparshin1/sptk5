/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CBaseLog.cpp  -  description
                             -------------------
    begin                : Mon Jan 30 2006
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
 
    This module creation was sponsored by Total Knowledge (http://www.total-knowledge.com).
    Author thanks the developers of CPPSERV project (http://www.total-knowledge.com/progs/cppserv)
    for defining the requirements for this class.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sptk5/CBaseLog.h>

using namespace std;
using namespace sptk;
/*
SP_EXPORT const CLogPriority sptk::CLP_DEBUG(LOG_DEBUG, "debug"), sptk::CLP_INFO(LOG_INFO, "info"), sptk::CLP_NOTICE(LOG_NOTICE,
        "notice"), sptk::CLP_WARNING(LOG_WARNING, "warning"), sptk::CLP_ERROR(LOG_ERR, "err"), sptk::CLP_CRITICAL(LOG_CRIT,
        "crit"), sptk::CLP_ALERT(LOG_ALERT, "alert"), sptk::CLP_PANIC(LOG_EMERG, "panic");
*/
//==========================================================================================
CLogStreamBuf::CLogStreamBuf()
{
    m_parent = 0;
    m_bytes = 0;
    m_size = 1024;
    m_buffer = (char *) malloc(m_size);
    m_priority = CLP_NOTICE;
    m_date = CDateTime::Now();
}

streambuf::int_type CLogStreamBuf::overflow(streambuf::int_type c)
{
    bool bufferOverflow = m_bytes > m_size - 2;
    bool lineBreak = c <= 13;

    if (lineBreak || bufferOverflow) {
        if (m_bytes) {
            m_buffer[m_bytes] = 0;
            if (m_parent && m_priority <= m_parent->minPriority())
                m_parent->saveMessage(m_date, m_buffer, m_bytes, m_priority);
            if (!bufferOverflow) {
                m_priority = m_parent->defaultPriority();
                m_date = CDateTime::Now();
            }
            m_bytes = 0;
        }
    }
    if (!lineBreak) {
        if (m_bytes == 0)
            m_date = CDateTime::Now();
        m_buffer[m_bytes] = (char) c;
        m_bytes++;
    }
    return traits_type::not_eof(c);
}
//==========================================================================================
SP_EXPORT CBaseLog& sptk::operator <<(CBaseLog &stream, CLogPriority priority)
{
    stream.priority(priority);
    return stream;
}

void CBaseLog::option(CLogOption option, bool flag)
{
    if (flag)
        m_options |= option;
    else
        m_options &= ~option;
}

string CBaseLog::priorityName(CLogPriority prt)
{
    switch (prt) {
    case CLP_DEBUG:     return "debug";
    case CLP_INFO:      return "info";
    case CLP_NOTICE:    return "notice";
    case CLP_WARNING:   return "warning";
    case CLP_ERROR:     return "err";
    case CLP_CRITICAL:  return "crit";
    case CLP_ALERT:     return "alert";
    case CLP_PANIC:     return "panic";
    }
    return "";
}
//==========================================================================================
