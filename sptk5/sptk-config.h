/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          sptk-config.h  -  description
                             -------------------
    begin                : Sun Apr 2007
    copyright            : (C) 2005-2007 by Alexey Parshin
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

#ifndef __SPTK_CONFIG_H__
#define __SPTK_CONFIG_H__

#define VERSION          "4.99.01"
#define THEMES_PREFIX    "/usr/local"

#define HAVE_FLTK        0
#define HAVE_ODBC        1
#define HAVE_SQLITE3     0
#define HAVE_POSTGRESQL  0
#define HAVE_ASPELL      0
#define SIZEOF_INT       4
#define SIZEOF_LONG      4
#define SIZEOF_SHORT     2
#define SIZEOF_DOUBLE    8
#define WORDS_BIG_ENDIAN 0

#define HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK 0
#define HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK 0

#endif
