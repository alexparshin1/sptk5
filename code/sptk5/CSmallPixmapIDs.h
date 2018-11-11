/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CSmallPixmapIDs.h - description                        ║
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

#ifndef __CSMALLPIXMAPIDS_H__
#define __CSMALLPIXMAPIDS_H__

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * A list of predefined small pixmaps
 */
enum CSmallPixmapType {
    /**
     * Small pixmap - executable
     */
    SXPM_EXECUTABLE,

    /**
     * Small pixmap - folder
     */
    SXPM_FOLDER,

    /**
     * Small pixmap - opened folder
     */
    SXPM_FOLDER_OPENED,

    /**
     * Small pixmap - document
     */
    SXPM_DOCUMENT,

    /**
     * Small pixmap - text document
     */
    SXPM_TXT_DOCUMENT,

    /**
     * Small pixmap - Excel document
     */
    SXPM_XLS_DOCUMENT,

    /**
     * Small pixmap - Word document
     */
    SXPM_DOC_DOCUMENT

};

/**
 * @brief A list of predefined icon names used by ListView, primarily
 */
extern const char* smallPixmapIDs[7];

/**
 * @}
 */
}

#endif
