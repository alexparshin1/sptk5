/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTabImage.cpp - description                            ║
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

#include <sptk5/sptk.h>

#include <FL/fl_draw.H>
#include <sptk5/gui/CTabImage.h>

using namespace std;
using namespace sptk;

CTabImage::CTabImage(const Tar& tar, const xml::Node* tabImageNode)
{
    m_name = tabImageNode->getAttribute("name").str();
    string fileName = tabImageNode->getAttribute("image");
    m_image = new CPngImage(tar.file(fileName));
    m_leftFrameWidth = tabImageNode->getAttribute("left_frame", "0");
    m_rightFrameWidth = tabImageNode->getAttribute("right_frame", "0");
    m_topFrameHeight = tabImageNode->getAttribute("top_frame", "0");
    m_bottomFrameHeight = tabImageNode->getAttribute("bottom_frame", "0");
    if (tabImageNode->getAttribute("fill").str() == "stretch")
        m_backgroundDrawMode = CPngImage::PDM_STRETCH;
    else
        m_backgroundDrawMode = CPngImage::PDM_TILE;
}

void CTabImage::draw(int x, int y, int w, int h)
{
    /// Top left corner
    m_image->draw(x, y, m_leftFrameWidth, m_topFrameHeight, 0, 0);

    /// Bottom left corner
    m_image->draw(x, y + h - m_bottomFrameHeight, m_leftFrameWidth, m_bottomFrameHeight, 0,
                  m_image->h() - m_bottomFrameHeight);

    /// Top right corner
    m_image->draw(x + w - m_rightFrameWidth, y, m_rightFrameWidth, m_topFrameHeight, m_image->w() - m_rightFrameWidth,
                  0);

    /// Bottom right corner
    m_image->draw(x + w - m_rightFrameWidth, y + h - m_bottomFrameHeight, m_rightFrameWidth, m_bottomFrameHeight,
                  m_image->w() - m_rightFrameWidth, m_image->h() - m_bottomFrameHeight);

    /// Left side
    m_image->cutStretchDraw(0, m_topFrameHeight, m_leftFrameWidth,
                            m_image->h() - (m_topFrameHeight + m_bottomFrameHeight),
                            x, y + m_topFrameHeight, m_leftFrameWidth, h - (m_topFrameHeight + m_bottomFrameHeight));

    /// Top side
    m_image->cutStretchDraw(m_leftFrameWidth, 0, m_image->w() - (m_leftFrameWidth + m_rightFrameWidth),
                            m_topFrameHeight,
                            x + m_leftFrameWidth, y, w - (m_leftFrameWidth + m_rightFrameWidth), m_topFrameHeight);

    /// Right side
    m_image->cutStretchDraw(m_image->w() - m_leftFrameWidth, m_topFrameHeight, m_leftFrameWidth,
                            m_image->h() - (m_topFrameHeight + m_bottomFrameHeight),
                            x + w - m_leftFrameWidth, y + m_topFrameHeight, m_leftFrameWidth,
                            h - (m_topFrameHeight + m_bottomFrameHeight));

    /// Bottom side
    m_image->cutStretchDraw(m_leftFrameWidth, m_image->h() - m_topFrameHeight,
                            m_image->w() - (m_leftFrameWidth + m_rightFrameWidth), m_topFrameHeight,
                            x + m_leftFrameWidth, y + h - m_topFrameHeight, w - (m_leftFrameWidth + m_rightFrameWidth),
                            m_topFrameHeight);

    /// Background
    m_image->cutStretchDraw(m_leftFrameWidth, m_topFrameHeight, m_image->w() - (m_leftFrameWidth + m_rightFrameWidth),
                            m_image->h() - (m_topFrameHeight + m_bottomFrameHeight),
                            x + m_leftFrameWidth, y + m_topFrameHeight, w - (m_leftFrameWidth + m_rightFrameWidth),
                            h - (m_topFrameHeight + m_bottomFrameHeight));
/*
    if (drawMode == PDM_STRETCH) {
        cutStretchDraw(cornerSizeX,0,w()-cornerSizeX*2,cornerSizeY,xx+cornerSizeX,yy,ww-cornerSizeX*2,cornerSizeY);
        cutStretchDraw(cornerSizeX,h()-cornerSizeY,w()-cornerSizeX*2,cornerSizeY,xx+cornerSizeX,yy+hh-cornerSizeY,ww-cornerSizeX*2,cornerSizeY);
        cutStretchDraw(0,cornerSizeY,cornerSizeX,h()-cornerSizeY*2,xx,yy+cornerSizeY,cornerSizeX,hh-cornerSizeY*2);
        cutStretchDraw(w()-cornerSizeX,cornerSizeY,cornerSizeX,h()-cornerSizeY*2,xx+ww-cornerSizeX,yy+cornerSizeY,cornerSizeX,hh-cornerSizeY*2);
        if (drawBackground)
            cutStretchDraw(cornerSizeX,cornerSizeY,w()-cornerSizeX*2,h()-cornerSizeY*2,xx+cornerSizeX,yy+cornerSizeY,ww-cornerSizeX*2,hh-cornerSizeY*2);
    } else {
        cutTileDraw(cornerSizeX,0,w()-cornerSizeX*2,cornerSizeY,xx+cornerSizeX,yy,ww-cornerSizeX*2,cornerSizeY);
        cutTileDraw(cornerSizeX,h()-cornerSizeY,w()-cornerSizeX*2,cornerSizeY,xx+cornerSizeX,yy+hh-cornerSizeY,ww-cornerSizeX*2,cornerSizeY);
        cutTileDraw(0,cornerSizeY,cornerSizeX,h()-cornerSizeY*2,xx,yy+cornerSizeY,cornerSizeX,hh-cornerSizeY*2);
        cutTileDraw(w()-cornerSizeX,cornerSizeY,cornerSizeX,h()-cornerSizeY*2,xx+ww-cornerSizeX,yy+cornerSizeY,cornerSizeX,hh-cornerSizeY*2);
        if (drawBackground)
            cutTileDraw(cornerSizeX,cornerSizeY,w()-cornerSizeX*2,h()-cornerSizeY*2,xx+cornerSizeX,yy+cornerSizeY,ww-cornerSizeX*2,hh-cornerSizeY*2);
    }
    */
}

void CTabImages::load(const Tar& tar, const xml::Node* tabImagesNode)
{
    clear();
    for (auto tabNode: *tabImagesNode) {
        auto tabImage = new CTabImage(tar, tabNode);
        (*this)[tabImage->name()] = tabImage;
    }
}

void CTabImages::clear()
{
    for (auto itor: *this)
        delete itor.second;
    map<String, CTabImage*>::clear();
}

CTabImage* CTabImages::tabImage(const char* imageName)
{
    auto itor = find(imageName);
    if (itor == end())
        return nullptr;
    return itor->second;
}
