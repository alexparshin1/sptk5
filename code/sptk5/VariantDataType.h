/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Variant types
 */
enum class VariantDataType : uint16_t
{
    VAR_NONE = 0,        ///< Undefined
    VAR_INT = 1,         ///< Integer
    VAR_FLOAT = 2,       ///< Floating-point (double)
    VAR_MONEY = 4,       ///< Special (integer quantity and scale) money
    VAR_STRING = 8,      ///< String pointer
    VAR_TEXT = 16,       ///< String pointer, corresponding to BLOBS in database
    VAR_BUFFER = 32,     ///< Data pointer, corresponding to BLOBS in database
    VAR_DATE = 64,       ///< DateTime (double)
    VAR_DATE_TIME = 128, ///< DateTime (double)
    VAR_IMAGE_PTR = 256, ///< Image pointer
    VAR_IMAGE_NDX = 512, ///< Image index in object-specific table of image pointers
    VAR_INT64 = 1024,    ///< 64bit integer
    VAR_BOOL = 2048      ///< Boolean
};

/**
 * @}
 */
} // namespace sptk
