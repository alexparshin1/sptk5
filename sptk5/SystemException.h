/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SystemException.h - description                        ║
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

#ifndef __SPTK_SYSTEMEXCEPTION_H__
#define __SPTK_SYSTEMEXCEPTION_H__

#include <sptk5/Exception.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief SPTK OS operation exception
 *
 * Retrieves information about OS error after failed OS operation.
 */
class SP_EXPORT SystemException : public Exception
{
public:
    /**
     * @brief Constructor
     * @param context std::string, the exception context
     * @param file std::string, the file name where exception occurs
     * @param line int, the line number in the file where exception occurs
     */
    SystemException(std::string context, std::string file="", int line=0) DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other const SystemException&, the other exception object
     */
    SystemException(const SystemException& other) = default;

    /**
     * @brief Destructor
     */
    virtual ~SystemException() = default;

    /**
     * @brief Returns OS error
     */
    static std::string osError();
};

/**
 * @}
 */
}

#endif
