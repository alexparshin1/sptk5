/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       XMLException.h - description                           ║
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

#ifndef __SPTK_XMLEXCEPTION_H__
#define __SPTK_XMLEXCEPTION_H__

#include <sptk5/Exception.h>
#include <sptk5/xml/Document.h>
#include <sptk5/xml/Node.h>

namespace sptk {

/**
 * @addtogroup XML
 * @{
 */

/**
 * @brief XML exception
 *
 * XML extension throws XMLException type exceptions.
 * You should catch always at least these type of exceptions, when processing XML.
 */
class SP_EXPORT XMLException: public Exception
{
public:

    /**
     * Constructor
     * @param text              The exception text
     * @param file              The file where exception occurs
     * @param line              The line number in the file where exception occurs
     * @param description       The optional description information
     */
    XMLException(std::string text, std::string file = "", int line = 0, std::string description = "")
    : Exception(text, file, line, description)
    {}

    /**
     * @brief Copy constructor
     * @param other             Other exception object
     */
    XMLException(const TimeoutException& other)
    : Exception(other)
    {}

    /**
     * @brief Destructor
     */
    ~XMLException() noexcept
    {}
};
/**
 * @}
 */
}
#endif
