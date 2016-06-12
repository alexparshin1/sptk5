/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        CParamBinding.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CPARAMBINDING_H__
#define __CPARAMBINDING_H__

#include <sptk5/sptk.h>
#include <sptk5/CVariant.h>

#include <vector>
#include <map>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

/// @brief Parameter Binding descriptor
///
/// Stores the last information on parameter binding
class SP_EXPORT CParamBinding
{
public:
    void*        m_stmt;        ///< Statement handle or id
    CVariantType m_dataType;    ///< Data type
    void*        m_buffer;      ///< Buffer
    uint32_t     m_size;        ///< Buffer size
    bool         m_output;      ///< Output parameter flag
public:
    /// @brief Constructor
    /// @param isOutput bool, Output parameter flag
    CParamBinding(bool isOutput)
    {
        reset(isOutput);
    }

    /// @brief Resets the binding information
    /// @param isOutput bool, Output parameter flag
    void reset(bool isOutput);

    /// @brief Checks if the parameter binding is matching the cached
    ///
    /// Returns true, if the passed parameters are matching last binding parameters.
    /// Returns false and stores new parameters into last binding parameters otherwise.
    /// @param stmt void*, statement handle
    /// @param type CVariantType, data type
    /// @param size uint32_t, binding buffer size
    /// @param buffer void*, binding buffer
    bool check(void* stmt, CVariantType type, uint32_t size, void* buffer);
};

/// @}
}

#endif
