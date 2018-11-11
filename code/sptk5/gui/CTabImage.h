/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTabImage.h - description                              ║
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

#ifndef __CTABIMAGE_H__
#define __CTABIMAGE_H__

#include <sptk5/gui/CPngImage.h>
#include <sptk5/Tar.h>
#include <sptk5/cxml>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Typee of the tab image
 */
enum CTabImageIndex {
    TI_INACTIVE=0,
    TI_ACTIVE=1,
    TI_BORDER=2
};

/**
 * @brief An image representing one of three required images for theme tabs
 */
class CTabImage {
    /**
     * Tab image
     */
    CPngImage*  m_image;

    /**
     * Logical name
     */
    String      m_name;

    /**
     * Left frame width
     */
    int         m_leftFrameWidth;

    /**
     * Right frame width
     */
    int         m_rightFrameWidth;

    /**
     * Top frame height
     */
    int         m_topFrameHeight;

    /**
     * Bottom frame height
     */
    int         m_bottomFrameHeight;

    /**
     * Background draw mode
     */
    CPngImage::CPatternDrawMode m_backgroundDrawMode;

public:
    /**
     * @brief Constructor
     * @param tar const sptk::CTar&, the theme tar archive
     * @param tabImageNode const sptk::xml::Node*, an XML node with the tab theme info
     */
    CTabImage(const sptk::Tar& tar,const sptk::xml::Node* tabImageNode);

    /**
     * @brief Destructor
     */
    ~CTabImage() {
        if (m_image) delete m_image;
    }

    /**
     * @brief Custom draw method
     */
    void draw(int x,int y,int w,int h);

    /**
     * @brief Tab name
     */
    String name() const { return m_name; }
};

/**
 * Tab theme images collection
 */
class CTabImages : protected std::map<String,CTabImage*> {
public:
    /**
     * @brief Constructor
     */
    CTabImages() {}

    /**
     * @brief Loads tab images from tar archive using XML node as an index
     * @param tar const sptk::CTar&, the theme tar archive
     * @param tabImagesNode const sptk::xml::Node*, an XML node with the tabs theme info
     */
    void load(const sptk::Tar& tar,const sptk::xml::Node* tabImagesNode);

    /**
     * @brief Destructor
     */
    ~CTabImages() { clear(); }

    /**
     * @brief Removes all the tab images from the vector
     */
    void clear();

    /**
     * @brief Returns tab image for the name, or NULL if not found
     */
    CTabImage* tabImage(const char* imageName);
};

/**
 * @}
 */
}
#endif
