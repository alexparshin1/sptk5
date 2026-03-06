/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
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

#include <sptk5/db/PoolDatabaseConnection.h>
#include <sptk5/sptk.h>

namespace sptk {

/**
 * @addtogroup Database Database Support.
 * @{
 */

/**
 *  Parameter Binding descriptor.
 *
 * Stores the last information on parameter binding.
 */
class SP_EXPORT QueryParameterBinding
{
public:
    /**
     * @brief Statement handle or id.
     */
    StmtHandle m_stmt {nullptr};

    /**
     * @brief Data type.
     */
    VariantDataType m_dataType {VariantDataType::VAR_NONE};

    /**
     * @brief Buffer.
     */
    uint8_t* m_buffer {nullptr};

    /**
     * @brief Buffer size.
     */
    uint32_t m_size {0};

    /**
     * @brief Output parameter flag.
     */
    bool m_output {false};

    /**
     * @brief Constructor.
     * @param isOutput          Output parameter flag.
     */
    explicit QueryParameterBinding(bool isOutput)
    {
        reset(isOutput);
    }

    /**
     *  Resets the binding information.
     * @param isOutput          Output parameter flag.
     */
    void reset(bool isOutput);

    /**
     * @brief Set binding to output.
     */
    void setOutput()
    {
        m_output = true;
    }
};

/**
 * @}
 */
} // namespace sptk
