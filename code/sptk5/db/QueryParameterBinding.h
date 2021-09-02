/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Variant.h>
#include <sptk5/sptk.h>

#include <map>
#include <sptk5/db/PoolDatabaseConnection.h>
#include <vector>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 *  Parameter Binding descriptor
 *
 * Stores the last information on parameter binding
 */
class SP_EXPORT QueryParameterBinding
{
public:
    /**
     * Statement handle or id
     */
    StmtHandle m_stmt {nullptr};

    /**
     * Data type
     */
    VariantDataType m_dataType {VariantDataType::VAR_NONE};

    /**
     * Buffer
     */
    uint8_t* m_buffer {nullptr};

    /**
     * Buffer size
     */
    uint32_t m_size {0};

    /**
     * Output parameter flag
     */
    bool m_output {false};

    /**
     * Constructor
     * @param isOutput          Output parameter flag
     */
    explicit QueryParameterBinding(bool isOutput)
    {
        reset(isOutput);
    }

    /**
     *  Resets the binding information
     * @param isOutput          Output parameter flag
     */
    void reset(bool isOutput);

    /**
     * Set binding to output
     */
    void setOutput()
    {
        m_output = true;
    }

    /**
     * Checks if the parameter binding is matching the cached
     *
     * Returns true, if the passed parameters are matching last binding parameters.
     * Returns false and stores new parameters into last binding parameters otherwise.
     * @param stmt              Statement handle
     * @param type              Data type
     * @param size              Binding buffer size
     * @param buffer            Binding buffer
     */
    bool check(StmtHandle stmt, VariantDataType type, uint32_t size, uint8_t* buffer);
};

/**
 * @}
 */
} // namespace sptk
