/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          sptk-config.h  -  description
                             -------------------
    begin                : Sun Apr 2007
    copyright            : (C) 2000-2012 by Alexey Parshin
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

#define VERSION          "@VERSION@"
#define THEMES_PREFIX    "@THEMES_PREFIX@"

#define HAVE_FLTK        @FLTK_FLAG@
#define HAVE_ODBC        @ODBC_FLAG@
#define HAVE_SQLITE3     @SQLITE3_FLAG@
#define HAVE_POSTGRESQL  @POSTGRESQL_FLAG@
#define HAVE_ASPELL      @ASPELL_FLAG@
#define SIZEOF_INT       @INT@
#define SIZEOF_LONG      @LONG@
#define SIZEOF_SHORT     @SHORT@
#define SIZEOF_DOUBLE    @DOUBLE@
#define WORDS_BIG_ENDIAN @BIG_ENDIAN_INIT@

#define HAVE_PTHREAD_MUTEX_TIMEDLOCK    @HAVE_PTHREAD_MUTEX_TIMEDLOCK@
#define HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK @HAVE_PTHREAD_RWLOCK_TIMEDRDLOCK@
#define HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK @HAVE_PTHREAD_RWLOCK_TIMEDWRLOCK@

#endif
