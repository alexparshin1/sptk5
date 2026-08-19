/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <source_location>
#include <sptk5/Strings.h>
#include <sptk5/sptk.h>

namespace sptk {
#ifndef _WIN32
#define DOESNT_THROW noexcept
#else
#define DOESNT_THROW throw()
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
class SP_EXPORT Exception : public std::exception
{
    /**
     * The file where exception occurs
     */
    const std::source_location m_location;

    /**
     * The exception text
     */
    String m_text;

    /**
     * The extended error information
     */
    String m_description;

    /**
     * The complete error information combining everything.
     */
    String m_fullMessage;

public:
    /**
     * @brief Constructor
     * @param text              The exception text
     * @param location          The location where the exception occurs
     * @param description       The optional description information.
     */
    explicit Exception(String text, const std::source_location& location = std::source_location::current(), String description = String()) DOESNT_THROW;

    /**
     * @brief Returns complete text of exception.
     */
    [[nodiscard]] const char* what() const noexcept override;

    /**
     * @brief Returns the exception message without the file name, line number, or description.
     */
    [[nodiscard]] String message() const;

    /**
     * @brief Returns exception location.
     */
    [[nodiscard]] std::source_location location() const;

    /**
     * @brief Returns exception description.
     */
    [[nodiscard]] String description() const;
};

/**
 * @brief Repeat operation exception.
 *
 * Thrown when the operation should be repeated.
 */
class SP_EXPORT RepeatOperationException : public Exception
{
public:
    using Exception::Exception;
};

/**
 * @brief Timeout exception.
 *
 * Thrown when the timeout error occurs.
 */
class SP_EXPORT TimeoutException : public Exception
{
public:
    /**
     * Constructor.
     * @param text              The exception text.
     * @param location          The location where the exception occurs.
     * @param description       The optional description information.
     */
    [[maybe_unused]] TimeoutException(const String& text, const std::source_location& location = std::source_location::current(), const String& description = String()) DOESNT_THROW;

    /**
     * @brief Copy constructor.
     * @param other             The other exception object.
     */
    TimeoutException(const TimeoutException& other) = default;
};

/**
 * @brief Connection exception.
 *
 * Thrown when the connection error occurs.
 */
class SP_EXPORT ConnectionException : public Exception
{
public:
    /**
     * Constructor.
     * @param text              The exception text.
     * @param location          The location where the exception occurs.
     * @param description       The optional description information.
     */
    explicit ConnectionException(const String& text, const std::source_location& location = std::source_location::current(), const String& description = String()) DOESNT_THROW;

    /**
     * @brief Copy constructor.
     * @param other             The other exception object.
     */
    ConnectionException(const ConnectionException& other) = default;
};

/**
 * @brief Database operation exception.
 *
 * Thrown when the database operation error occurs.
 */
class SP_EXPORT DatabaseException : public Exception
{
public:
    /**
     * @brief Constructor.
     * @param text              The exception text.
     * @param location          The location where the exception occurs.
     * @param description       The optional description information.
     */
    DatabaseException(const String& text, const std::source_location& location = std::source_location::current(), const String& description = String()) DOESNT_THROW;

    /**
     * @brief Copy constructor.
     * @param other             The other exception object.
     */
    DatabaseException(const DatabaseException& other) = default;
};

/**
 * @brief SOAP exception.
 *
 * Thrown every time when the SOAP fault occurs.
 */
class SP_EXPORT SOAPException : public Exception
{
public:
    /**
     * Constructor.
     * @param text              The exception text.
     * @param location          The location where the exception occurs.
     * @param description       The optional description information.
     */
    SOAPException(const String& text, const std::source_location& location = std::source_location::current(), const String& description = String()) DOESNT_THROW;

    /**
     * @brief Copy constructor.
     * @param other             The other exception object.
     */
    SOAPException(const SOAPException& other) = default;
};

/**
 * @brief SOAP exception.
 *
 * Thrown every time when the SOAP fault occurs.
 */
class SP_EXPORT HTTPException : public Exception
{
    size_t m_statusCode; ///< HTTP status code
    String m_statusText; ///< HTTP status text
public:
    /**
     * Constructor.
     * @param statusCode        The HTTP status.
     * @param text              The exception text.
     * @param location          The location where the exception occurs.
     * @param description       The optional description information.
     */
    HTTPException(size_t statusCode, const String& text, const std::source_location& location = std::source_location::current(), const String& description = String()) DOESNT_THROW;

    /**
     * @brief Copy constructor.
     * @param other             The other exception object.
     */
    HTTPException(const HTTPException& other) = default;

    /**
     * Get HTTP status code.
     * @return HTTP status code.
     */
    [[nodiscard]] size_t statusCode() const
    {
        return m_statusCode;
    }

    /**
     * Get HTTP status text.
     * @return HTTP status text.
     */
    [[nodiscard]] String statusText() const
    {
        return m_statusText;
    }

    /**
     * Return standard HTTP response status text for status code.
     * @param statusCode        The HTTP status code.
     * @return Status text.
     */
    [[nodiscard]] static String httpResponseStatus(size_t statusCode);
};

/**
 * @}
 */
} // namespace sptk
