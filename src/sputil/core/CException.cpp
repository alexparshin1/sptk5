/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CException.cpp  -  description
                             -------------------
    begin                : Thu Apr 27 2000
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CException.h>
#include <sptk5/CStrings.h>

using namespace std;
using namespace sptk;

CException::CException(string text, string file, int line, string description)
: m_file(file), m_line(line), m_text(text), m_description(description), m_fullMessage(m_text)
{
    if (m_line && !m_file.empty())
        m_fullMessage += " " + m_file + "(" + int2string(uint32_t(m_line)) + ") ";

    if (!m_description.empty())
        m_fullMessage += "\n" + m_description;
}

CException::CException(const CException& other)
: m_file(other.m_file), m_line(other.m_line), m_text(other.m_text), m_description(other.m_description), m_fullMessage(other.m_fullMessage)
{
}

CException::~CException() DOESNT_THROW
{
}

CTimeoutException::~CTimeoutException() DOESNT_THROW
{
}

CDatabaseException::~CDatabaseException() DOESNT_THROW
{
}
