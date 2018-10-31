/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        QueryParameterBinding.h - description                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __SPTK_QUERYPARAMBINDING_H__
#define __SPTK_QUERYPARAMBINDING_H__

#include <sptk5/sptk.h>
#include <sptk5/Variant.h>

#include <vector>
#include <map>

namespace sptk
{

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
    void*        m_stmt;

    /**
     * Data type
     */
    VariantType  m_dataType;

    /**
     * Buffer
     */
    void*        m_buffer;

    /**
     * Buffer size
     */
    uint32_t     m_size;

    /**
     * Output parameter flag
     */
    bool         m_output;

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
    bool check(void* stmt, VariantType type, uint32_t size, void* buffer);
};

/**
 * @}
 */
}

#endif
