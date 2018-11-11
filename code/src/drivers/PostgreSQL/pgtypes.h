/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       pgtypes.h - description                                ║
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

#ifndef __PGTYPES_H__
#define __PGTYPES_H__

/// @brief PostgreSQL native data types
enum PG_DATA_TYPE {
    PG_BOOL = 16,               ///< Boolean
    PG_BYTEA = 17,              ///< Byte array
    PG_BYTE = 18,               ///< Single byte
    PG_NAME = 19,               ///< String name
    PG_INT8 = 20,               ///< 64-bit integer
    PG_INT2 = 21,               ///< 16-bit integer
    PG_INT4 = 23,               ///< 32-bit integer
    PG_TEXT = 25,               ///< Text string
    PG_OID = 26,                ///< Object id (32-bit integer)
    PG_FLOAT4 = 700,            ///< 32-bit float
    PG_FLOAT8 = 701,            ///< 64-bit float (double)
    PG_CHAR_ARRAY = 1002,       ///< 16-bit integer vector
    PG_INT2_VECTOR = 1005,      ///< 16-bit integer vector
    PG_INT2_ARRAY = 1006,       ///< 16-bit integer array
    PG_INT4_ARRAY = 1007,       ///< 32-bit integer vector
    PG_TEXT_ARRAY = 1009,       ///< text array
    PG_VARCHAR_ARRAY = 1015,    ///< text array
    PG_INT8_ARRAY = 1016,       ///< 64-bit integer vector
    PG_FLOAT4_ARRAY = 1021,     ///< float array
    PG_FLOAT8_ARRAY = 1022,     ///< double array
    PG_CHAR = 1042,             ///< String
    PG_VARCHAR = 1043,          ///< String
    PG_DATE = 1082,             ///< Date
    PG_TIME = 1083,             ///< Time
    PG_TIMESTAMP = 1114,        ///< Time stamp
    PG_TIMESTAMP_ARRAY = 1115,  ///< Time stamp array
    PG_TIMESTAMPTZ = 1184,      ///< Time stamp with time zone
    PG_TIMESTAMPTZ_ARRAY = 1185,///< Time stamp with time zone array
    PG_INTERVAL = 1186,         ///< Time interval
    PG_TIMETZ = 1266,           ///< Time zone
    PG_VARBIT = 1562,           ///< Var bit
    PG_NUMERIC = 1700           ///< Numeric (decimal)
};

// This is copied from the server headers. We assume that it wouldn't change in the future..
#define VOIDOID     2278

#endif
