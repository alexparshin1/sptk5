/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       HttpParams.h - description                             ║
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

#ifndef __HTTP_PARAMS_H__
#define __HTTP_PARAMS_H__

#include <sptk5/sptk.h>
#include <sptk5/Buffer.h>
#include <sptk5/CaseInsensitiveCompare.h>

#include <string>
#include <map>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * HTTP fields are implemented as case-insensitive map
 */
typedef std::map<sptk::String, sptk::String, CaseInsensitiveCompare> StringHttpFieldMap;

/**
 * @brief HTTP params map
 *
 * Designed to hold HTTP parametrs in
 * CHttpConnect and CCgiApplication. It is, basically, a string-to-string
 * map with an addition of encode and decode functions for HTTP Mime.
 */
class HttpParams: public StringHttpFieldMap
{
    /**
     * @brief Encodes a string into HTML parameters
     */
    static String encodeString(const String& str);

    /**
     * @brief Decodes a string from HTML parameters
     */
    static sptk::String decodeString(const String& str);
public:
    /**
     * @brief Default constructor.
     */
    HttpParams()
    : StringHttpFieldMap()
    {
    }

    /**
     * @brief Encodes HTTP parameters for sending to the server.
     * @param result            Output - encoded parameters string (if any) as the buffer.
     */
    void encode(Buffer& result) const;

    /**
     * @brief Decodes HTTP parameters that came from the server as a string into parameters map.
     * @param paramString       Parameters string from HTTP server
     * @param lowerCaseNames    True if you want to lower-case the parameter names
     */
    void decode(const Buffer& paramString, bool lowerCaseNames = false);

    /**
     * @brief Returns parameter value, or empty string if not found
     * @param paramName         Parameter name
     * @return parameter value
     */
    String get(const String& paramName) const;
};
/**
 * @}
 */
}
#endif
