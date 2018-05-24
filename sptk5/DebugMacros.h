/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DebugMacros.h - description                            ║
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

#ifndef __DEBUGMACROS_H__
#define __DEBUGMACROS_H__

#ifdef DEBUG
#include <stdio.h>

#define DEBUG_MESSAGE(str) printf("%s",str); fflush(stdout);
#define DEBUG_MESSAGE_LN(str) printf("%s\n",str);
#define DEBUG_MESSAGE_FLN(str) printf("%s (%i): %s\n",__FILE__,__LINE__,str);
#define DEBUG_STRING(name,str) printf("%s: %s",name,str); fflush(stdout);
#define DEBUG_STRING_LN(name,str) printf("%s: %s\n",name,str);
#define DEBUG_INTEGER(name,value) printf("%s: %i",name,value); fflush(stdout);
#define DEBUG_INTEGER_LN(name,value) printf(" %s: %i\n",name,value);
#define DEBUG_DOUBLE(name,str) printf("%s: %0.2f",name,double(value)); fflush(stdout);
#define DEBUG_DOUBLE_LN(name,value) printf("%s: %0.2f\n",name,double(value));
#else
#define DEBUG_MESSAGE(str) ;
#define DEBUG_MESSAGE_LN(str) ;
#define DEBUG_STRING(name,str) ;
#define DEBUG_STRING_LN(name,str) ;
#define DEBUG_INTEGER(name,value) ;
#define DEBUG_INTEGER_LN(name,value) ;
#define DEBUG_DOUBLE(name,str) ;
#define DEBUG_DOUBLE_LN(name,value) ;
#endif

#endif
