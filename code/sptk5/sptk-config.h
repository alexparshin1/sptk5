/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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

constexpr const char* VERSION = "5.6.1";
constexpr const char* THEMES_PREFIX = "/usr/local";

#define HAVE_FLTK
#define HAVE_ODBC
#define HAVE_SQLITE3
#define HAVE_POSTGRESQL

#define HAVE_EPOLL

#define HAVE_MYSQL
// MariaDB is not used
// MySQL doesn't define my_bool

// Oracle SQL is not used
#define HAVE_ORACLE_OCI
#define HAVE_ASPELL
// PCRE is not used
#define HAVE_PCRE2
#define HAVE_OPENSSL
#define HAVE_ZLIB
#define HAVE_BROTLI

#define USE_NEW_ABI
#define USE_GTEST

constexpr const char* TEST_DIRECTORY = "/home/alexeyp/workspace/sptk5/code/test";    ///< Directory that contains data, used in unit tests

#define BUILD_TEST_WS
