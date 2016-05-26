/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          LogEngine.cpp  -  description
                             -------------------
    begin                : Mon Jan 30 2006
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.

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

#include <sptk5/LogEngine.h>

using namespace std;
using namespace sptk;

LogEngine::LogEngine()
{
    m_defaultPriority = LP_INFO;
    m_minPriority = LP_INFO;
    m_options = LO_ENABLE | LO_DATE | LO_TIME | LO_PRIORITY;
}

LogEngine::~LogEngine()
{
}


void LogEngine::option(Option option, bool flag)
{
    SYNCHRONIZED_CODE;
    if (flag)
        m_options |= option;
    else
        m_options &= ~option;
}

string LogEngine::priorityName(LogPriority prt)
{
    switch (prt) {
    case LP_DEBUG:     return "DEBUG";
    case LP_INFO:      return "INFO";
    case LP_NOTICE:    return "NOTICE";
    case LP_WARNING:   return "WARNING";
    case LP_ERROR:     return "ERROR";
    case LP_CRITICAL:  return "CRITICAL";
    case LP_ALERT:     return "ALERT";
    case LP_PANIC:     return "PANIC";
    }
    return "";
}
