/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin                             ║
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

constexpr const char* VERSION = "@VERSION@";
constexpr const char* THEMES_PREFIX = "@THEMES_PREFIX@";

@FLTK_DEFINE@
@ODBC_DEFINE@
@MSSQL_TESTS_DEFINE@
@SQLITE3_DEFINE@
@POSTGRESQL_DEFINE@
@FIREBIRD_DEFINE@
@EPOLL_DEFINE@

@MYSQL_DEFINE@
@MARIADB_DEFINE@
@MYSQL_HAS_MY_BOOL_DEFINE@

@ORACLE_DEFINE@
@ORACLE_OCI_DEFINE@
@ASPELL_DEFINE@
@PCRE_DEFINE@
@PCRE2_DEFINE@
@OPENSSL_DEFINE@
@ZLIB_DEFINE@
@BROTLI_DEFINE@

@NEW_ABI_DEFINE@
@GTEST_DEFINE@

constexpr const char* TEST_DIRECTORY = "@TEST_DIRECTORY@";    ///< Directory that contains data, used in unit tests

@BUILD_TEST_WS_DEFINE@
