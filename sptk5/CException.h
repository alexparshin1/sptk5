/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CException.h - description                             ║
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

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

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

/// @addtogroup utility Utility Classes
/// @{

/// @brief SPTK generic exception class.
///
/// Contains information about what
/// happened and where. It's based on std::exception, so if you
/// just want to catch STL and SPTK exceptions - you can use
/// try {} catch (std::exception& e) {} block.
class SP_EXPORT CException: public std::exception
{
    std::string m_file;         ///< The file where exception occurs
    int         m_line;         ///< The line number in the file where exception occurs
    std::string m_text;         ///< The exception text
    std::string m_description;  ///< The extended error information
    std::string m_fullMessage;  ///< The complete error information combining everything together
public:
    /// @brief Constructor
    /// @param text std::string, the exception text
    /// @param file std::string, the file where exception occurs
    /// @param line int, the line number in the file where exception occurs
    /// @param description std::string, the optional description information
    CException(std::string text, std::string file = "", int line = 0, std::string description = "");

    /// @brief Copy constructor
    /// @param other const CException&, the other exception object
    CException(const CException& other);

    /// @brief Destructor
    ~CException() DOESNT_THROW;

    /// @brief Returns complete text of exception
    virtual const char * what() const DOESNT_THROW
    {
        return m_fullMessage.c_str();
    }

    /// @brief Returns exception message without file name, line number, or description
    std::string message() const
    {
        return m_text;
    }

    /// @brief Returns exception file name
    std::string file() const
    {
        return m_file;
    }

    /// @brief Returns exception line number
    int line() const
    {
        return m_line;
    }

    /// @brief Returns exception description
    std::string description() const
    {
        return m_description;
    }
};

/// @brief Timeout exception
///
/// Thrown every time when timeout error occurs.
class SP_EXPORT CTimeoutException: public CException
{
public:
    /// Constructor
    /// @param text std::string, the exception text
    /// @param file std::string, the file where exception occurs
    /// @param line int, the line number in the file where exception occurs
    /// @param description std::string, the optional description information
    CTimeoutException(std::string text, std::string file = "", int line = 0,
            std::string description = "") :
            CException(text, file, line, description)
    {
    }

    /// @brief Copy constructor
    /// @param other const CTimeoutException&, other exception object
    CTimeoutException(const CTimeoutException& other)
    : CException(other)
    {
    }

    /// @brief Destructor
    ~CTimeoutException() DOESNT_THROW;
};

/// @brief Database operation exception
///
/// Thrown every time when database operation error occurs.
class SP_EXPORT CDatabaseException: public CException
{
public:
    /// @brief Constructor
    /// @param text std::string, the exception text
    /// @param file std::string, the file where exception occurs
    /// @param line int, the line number in the file where exception occurs
    /// @param description std::string, the optional description information
    CDatabaseException(std::string text, std::string file = "", int line = 0,
            std::string description = "") :
            CException(text, file, line, description)
    {
    }

    /// @brief Copy constructor
    /// @param other const CDatabaseException&, other exception object
    CDatabaseException(const CDatabaseException& other)
    : CException(other)
    {
    }

    /// @brief Destructor
    ~CDatabaseException() DOESNT_THROW;
};

/// @brief SOAP exception
///
/// Thrown every time when SOAP fault occurs.
class SP_EXPORT CSOAPException: public CException
{
public:
    /// Constructor
    /// @param text std::string, the exception text
    /// @param file std::string, the file where exception occurs
    /// @param line int, the line number in the file where exception occurs
    /// @param description std::string, the optional description information
    CSOAPException(std::string text, std::string file = "", int line = 0,
                      std::string description = "") :
                      CException(text, file, line, description)
    {
    }

    /// @brief Copy constructor
    /// @param other const CSOAPException&, other exception object
    CSOAPException(const CSOAPException& other)
    : CException(other)
    {
    }

    /// @brief Destructor
    ~CSOAPException() DOESNT_THROW;
};

/// Defines a handy macros that automatically registers filename and line number
/// for the place an exception is thrown from

/// @brief Throws exception with file name and line number
#define throwException(msg) throw sptk::CException(msg,__FILE__,__LINE__)

/// @brief Throws timeout exception with file name and line number
#define throwTimeoutException(msg) throw sptk::CTimeoutException(msg,__FILE__,__LINE__)

/// @brief Throws database exception with file name and line number
#define throwDatabaseException(msg) throw sptk::CDatabaseException(msg,__FILE__,__LINE__)

/// @brief Throws SOAP exception with file name and line number
#define throwSOAPException(msg) throw sptk::CSOAPException(msg,__FILE__,__LINE__)

/// @}
}
#endif
