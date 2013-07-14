/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          ThemesUtils.cpp  -  description
                             -------------------
    begin                : Sat Jun 28 2008
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#ifndef __THEMEUTILS_H__
#define	__THEMEUTILS_H__

#include <sptk5/cgui>
#include "ThemeUtils.h"

using namespace std;

namespace sptk {

    CPngImage *loadValidatePNGImage(string fileName,bool externalFile) {
        try {
            CPngImage *img;
            if (externalFile) {
                CBuffer imageBuffer;
                imageBuffer.loadFromFile(fileName);
                img = new CPngImage(imageBuffer);
            } else {
                const CBuffer& imageBuffer = CThemes::m_tar.file(fileName);
                img = new CPngImage(imageBuffer);
            }
            if (img) {
                if (img->data())
                    return img;
                delete img;
                img = 0L;
            }
            return img;
        } catch (...) {}
        return 0;
    }

}

#endif	/* _THEMEUTILS_H */

