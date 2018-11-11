/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Exception.h - description                              ║
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

#ifndef __SPTK_EXCEPTION_H__
#define __SPTK_EXCEPTION_H__

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>
#include <stdexcept>
#include <sstream>

namespace sptk {

#ifndef _WIN32
    #define DOESNT_THROW        noexcept
#else
    #define DOESNT_THROW        throw()
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
    String      m_file;

    /**
     * The line number in the file where exception occurs
     */
    int         m_line;

    /**
     * The exception text
     */
    String      m_text;

    /**
     * The extended error information
     */
    String      m_description;

    /**
     * The complete error information combining everything together
     */
    String      m_fullMessage;

public:
    /**
     * @brief Constructor
     * @param text              The exception text
     * @param file              The file where exception occurs
     * @param line              The line number in the file where exception occurs
     * @param description       The optional description information
     */
    Exception(const String& text, const String& file = "", int line = 0, const String& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other             The other exception object
     */
    Exception(const Exception& other) DOESNT_THROW;

    /**
     * @brief Returns complete text of exception
     */
    virtual const char * what() const DOESNT_THROW;

    /**
     * @brief Returns exception message without file name, line number, or description
     */
    String message() const;

    /**
     * @brief Returns exception file name
     */
    String file() const;

    /**
     * @brief Returns exception line number
     */
    int line() const;

    /**
     * @brief Returns exception description
     */
    String description() const;
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
     * @param text              The exception text
     * @param file              The file where exception occurs
     * @param line              The line number in the file where exception occurs
     * @param description       The optional description information
     */
    TimeoutException(const String& text, const String& file = "", int line = 0, const String& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other             Other exception object
     */
    TimeoutException(const TimeoutException& other) = default;
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
     * @param text              The exception text
     * @param file              The file where exception occurs
     * @param line              The line number in the file where exception occurs
     * @param description       The optional description information
     */
    ConnectionException(const String& text, const String& file = "", int line = 0, const String& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other             Other exception object
     */
    ConnectionException(const ConnectionException& other) = default;
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
     * @param text              The exception text
     * @param file              The file where exception occurs
     * @param line              The line number in the file where exception occurs
     * @param description       The optional description information
     */
    DatabaseException(const String& text, const String& file = "", int line = 0, const String& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other             Other exception object
     */
    DatabaseException(const DatabaseException& other) = default;
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
     * @param text              The exception text
     * @param file              The file where exception occurs
     * @param line              The line number in the file where exception occurs
     * @param description       The optional description information
     */
    SOAPException(const String& text, const String& file = "", int line = 0, const String& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other             Other exception object
     */
    SOAPException(const SOAPException& other) = default;
};

/**
 * @brief SOAP exception
 *
 * Thrown every time when SOAP fault occurs.
 */
class SP_EXPORT HTTPException: public Exception
{
    size_t      m_statusCode;   ///< HTTP status code
    String      m_statusText;   ///< HTTP status text
public:
    /**
     * Constructor
     * @param statusCode        The HTTP status
     * @param text              The exception text
     * @param file              The file where exception occurs
     * @param line              The line number in the file where exception occurs
     * @param description       The optional description information
     */
    HTTPException(size_t statusCode, const String& text, const String& file = "", int line = 0, const String& description = "") DOESNT_THROW;

    /**
     * @brief Copy constructor
     * @param other             Other exception object
     */
    HTTPException(const HTTPException& other) = default;

    /**
     * Get HTTP status code
     * @return HTTP status code
     */
    size_t statusCode() const { return m_statusCode; }

    /**
     * Get HTTP status text
     * @return HTTP status text
     */
    String statusText() const { return m_statusText; }
};

/**
 * Defines a handy macros that automatically registers filename and line number
 * for the place an exception is thrown from
 */

/**
 * @brief Throws exception with file name and line number
 */
#define throwException(msg) { std::stringstream err; err << msg; throw sptk::Exception(err.str(),__FILE__,__LINE__); }

/**
 * @brief Throws timeout exception with file name and line number
 */
#define throwTimeoutException(msg) { std::stringstream err; err << msg; throw sptk::TimeoutException(err.str(),__FILE__,__LINE__); }

/**
 * @brief Throws connection exception with file name and line number
 */
#define throwConnectionException(msg) { std::stringstream err; err << msg; throw sptk::ConnectionException(err.str(),__FILE__,__LINE__); }

/**
 * @brief Throws database exception with file name and line number
 */
#define throwDatabaseException(msg) { std::stringstream err; err << msg; throw sptk::DatabaseException(err.str(),__FILE__,__LINE__); }

/**
 * @brief Throws SOAP exception with file name and line number
 */
#define throwSOAPException(msg) { std::stringstream err; err << msg; throw sptk::SOAPException(err.str(),__FILE__,__LINE__); }

/**
 * @}
 */
}
#endif
