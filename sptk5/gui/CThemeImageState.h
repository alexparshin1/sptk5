/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CThemeImageState.h - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CTHEMEIMAGESTATE_H__
#define	__CTHEMEIMAGESTATE_H__

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

#define THMF_NORMAL 0
#define THMF_ACTIVE 1
#define THMF_HIGHLIGHTED 2

/**
 * @brief Theme button type - defines which routine to use for which FLTK button type
 */
enum CThemeImageState { 
    THM_IMAGE_NORMAL = THMF_NORMAL, 
    THM_IMAGE_ACTIVE = THMF_ACTIVE, 
    THM_IMAGE_NORMAL_HIGHLITED = THMF_NORMAL | THMF_HIGHLIGHTED, 
    THM_IMAGE_ACTIVE_HIGHLITED = THMF_ACTIVE | THMF_HIGHLIGHTED, 
    THM_DEFAULT_FRAME };

#define MAX_IMAGE_STATES 5

/**
 * @}
 */
}

#endif	/* _CTHEMEIMAGESTATE_H */

