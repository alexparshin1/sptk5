/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRWLock.cpp  -  description
                             -------------------
    begin                : Thu Apr 28 2005
    copyright            : (C) 1999-2016 by Ilya A. Volynets-Evenbakh
    email                : ilya@total-knowledge.com
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

#include <sptk5/sptk.h>
#include <sptk5/threads/CRWLock.h>
#include <time.h>
#include <errno.h>
#include <thread>

using namespace sptk;
using namespace std;

CRWLock::CRWLock()
{
    m_readerCount = 0;
    m_writerMode = false;
}

CRWLock::~CRWLock()
{
}

int CRWLock::lockR(int timeoutMS)
{
    if (timeoutMS < 0)
        timeoutMS = 999999999;

    unique_lock<mutex>  lock(m_writeLock);

    // Wait for no writers
    if (!m_condition.wait_for(lock, 
                              chrono::milliseconds(timeoutMS), 
                              [this](){return m_writerMode == false;}))
    {
        return false;
    }

    m_readerCount++;

    return true;
}

int CRWLock::lockRW(int timeoutMS)
{
    if (timeoutMS < 0)
        timeoutMS = 999999999;

    unique_lock<mutex>  lock(m_writeLock);
    
    // Wait for no readers or writers
    if (!m_condition.wait_for(lock, 
                              chrono::milliseconds(timeoutMS), 
                              [this](){return m_writerMode == false && m_readerCount == 0;}))
    {
        return false;
    }

    m_writerMode = true;

    return true;
}

void CRWLock::unlock()
{
    lock_guard<mutex>   guard(m_writeLock);
    if (m_writerMode)
        m_writerMode = false;
    else
        if (m_readerCount > 0)
            m_readerCount--;
}
