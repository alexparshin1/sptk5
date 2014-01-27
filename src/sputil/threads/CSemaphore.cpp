/***************************************************************************
                         SIMPLY POWERFUL TOOLKIT (SPTK)
                         CSemaphore.cpp  -  description
                             -------------------
    begin                : Sat Feb 25 2012
    copyright            : (C) 1999-2013 by Alexey Parshin
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

#include <sptk5/threads/CSemaphore.h>
#include <sptk5/CSystemException.h>

#include <limits.h>
#include <time.h>

using namespace std;
using namespace sptk;

CSemaphore::CSemaphore(uint32_t startingValue)
: m_value(startingValue)
{
}

CSemaphore::~CSemaphore()
{
}

void CSemaphore::post() throw (exception)
{
    m_value++;
}

bool CSemaphore::wait(uint32_t timeoutMS) throw (exception)
{
    if (timeoutMS < 0)
        timeoutMS = 999999999;

    unique_lock<mutex>  lock(m_mutex);
    
    // Wait until semaphore value is greater than 0
    if (!m_condition.wait_for(lock, 
                              chrono::milliseconds(timeoutMS), 
                              [this](){return m_value > 0;}))
    {
        return false;
    }

    m_value--;

    return true;
}
