/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CFileLog.cpp  -  description
                             -------------------
    begin                : Tue Jan 31 2006
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

#include <sptk5/CFileLog.h>

using namespace std;
using namespace sptk;

void CFileLog::saveMessage(CDateTime date, const char *message, uint32_t, CLogPriority priority) THROWS_EXCEPTIONS
{
    SYNCHRONIZED_CODE;
    if (options() & CLO_ENABLE) {
        if (!m_fileStream.is_open()) {
            m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::app);
            if (!m_fileStream.is_open())
                throw CException("Can't append or create log file '" + m_fileName + "'", __FILE__, __LINE__);
        }

        if (options() & CLO_DATE)
            m_fileStream << date.dateString() << " ";

        if (m_options & CLO_TIME)
            m_fileStream << date.timeString(true) << " ";

        if (m_options & CLO_PRIORITY)
            m_fileStream << "[" << priorityName(priority) << "] ";

        m_fileStream << message << endl;
    }

    if (options() & CLO_STDOUT) {
        if (m_options & CLO_DATE)
            cout << date.dateString() << " ";

        if (m_options & CLO_TIME)
            cout << date.timeString(true) << " ";

        if (m_options & CLO_PRIORITY)
            cout << "[" << priorityName(priority) << "] ";

        cout << message << endl;
    }

    if (m_fileStream.bad())
        throw CException("Can't write to log file '" + m_fileName + "'", __FILE__, __LINE__);
}

CFileLog::~CFileLog()
{
    SYNCHRONIZED_CODE;
    m_buffer->flush();
    if (m_fileStream.is_open())
        m_fileStream.close();
}

void CFileLog::reset() THROWS_EXCEPTIONS
{
    SYNCHRONIZED_CODE;
    if (m_fileStream.is_open())
        m_fileStream.close();
    if (m_fileName.empty())
        throw CException("File name isn't defined", __FILE__, __LINE__);
    m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::trunc);
    if (!m_fileStream.is_open())
        throw CException("Can't open log file '" + m_fileName + "'", __FILE__, __LINE__);
}
