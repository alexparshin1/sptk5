/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          cexception.h  -  description
                             -------------------
    begin                : Thu Apr 27 2000
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <sptk5/sptk.h>
#include <string>
#include <stdexcept>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief SPTK generic exception class.
///
/// Contains information about what
/// happened and where. It's based on std::exception, so if you
/// just want to catch STL and SPTK exceptions - you can use
/// try {} catch (std::exception& e) {} block.
class SP_EXPORT CException : public std::exception {
    std::string m_file;         ///< The file where exception occurs
    int         m_line;         ///< The line number in the file where exception occurs
    std::string m_text;         ///< The exception text
    std::string m_description;  ///< The extended error information
    std::string m_fullMessage;  ///< The complete error information combining everything together
public:
    /// Constructor
    /// @param text std::string, the exception text
    /// @param file std::string, the file where exception occurs
    /// @param line int, the line number in the file where exception occurs
    /// @param description std::string, the optional description information
    CException(std::string text,std::string file="",int line=0,std::string description="");

    /// Destructor
    virtual ~CException() throw() {}

    /// Returns complete text of exception
    virtual const char * what() const throw() {
        return m_fullMessage.c_str();
    }

    /// Returns exception message without file name, line number, or description
    std::string message() const {
        return m_text;
    }

    /// Returns exception file name
    std::string file() const {
        return m_file;
    }

    /// Returns exception line number
    int line() const {
        return m_line;
    }

    /// Returns exception description
    std::string description() const {
        return m_description;
    }
};

/// @brief Timeout exception
///
/// Thrown every time when timeout error occurs.
class SP_EXPORT CTimeoutException : public CException {
public:
    /// Constructor
    /// @param text std::string, the exception text
    /// @param file std::string, the file where exception occurs
    /// @param line int, the line number in the file where exception occurs
    /// @param description std::string, the optional description information
    CTimeoutException(std::string text,std::string file="",int line=0,std::string description="") :
        CException(text, file, line, description) {}
};

/// @brief Database operation exception
///
/// Thrown every time when database operation error occurs.
class SP_EXPORT CDatabaseException : public CException {
public:
    /// Constructor
    /// @param text std::string, the exception text
    /// @param file std::string, the file where exception occurs
    /// @param line int, the line number in the file where exception occurs
    /// @param description std::string, the optional description information
    CDatabaseException(std::string text,std::string file="",int line=0,std::string description="") :
        CException(text, file, line, description) {}
};

/// Defines a handy macros that automatically registers filename and line number
/// for the place an exception is thrown from
#define throwException(msg) throw CException(msg,__FILE__,__LINE__)
#define throwTimeoutException(msg) throw CTimeoutException(msg,__FILE__,__LINE__)

/// @}
}
#endif
