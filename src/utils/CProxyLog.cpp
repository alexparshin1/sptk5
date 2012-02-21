/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CProxyLog.cpp  -  description
                             -------------------
    begin                : Tue Jan 11 2008
    copyright            : (C) 2008-2012 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CProxyLog.h>
#include <sptk5/CGuard.h>

using namespace std;
using namespace sptk;

void CProxyLog::saveMessage(CDateTime date,const char *message,uint32_t sz,const CLogPriority *priority) throw(CException) {
    if (m_options & CLO_ENABLE) {
        CGuard guard(m_destination);
        m_destination.saveMessage(date,message,sz,priority);
    }
}

void CProxyLog::reset() throw(CException) {
   CGuard guard(m_destination);
   m_destination.reset();
}
