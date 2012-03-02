/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          cexception.cpp  -  description
                             -------------------
    begin                : Thu Apr 27 2000
    copyright            : (C) 2000-2011 by Alexey Parshin. All rights reserved.
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

#ifndef __SYSTEMEXCEPTION_H__
#define __SYSTEMEXCEPTION_H__

#include <sptk5/CException.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief SPTK OS operation exception
///
/// Retrieves information about OS error after failed OS operation.
class SP_EXPORT CSystemException : public CException
{
public:
    /// @brief Constructor
    /// @param text std::string, the exception context
    /// @param file std::string, the file name where exception occurs
    /// @param line int, the line number in the file where exception occurs
    /// @param description std::string, the optional description information
    CSystemException(std::string context, std::string file="", int line=0);

    /// @brief Returns OS error
    static std::string osError();
};

/// @}
}

#endif
