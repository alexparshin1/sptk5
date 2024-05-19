/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       pgtypes.h - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

/// @brief PostgreSQL native data types
enum class PostgreSQLDataType
{
    BOOLEAN = 16,             ///< Boolean
    BYTEA = 17,               ///< Byte array
    BYTE = 18,                ///< Single byte
    NAME = 19,                ///< String name
    INT8 = 20,                ///< 64-bit integer
    INT2 = 21,                ///< 16-bit integer
    INT4 = 23,                ///< 32-bit integer
    TEXT = 25,                ///< Text string
    OID = 26,                 ///< Object id (32-bit integer)
    FLOAT4 = 700,             ///< 32-bit float
    FLOAT8 = 701,             ///< 64-bit float (double)
    CHAR_ARRAY = 1002,        ///< 16-bit integer vector
    INT2_VECTOR = 1005,       ///< 16-bit integer vector
    INT2_ARRAY = 1006,        ///< 16-bit integer array
    INT4_ARRAY = 1007,        ///< 32-bit integer vector
    TEXT_ARRAY = 1009,        ///< text array
    VARCHAR_ARRAY = 1015,     ///< text array
    INT8_ARRAY = 1016,        ///< 64-bit integer vector
    FLOAT4_ARRAY = 1021,      ///< float array
    FLOAT8_ARRAY = 1022,      ///< double array
    CHAR = 1042,              ///< String
    VARCHAR = 1043,           ///< String
    DATE = 1082,              ///< Date
    TIME = 1083,              ///< Time
    TIMESTAMP = 1114,         ///< Time stamp
    TIMESTAMP_ARRAY = 1115,   ///< Time stamp array
    TIMESTAMPTZ = 1184,       ///< Time stamp with time zone
    TIMESTAMPTZ_ARRAY = 1185, ///< Time stamp with time zone array
    INTERVAL = 1186,          ///< Time interval
    TIMETZ = 1266,            ///< Time zone
    VARBIT = 1562,            ///< Var bit
    NUMERIC = 1700            ///< Numeric (decimal)
};

// This is copied from the server headers. We assume that it wouldn't change in the future.
constexpr int VOIDOID = 2278;
