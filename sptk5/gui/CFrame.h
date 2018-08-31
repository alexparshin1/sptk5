/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFrame.h - description                                 ║
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

#ifndef __CFRAME_H__
#define __CFRAME_H__

#include <sptk5/cxml>
#include <sptk5/gui/CPngImage.h>
#include <sptk5/Tar.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Frame information class
 *
 * Contains the frame image and frame width
 */

class CFrame : public CPngImage
{
public:
    /**
     * @brief An expected usage of the frame
     */

    enum CFrameKind
    {
        /**
         * The frame should be used to draw standard FLTK frames
         */
        FLTK_STANDARD,

        /**
         * The frame should be used to draw user frames
         */
        USER_EXTENDED

    };
protected:
    /**
     * Frame width, to keep widgets inside the frame w/o overlaping
     */
    uint32_t m_frameWidth;

    /**
     * Corner zone width to draw the frame corners without changes
     */
    uint32_t m_cornerZone;

    /**
     * Pattern draw mode
     */
    CPatternDrawMode m_drawMode;

    /**
     * Frame kind (standard or user-extended)
     */
    CFrameKind m_kind;

public:
    /**
     * @brief Constructor
     * @param imageData const CBuffer&, an image data presented as memory buffer
     * @param frameWidth uint32_t, frame width, to keep widgets inside the frame w/o overlaping
     * @param cornerZone uint32_t, corner zone width to draw the frame corners without changes
     * @param drawMode CPatternDrawMode, pattern draw mode
     * @param kind CFrameKind, frame kind (standard or user-extended)
     */

    CFrame(const Buffer& imageData, uint32_t frameWidth, uint32_t cornerZone, CPatternDrawMode drawMode = CPngImage::PDM_STRETCH, CFrameKind kind = USER_EXTENDED)
    : CPngImage(imageData)
    {
        m_frameWidth = frameWidth;
        m_cornerZone = cornerZone;
        m_kind = kind;
        m_drawMode = drawMode;
    }

    /**
     * @brief Draws resized image
     *
     * @param x int, the x coordinate to draw image
     * @param y int, the y coordinate to draw image
     * @param w int, the width to draw image
     * @param h int, the height to draw image
     * @param drawBackground bool, if true then the internal area of the image is used for background
     */

    void drawResized(int x, int y, int w, int h, bool drawBackground)
    {
        CPngImage::drawResized(x, y, w, h, (int) m_cornerZone, m_drawMode, drawBackground);
    }

    /**
     * @brief Returns frame width, to keep widgets inside the frame w/o overlaping
     */

    uint32_t frameWidth() const
    {
        return m_frameWidth;
    }

    /**
     * @brief Returns corner zone width to draw the frame corners without changes
     */

    uint32_t cornerZone() const
    {
        return m_cornerZone;
    }

    /**
     * @brief Returns an expected usage of the frame
     */

    CFrameKind kind() const
    {
        return m_kind;
    }
};

/**
 * @brief Frame images collection
 */

class CFrames
{
    /**
     * String (frame name) to frame map
     */
    typedef std::map<std::string, CFrame*> CFrameMap;

    /**
     * Box type to frame map
     */
    typedef std::map<Fl_Boxtype, CFrame*> CFltkFrameMap;


    /**
     * All the registered frames
     */
    CFrameMap m_frames;

    /**
     * All the frames that are FLTK standard frames replacement
     */
    CFltkFrameMap m_fltkFrames;


    static const Strings frameTypeNames;
    static const Fl_Boxtype frameTypes[4];

public:
    /**
     * @brief Constructor
     */

    CFrames() noexcept
    {
    }

    /**
     * @brief Destructor
     */

    ~CFrames()
    {
        clear();
    }

    /**
     * @brief Clears the frame collection
     *
     * All the memory allocated for images is deleted
     */
    void clear();

    /**
     * @brief Loads the frames from the tar archive by the XML description
     * @param tar CTar&, tar archive with the images
     * @param frameNode xml::Node*, XML description of the frames
     */
    void load(Tar& tar, xml::Node* frameNode);

    /**
     * @brief Registers a single frame image in the collection
     * @param frameName std::string, symbolic name for the frame
     * @param frame CFrame*, a frame image
     * @param frameType Fl_Boxtype, FLTK frame type if applicable
     */
    void registerFrame(std::string frameName, CFrame* frame, Fl_Boxtype frameType = FL_NO_BOX);

    /**
     * @brief Returns a standard FLTK frame image, or NULL if not defined in the collection
     * @param frameType Fl_Boxtype, standard FLTK frame type to find
     */
    CFrame* find(Fl_Boxtype frameType) const;

    /**
     * @brief Returns a frame image, or NULL if not defined in the collection
     * @param frameName std::string, symbolic frame name
     */
    CFrame* find(std::string frameName) const;
};
/**
 * @}
 */
}

#endif
