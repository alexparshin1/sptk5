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

#include <sptk5/sptk.h>

namespace sptk {
class String;
class Strings;

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Converts string to upper case
 */
std::string SP_EXPORT upperCase(std::string_view str);

/**
 * Converts string to lower case
 */
std::string SP_EXPORT lowerCase(std::string_view str);

/**
 * Trims string to remove leading and trailing spaces
 */
std::string SP_EXPORT trim(std::string_view str);

/**
 * Converts string to integer. The optional default value is used
 * for unsuccessful conversion
 */
int SP_EXPORT string2int(std::string_view str, int defaultValue = 0);

/**
 * Converts string to int64. The optional default value is used
 * for unsuccessful conversion
 */
int64_t SP_EXPORT string2int64(std::string_view str, int64_t defaultValue = 0);

/**
 * Converts double to string, using fixed format. Any trailing zeros are truncated.
 * @param value                 Double value
 * @return string presentation of double
 */
std::string SP_EXPORT double2string(double value);

/**
 * Converts string to double. The exception is thrown
 * for unsuccessful conversion
 */
double SP_EXPORT string2double(std::string_view str);

/**
 * Converts string to double. The optional default value is used
 * for unsuccessful conversion
 */
double SP_EXPORT string2double(std::string_view str, double defaultValue);

/**
 * Capitalizes all the words in string
 */
String SP_EXPORT capitalizeWords(const sptk::String& str);

/**
 * @}
 */

} // namespace sptk
