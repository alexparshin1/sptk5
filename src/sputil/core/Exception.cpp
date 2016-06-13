/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Exception.cpp - description                            ║
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

#include <sptk5/Exception.h>
#include <sptk5/Strings.h>

using namespace std;
using namespace sptk;

Exception::Exception(string text, string file, int line, string description)
: m_file(file), m_line(line), m_text(text), m_description(description), m_fullMessage(m_text)
{
    if (m_line && !m_file.empty())
        m_fullMessage += " " + m_file + "(" + int2string(uint32_t(m_line)) + ") ";

    if (!m_description.empty())
        m_fullMessage += "\n" + m_description;
}

Exception::Exception(const Exception& other)
: m_file(other.m_file), m_line(other.m_line), m_text(other.m_text), m_description(other.m_description), m_fullMessage(other.m_fullMessage)
{
}

Exception::~Exception() DOESNT_THROW
{
}

const char* Exception::what() const DOESNT_THROW
{
    return m_fullMessage.c_str();
}

std::string Exception::message() const
{
    return m_text;
}

string Exception::file() const
{
    return m_file;
}

int Exception::line() const
{
    return m_line;
}

string Exception::description() const
{
    return m_description;
}

TimeoutException::TimeoutException(std::string text, std::string file, int line, string description)
: Exception(text, file, line, description)
{
}

TimeoutException::TimeoutException(const TimeoutException& other)
: Exception(other)
{
}

TimeoutException::~TimeoutException() DOESNT_THROW
{
}

DatabaseException::DatabaseException(std::string text, std::string file, int line, std::string description)
: Exception(text, file, line, description)
{
}

DatabaseException::DatabaseException(const DatabaseException& other)
: Exception(other)
{
}
    
DatabaseException::~DatabaseException() DOESNT_THROW
{
}

SOAPException::SOAPException(std::string text, std::string file, int line, string description)
: Exception(text, file, line, description)
{
}

SOAPException::SOAPException(const SOAPException& other)
: Exception(other)
{
}

SOAPException::~SOAPException() DOESNT_THROW
{
}
