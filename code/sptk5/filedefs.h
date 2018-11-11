/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       filedefs.h - description                               ║
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

#ifndef __FILEDEFS_H__
#define __FILEDEFS_H__

# include <sys/stat.h>

#ifdef _WIN32

# include <windows.h>
# include <winsock2.h>
# include <io.h>
# include <direct.h>

# define S_ISLNK(m)      (false)
# define S_ISEXEC(m)     (((m) & _S_IEXEC) == _S_IEXEC)
# if !defined(__UNIX_COMPILER__) && !defined(__BORLANDC__)
#   define S_ISREG(m)      (((m) & _S_IFREG) == _S_IFREG)
#   define S_ISDIR(m)      (((m) & _S_IFDIR) == _S_IFDIR)
#   define S_ISBLK(m)      (((m) & _S_IFBLK) == _S_IFBLK)
# endif

# define lstat stat

#else

#include <unistd.h>
#include <dirent.h>

#ifndef S_ISEXEC
# define S_ISEXEC(m)  (((m) & S_IEXEC) == S_IEXEC)
#endif
#ifndef S_ISREG
# define S_ISREG(m)   (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
# define S_ISDIR(m)   (((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISBLK
# define S_ISBLK(m)   (((m) & S_IFMT) == S_IFBLK)
#endif

#endif

#endif

