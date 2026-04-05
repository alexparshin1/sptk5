/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/Buffer.h>
#include <sptk5/CaseInsensitiveCompare.h>

#include <map>

namespace sptk {

/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief HTTP fields are implemented as a case-insensitive map.
 */
using StringHttpFieldMap = std::map<String, String, CaseInsensitiveCompare>;

class SP_EXPORT Url
{
public:
    /**
     * @brief Encodes a string into HTML parameters.
     */
    static String encode(const String& str);

    /**
     * @brief Decodes a string from HTML parameters.
     */
    static String decode(const String& str);
};

/**
 * @brief HTTP params.
 *
 * Designed to hold HTTP parameters in HttpConnect. It is a string-to-string
 * map with an addition of encode and decode functions for HTTP Mime.
 * The parameter names are case-insensitive.
 */
class SP_EXPORT HttpParams
{
public:
    /**
     * @brief Default constructor.
     */
    HttpParams() = default;

    /**
     * @brief Copy constructor.
     */
    HttpParams(const HttpParams& other) = default;

    /**
     * @brief Initialization constructor.
     */
    HttpParams(std::initializer_list<std::pair<String, String>> lst);

    /**
     * @brief Encodes URL parameters for sending to the server.
     * @param result            Output - encoded parameters string (if any) as the buffer.
     */
    void encode(Buffer& result) const;

    /**
     * @brief Decodes URL parameters that came from the server as a string into the parameter map.
     * @param buffer       Parameters string from HTTP server.
     */
    void decode(const Buffer& buffer);

    /**
     * @brief Returns parameter value, or empty string if not found.
     * @param paramName         Parameter name.
     * @return parameter value.
     */
    [[nodiscard]] String get(const String& paramName) const;

    /**
     * @brief Returns true if parameter exists.
     * @param paramName         Parameter name.
     * @return true if parameter exists.
     */
    [[nodiscard]] bool has(const String& paramName) const;

    /**
     * @return True if no parameters are set.
     */
    [[nodiscard]] bool empty() const;

    /**
     * @return Number of parameters.
     */
    [[nodiscard]] size_t size() const
    {
        return m_params.size();
    }

    [[nodiscard]] auto find(const String& paramName)
    {
        return m_params.find(paramName);
    }

    [[nodiscard]] auto find(const String& paramName) const
    {
        return m_params.find(paramName);
    }

    /**
     * @brief Returns an iterator to the first parameter.
     * @return Iterator to the beginning of the parameters' container.
     */
    [[nodiscard]] auto begin()
    {
        return m_params.begin();
    }

    /**
     * @brief Returns an iterator one past the last parameter.
     * @return Iterator to the end of the parameters' container.
     */
    [[nodiscard]] auto end()
    {
        return m_params.end();
    }

    /**
     * @brief Returns a const iterator to the first parameter.
     * @return Const iterator to the beginning of the parameters' container.
     */
    [[nodiscard]] auto begin() const
    {
        return m_params.begin();
    }

    /**
     * @brief Returns a const iterator one past the last parameter.
     * @return Const iterator to the end of the parameters' container.
     */
    [[nodiscard]] auto end() const
    {
        return m_params.end();
    }

    auto& operator[](const String& paramName)
    {
        return m_params[paramName];
    }

private:
    StringHttpFieldMap m_params;
};
/**
 * @}
 */
} // namespace sptk
