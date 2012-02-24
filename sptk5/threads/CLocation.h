/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CLocation.h  -  description
                             -------------------
    begin                : Sun Jan 22 2012
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the
  Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 ***************************************************************************/

#ifndef __LOCATION_H__
#define __LOCATION_H__

#include <sptk5/sptk.h>
#include <sptk5/string_ext.h>

namespace sptk {

/// @addtogroup threads Threads Classes
/// @{

/// @brief Location object
///
/// Stores file name and line number in the file
class SP_EXPORT CLocation
{
    const char*         m_file;         ///< File name
    int                 m_line;         ///< Line number

public:
    /// @brief Constructor
    /// @param file const char*, File name
    /// @param line int, Line number
    CLocation(const char* file, int line) :
        m_file(file), m_line(line) {}

    /// @brief Modifies location
    /// @param file const char*, File name
    /// @param line int, Line number
    void set(const char* file, int line)
    {
        m_file = file;
        m_line = line;
    }

    /// @brief Returns location file name
    const char* file() const
    {
        return m_file;
    }

    /// @brief Returns location line number
    int line() const
    {
        return m_line;
    }

    /// @brief Returns string presentation of location
    std::string toString() const
    {
        return std::string(m_file) + "(" + int2string(m_line) + ")";
    }

    /// @brief Returns true if location is empty
    bool empty() const
    {
        return (m_file == NULL) && (m_line == 0);
    }
};

/// @}
}

#endif
