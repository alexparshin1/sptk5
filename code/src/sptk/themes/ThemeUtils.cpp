/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ThemeUtils.cpp - description                           ║
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

#ifndef __THEMEUTILS_H__
#define	__THEMEUTILS_H__

#include <sptk5/sptk.h>

#include <sptk5/cgui>

using namespace std;

namespace sptk {

    CPngImage *loadValidatePNGImage(const string& fileName, bool externalFile) {
        try {
            CPngImage *img;
            if (externalFile) {
                Buffer imageBuffer;
                imageBuffer.loadFromFile(fileName);
                img = new CPngImage(imageBuffer);
            } else {
                const Buffer& imageBuffer = CThemes::m_tar.file(fileName);
                img = new CPngImage(imageBuffer);
            }
            if (img->data())
                return img;
            delete img;
            img = nullptr;
            return img;
        } catch (...) {}
        return nullptr;
    }

}

#endif	/* _THEMEUTILS_H */

