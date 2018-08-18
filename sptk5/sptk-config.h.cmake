/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       sptk-config.h - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_CONFIG_H__
#define __SPTK_CONFIG_H__

#define VERSION          "@VERSION@"
#define THEMES_PREFIX    "@THEMES_PREFIX@"

/// @addtogroup utility Utility Classes
/// @{

#define HAVE_FLTK        @FLTK_FLAG@        ///< Flag indicating if FLTK library is installed
#define HAVE_ODBC        @ODBC_FLAG@        ///< Flag indicating if ODBC library is installed
#define HAVE_SQLITE3     @SQLITE3_FLAG@     ///< Flag indicating if SQLite3 library is installed
#define HAVE_POSTGRESQL  @POSTGRESQL_FLAG@  ///< Flag indicating if LibPQ (PostgreSQL client) library is installed
#define HAVE_FIREBIRD    @FIREBIRD_FLAG@    ///< Flag indicating if Firebird client library is installed
#define HAVE_MYSQL       @MYSQL_FLAG@       ///< Flag indicating if MySQL client library is installed
#define HAVE_ORACLE      @ORACLE_FLAG@      ///< Flag indicating if Oracle client library is installed
#define HAVE_ASPELL      @ASPELL_FLAG@      ///< Flag indicating if ASpell library is installed
#define HAVE_PCRE        @PCRE_FLAG@        ///< Flag indicating if PCRE library is installed
#define HAVE_OPENSSL     @OPENSSL_FLAG@     ///< Flag indicating if OpenSSL library is installed
#define HAVE_ZLIB        @ZLIB_FLAG@        ///< Flag indicating if ZLib library is installed
#define WORDS_BIG_ENDIAN @BIG_ENDIAN_INIT@  ///< Flag indicating if byte order for mashine is big endian
#define BITNESS          @BITNESS@          ///< Number of machine bits

#define USE_NEW_ABI      @NEW_ABI_FLAG@     ///< Flag if new ABI is used
  #define USE_GTEST      @GTEST_FLAG@       ///< Flag indicating if GTest library is installed and used

/// @}

#endif
