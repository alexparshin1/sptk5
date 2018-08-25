/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       sptk.h - description                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __SPTK_H__
#define __SPTK_H__ ///< sptk.h

#if defined(__GNUC__) || defined(__SUNPRO_CC)
    /**
     * Unix compiler flag
     */
    #define __UNIX_COMPILER__

#endif

#ifndef __UNIX_COMPILER__
#  if defined(SP_DLL) && defined(WIN32)
#    ifdef SP_LIBRARY
#      define SP_EXPORT   __declspec(dllexport)
#    else
#      define SP_EXPORT   __declspec(dllimport)
#    endif
#  else
#    define SP_EXPORT
#  endif
#else
#  define SP_EXPORT     ///< DLL/SO classes load attributes
#endif

#ifndef __UNIX_COMPILER__
#  if defined(WIN32)
#    ifdef SP_DRIVER_LIBRARY
#      define SP_DRIVER_EXPORT   __declspec(dllexport)
#    else
#      define SP_DRIVER_EXPORT   __declspec(dllimport)
#    endif
#  else
#    define SP_DRIVER_EXPORT
#  endif
#else
#  define SP_DRIVER_EXPORT  ///< DLL/SO driver classes load attributes
#endif

#include <sptk5/sptk-config.h>
#if defined(_MSC_VER) || defined(__BORLANDC__)
    #include <winsock2.h>
    #include <windows.h>
    #include <process.h>
    #pragma warning (disable: 4251)
    #pragma warning (disable: 4290)
    #pragma warning (disable: 4355)
    #pragma warning (disable: 4786)
    #pragma warning (disable: 4996)
    #if defined(_MSC_VER)
        #define STRING_NPOS -1
    #endif
#else
    #include <unistd.h>
    #include <errno.h>
#endif

#ifndef STRING_NPOS
    /**
     * Definition for string::npos missing in some compilers
     */
    #define STRING_NPOS string::npos

#endif

#include <map>
#include <string>
#include <vector>

namespace sptk {

#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2
#define ALIGN_CENTER 3

}

#ifdef __UNIX_COMPILER__
    #include <inttypes.h>
    #include <stdint.h>

#elif __BORLANDC__
    #include <stdint.h>
    #include <ctype.h>

#elif defined(_MSC_VER)
    #if _MSC_VER >= 1800
        // Visual Studio 2013 and up
        #include <stdint.h>
        #include <inttypes.h>
    #else
        // Visual Studio 2012 and before
        #include <stdint.h>
        #include <inttypes.h>
    #endif
#else
    #include <stdint.h>
    #include <inttypes.h>

#endif

#ifdef _WIN32 
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
#endif

#endif

