/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPngImage.h - description                              ║
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

#ifndef __CPNGIMAGE_H__
#define __CPNGIMAGE_H__

#include <FL/Fl.H>
#include <FL/Fl_Image.H>
#include <sptk5/Buffer.h>

namespace sptk {

/**
 * @brief For the borders, border size indices
 */
enum CBorderIndex
{
    BORDER_LEFT, BORDER_RIGHT, BORDER_TOP, BORDER_BOTTOM
};

/**
 * @brief PNG image that is initialized from the memory buffer
 *
 * This class is used by SPTK themes
 */
class CPngImage : public Fl_RGB_Image
{

    friend class CIcon;

    typedef void (* resized_draw_function)(CPngImage* image, int srcX, int srcY, int srcW, int srcH, int destX,
                                           int destY, int destW, int destH);

protected:

    /**
     * @brief Internal loader from PNG image buffer
     * @param buffer const CBuffer&, image data
     */
    void load(const Buffer& buffer);

    /**
     * @brief Draws a part of the image by cutting a corresponding source and tiling it in required area
     * @param image CPngImage*, source image
     * @param srcX int, x-coordinate of the source fragment
     * @param srcY int, y-coordinate of the source fragment
     * @param srcW int, width of the source fragment
     * @param srcH int, height of the source fragment
     * @param destX int, x-coordinate of the destination fragment
     * @param destY int, y-coordinate of the destination fragment
     * @param destW int, width of the destination fragment
     * @param destH int, height of the destination fragment
     */
    static void cutTileDraw(CPngImage* image, int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW,
                            int destH);

    /**
     * @brief Draws a part of the image by cutting a corresponding source and stretching it in required area
     * @param image CPngImage*, source image
     * @param srcX int, x-coordinate of the source fragment
     * @param srcY int, y-coordinate of the source fragment
     * @param srcW int, width of the source fragment
     * @param srcH int, height of the source fragment
     * @param destX int, x-coordinate of the destination fragment
     * @param destY int, y-coordinate of the destination fragment
     * @param destW int, width of the destination fragment
     * @param destH int, height of the destination fragment
     */
    static void cutStretchDraw(CPngImage* image, int srcX, int srcY, int srcW, int srcH, int destX, int destY,
                               int destW, int destH);

public:
    /**
     * @brief The way how the resized parts of image are drawn
     *
     * By resized parts I mean everything besides four corners.
     * Corners are rectangles with the side of corner zone
     */
    enum CPatternDrawMode
    {
        /**
         * Undefined and not drawn, should be defined later.
         */
                PDM_UNDEFINED,

        /**
         * The resized parts are tiled
         */
                PDM_TILE,

        /**
         * The resized parts are stretched
         */
                PDM_STRETCH

    };

    /**
     * @brief Draws a part of the image by cutting a corresponding source and tiling it in required area
     * @param srcX int, x-coordinate of the source fragment
     * @param srcY int, y-coordinate of the source fragment
     * @param srcW int, width of the source fragment
     * @param srcH int, height of the source fragment
     * @param destX int, x-coordinate of the destination fragment
     * @param destY int, y-coordinate of the destination fragment
     * @param destW int, width of the destination fragment
     * @param destH int, height of the destination fragment
     */
    void cutTileDraw(int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH)
    {
        cutTileDraw(this, srcX, srcY, srcW, srcH, destX, destY, destW, destH);
    }

    /**
     * @brief Draws a part of the image by cutting a corresponding source and stretching it in required area
     * @param srcX int, x-coordinate of the source fragment
     * @param srcY int, y-coordinate of the source fragment
     * @param srcW int, width of the source fragment
     * @param srcH int, height of the source fragment
     * @param destX int, x-coordinate of the destination fragment
     * @param destY int, y-coordinate of the destination fragment
     * @param destW int, width of the destination fragment
     * @param destH int, height of the destination fragment
     */
    void cutStretchDraw(int srcX, int srcY, int srcW, int srcH, int destX, int destY, int destW, int destH)
    {
        cutStretchDraw(this, srcX, srcY, srcW, srcH, destX, destY, destW, destH);
    }

public:
    /**
     * @brief Constructor
     *
     * An object of the class contains it's own copy of the image data
     * @param imagedata const CBuffer&, PNG image data in memory buffer
     */
    explicit CPngImage(const Buffer& imagedata);

    /**
     * @brief Constructor
     *
     * An object of the class contains it's own copy of the image data
     * @param image Fl_RGB_Image*, RGB image data in memory buffer
     */
    explicit CPngImage(const Fl_RGB_Image* image);

    /**
     * @brief Constructor
     *
     * An object of the class contains it's own copy of the image data
     * @param fileName std::string, image file (.png)
     */
    explicit CPngImage(const String& fileName);

    /**
     * @brief Draws resized image
     *
     * @param x int, the x coordinate to draw image
     * @param y int, the y coordinate to draw image
     * @param w int, the width to draw image
     * @param h int, the height to draw image
     * @param cornerZone int, the height (and width) of the corners that are simply copied from the image w/o processing 
     * @param drawMode CPatternDrawMode, the mode to draw the resized parts of image
     * @param drawBackground bool, if true then the internal area of the image is used for background
     */
    void drawResized(int x, int y, int w, int h, int cornerZone, CPatternDrawMode drawMode, bool drawBackground);

    /**
     * @brief Draws resized image
     *
     * @param x int, the x coordinate to draw image
     * @param y int, the y coordinate to draw image
     * @param w int, the width to draw image
     * @param h int, the height to draw image
     * @param border int[], the borders that are copied from the image with minimal possible processing 
     * @param drawMode CPatternDrawMode, the mode to draw the resized parts of image
     * @param drawBackground bool, if true then the internal area of the image is used for background
     */
    void drawResized(int x, int y, int w, int h, int border[], CPatternDrawMode drawMode, bool drawBackground);
};

}

#endif
