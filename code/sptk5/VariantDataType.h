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

#include <cstddef>
#include <cstdint>

namespace sptk {
/**
 * @addtogroup utility Utility Classes.
 * @{
 */

/**
 * Variant types
 */
enum class VariantDataType : uint16_t
{
    VAR_NONE = 0,          ///< Undefined.
    VAR_INT = 1,           ///< Integer.
    VAR_FLOAT = 2,         ///< Floating-point (double).
    VAR_MONEY = 4,         ///< Special (integer quantity and scale) money.
    VAR_STRING = 8,        ///< String pointer.
    VAR_TEXT = 16,         ///< String pointer, corresponding to BLOBS in database.
    VAR_BUFFER = 32,       ///< Data pointer, corresponding to BLOBS in database.
    VAR_DATE = 64,         ///< DateTime (double) w/o time part.
    VAR_DATE_TIME = 128,   ///< DateTime (double).
    VAR_IMAGE_PTR = 256,   ///< Image pointer.
    VAR_IMAGE_NDX = 512,   ///< Image index in object-specific table of image pointers.
    VAR_INT64 = 1024,      ///< 64bit integer.
    VAR_BOOL = 2048,       ///< Boolean.
    VAR_BYTE_POINTER = 256 ///< Byte pointer.
};

struct VariantType
{
    VariantDataType type : 12;
    bool            isNull : 1;
    bool            isExternalBuffer : 1;
    size_t          size : 48;
};

/**
 * @}
 */
} // namespace sptk
