/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#if defined(__GNUC__) || defined(__SUNPRO_CC)
/**
 * Unix compiler flag
 */
#define __UNIX_COMPILER__

#endif

#if defined(__FreeBSD__) || defined(__NetBSD__)
/**
 * BSD compiler flag
 */
#define __BSD__

#endif

#ifndef __UNIX_COMPILER__
#if defined(SP_DLL) && defined(WIN32)
#ifdef SP_LIBRARY
#define SP_EXPORT __declspec(dllexport)
#else
#define SP_EXPORT __declspec(dllimport)
#endif
#define WS_EXPORT __declspec(dllexport)
#else
#define SP_EXPORT
#define WS_EXPORT
#endif
#else
#define SP_EXPORT ///< DLL/SO classes load attributes
#define WS_EXPORT
#endif

#ifndef __UNIX_COMPILER__
#if defined(WIN32)
#ifdef SP_DRIVER_LIBRARY
#define SP_DRIVER_EXPORT __declspec(dllexport)
#else
#define SP_DRIVER_EXPORT __declspec(dllimport)
#endif
#else
#define SP_DRIVER_EXPORT
#endif
#else
#define SP_DRIVER_EXPORT ///< DLL/SO driver classes load attributes
#endif

#include <limits>
#include <sptk5/sptk-config.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <winsock2.h>

#include <process.h>
#include <windows.h>
#pragma warning(disable : 4251)
#pragma warning(disable : 4290)
#pragma warning(disable : 4355)
#pragma warning(disable : 4786)
#pragma warning(disable : 4996)

#ifdef min
#undef min
#undef max
#endif

#else

#include <cerrno>
#include <unistd.h>

#endif

#ifndef STRING_NPOS
/**
 * Definition for string::npos missing in some compilers
 */
#define STRING_NPOS string::npos

#endif

#include <array>
#include <cinttypes>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define strtok_r strtok_s

#define gmtime_r(a, b) gmtime_s(b, a)
#define localtime_r(a, b) localtime_s(b, a)

#ifdef min
#undef min
#endif

using ssize_t = long;

#endif

#include <filesystem>

[[maybe_unused]] constexpr int ALIGN_LEFT = 1;
[[maybe_unused]] constexpr int ALIGN_RIGHT = 2;
[[maybe_unused]] constexpr int ALIGN_CENTER = 3;
