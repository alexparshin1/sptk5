/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSystemException.cpp  -  description
                             -------------------
    begin                : Thu Apr 27 2000
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CSystemException.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;
using namespace sptk;

CSystemException::CSystemException(string context, std::string file, int line)
: CException(context + ": " + osError(), file, line)
{
}

CSystemException::CSystemException(const CSystemException& other)
: CException(other)
{
}

CSystemException::~CSystemException() DOESNT_THROW
{
}

string CSystemException::osError()
{
#ifdef WIN32
    // Get Windows last error
    LPCTSTR lpMsgBuf = NULL;
    DWORD dw = GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
    return lpMsgBuf;
#else
    // Get Unix errno-based error
    char buffer[256];
    buffer[0] = 0;
    const char* osError = (const char*) strerror_r(errno, buffer, sizeof(buffer));
    if (buffer[0])
        osError = buffer;
    return osError;
#endif
}
