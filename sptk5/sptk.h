/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          sptk.h  -  description
                             -------------------
    begin                : Mon Apr 17 2000
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __SPTK_H_
#define __SPTK_H__

#if defined(__GNUC__) || defined(__SUNPRO_CC)
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
#  define SP_EXPORT
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
    #include <sptk5/sptk-config.h.win>
    #include <winsock2.h>
    #include <windows.h>
    #include <process.h>
    #pragma warning (disable: 4290)
    #pragma warning (disable: 4355)
    #pragma warning (disable: 4786)
    #pragma warning (disable: 4996)
#else
    #include <sptk5/sptk-config.h>
    #include <unistd.h>
    #include <pthread.h>
    #include <errno.h>
#endif

#include <map>
#include <string>
#include <vector>

namespace sptk {
#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2
#define ALIGN_CENTER 3

/// Workaround for VC++ bug
typedef std::string std_string;
}

#ifdef __UNIX_COMPILER__
	#include <stdint.h>
	#include <inttypes.h>
#else
	#ifdef __BORLANDC__
		#include <stdint.h>
		#include <ctype.h>
	#else
		#define int16_t  __int16
		#define uint16_t unsigned __int16
		#define int32_t  __int32
		#define uint32_t unsigned __int32
		#define int64_t  __int64
		#define uint64_t unsigned __int64
	#endif
#endif

#endif
