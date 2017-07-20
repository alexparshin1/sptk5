/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Exception.h - description                              ║
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

#ifndef __SPTK_EXCEPTION_H__
#define __SPTK_EXCEPTION_H__

#include <sptk5/sptk.h>
#include <string>
#include <stdexcept>

namespace sptk {

#ifndef _WIN32
    #define DOESNT_THROW        noexcept
    #define THROWS_EXCEPTIONS   noexcept(false)
#else
    #define DOESNT_THROW        throw()
    #define THROWS_EXCEPTIONS   throw(std::exception)
#endif

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief SPTK generic exception class.
 *
 * Contains information about what
 * happened and where. It's based on std::exception, so if you
 * just want to catch STL and SPTK exceptions - you can use
 * try {} catch (std::exception& e) {} block.
 */
class SP_EXPORT Exception: public std::exception
{
    /**
     * The file where exception occurs
     */
    std::string m_file;

    /**
     * The line number in the file where exception occurs
     */
    int         m_line;

    /**
     * The exception text
     */
    std::string m_text;

    /**
     * The extended error information
     */
    std::string m_description;

    /**
     * The complete error information combining everything together
     */
    std::string m_fullMessage;

public:
    /**
     * @brief Constructor
     * @param text std::string, the exception text
     * @param file std::string, the file where exception occurs
     * @param line int, the line number in the file where exception occurs
     * @param description std::string, the optional description information
     */
    Exception(const std::string& text, const std::string& file = "", int line = 0, const std::string& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other const CException&, the other exception object
     */
    Exception(const Exception& other) DOESNT_THROW;

    /**
     * @brief Returns complete text of exception
     */
    virtual const char * what() const DOESNT_THROW;

    /**
     * @brief Returns exception message without file name, line number, or description
     */
    std::string message() const;

    /**
     * @brief Returns exception file name
     */
    std::string file() const;

    /**
     * @brief Returns exception line number
     */
    int line() const;

    /**
     * @brief Returns exception description
     */
    std::string description() const;
};

/**
 * @brief Timeout exception
 *
 * Thrown when timeout error occurs.
 */
class SP_EXPORT TimeoutException: public Exception
{
public:
    /**
     * Constructor
     * @param text std::string, the exception text
     * @param file std::string, the file where exception occurs
     * @param line int, the line number in the file where exception occurs
     * @param description std::string, the optional description information
     */
    TimeoutException(const std::string& text, const std::string& file = "", int line = 0, const std::string& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other const TimeoutException&, other exception object
     */
    TimeoutException(const TimeoutException& other) DOESNT_THROW;
};

/**
 * @brief Connection exception
 *
 * Thrown when connection error occurs.
 */
class SP_EXPORT ConnectionException: public Exception
{
public:
    /**
     * Constructor
     * @param text std::string, the exception text
     * @param file std::string, the file where exception occurs
     * @param line int, the line number in the file where exception occurs
     * @param description std::string, the optional description information
     */
    ConnectionException(const std::string& text, const std::string& file = "", int line = 0, const std::string& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other const ConnectionException&, other exception object
     */
    ConnectionException(const ConnectionException& other) DOESNT_THROW;
};

/**
 * @brief Database operation exception
 *
 * Thrown when database operation error occurs.
 */
class SP_EXPORT DatabaseException: public Exception
{
public:
    /**
     * @brief Constructor
     * @param text std::string, the exception text
     * @param file std::string, the file where exception occurs
     * @param line int, the line number in the file where exception occurs
     * @param description std::string, the optional description information
     */
    DatabaseException(const std::string& text, const std::string& file = "", int line = 0, const std::string& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other const DatabaseException&, other exception object
     */
    DatabaseException(const DatabaseException& other) DOESNT_THROW;
};

/**
 * @brief SOAP exception
 *
 * Thrown every time when SOAP fault occurs.
 */
class SP_EXPORT SOAPException: public Exception
{
public:
    /**
     * Constructor
     * @param text std::string, the exception text
     * @param file std::string, the file where exception occurs
     * @param line int, the line number in the file where exception occurs
     * @param description std::string, the optional description information
     */
    SOAPException(const std::string& text, const std::string& file = "", int line = 0, const std::string& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other const CSOAPException&, other exception object
     */
    SOAPException(const SOAPException& other) DOESNT_THROW;
};

/**
 * Defines a handy macros that automatically registers filename and line number
 * for the place an exception is thrown from
 */

/**
 * @brief Throws exception with file name and line number
 */
#define throwException(msg) throw sptk::Exception(msg,__FILE__,__LINE__)

/**
 * @brief Throws timeout exception with file name and line number
 */
#define throwTimeoutException(msg) throw sptk::TimeoutException(msg,__FILE__,__LINE__)

/**
 * @brief Throws connection exception with file name and line number
 */
#define throwConnectionException(msg) throw sptk::ConnectionException(msg,__FILE__,__LINE__)

/**
 * @brief Throws database exception with file name and line number
 */
#define throwDatabaseException(msg) throw sptk::DatabaseException(msg,__FILE__,__LINE__)

/**
 * @brief Throws SOAP exception with file name and line number
 */
#define throwSOAPException(msg) throw sptk::SOAPException(msg,__FILE__,__LINE__)

/**
 * @}
 */
}
#endif
