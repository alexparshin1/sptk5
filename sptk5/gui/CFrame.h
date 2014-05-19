/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CFrame.h  -  description
                             -------------------
    begin                : Thu Sep 14 2006
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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
#ifndef __CFRAME_H__
#define __CFRAME_H__

#include <sptk5/cxml>
#include <sptk5/gui/CPngImage.h>
#include <sptk5/CTar.h>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// @brief Frame information class
///
/// Contains the frame image and frame width

class CFrame : public CPngImage
{
public:
    /// @brief An expected usage of the frame

    enum CFrameKind
    {
        FLTK_STANDARD, ///< The frame should be used to draw standard FLTK frames
        USER_EXTENDED ///< The frame should be used to draw user frames
    };
protected:
    uint32_t m_frameWidth; ///< Frame width, to keep widgets inside the frame w/o overlaping
    uint32_t m_cornerZone; ///< Corner zone width to draw the frame corners without changes
    CPatternDrawMode m_drawMode; ///< Pattern draw mode
    CFrameKind m_kind; ///< Frame kind (standard or user-extended)
public:
    /// @brief Constructor
    /// @param imageData const CBuffer&, an image data presented as memory buffer
    /// @param frameWidth uint32_t, frame width, to keep widgets inside the frame w/o overlaping
    /// @param cornerZone uint32_t, corner zone width to draw the frame corners without changes
    /// @param drawMode CPatternDrawMode, pattern draw mode
    /// @param kind CFrameKind, frame kind (standard or user-extended)

    CFrame(const CBuffer& imageData, uint32_t frameWidth, uint32_t cornerZone, CPatternDrawMode drawMode = CPngImage::PDM_STRETCH, CFrameKind kind = USER_EXTENDED)
    : CPngImage(imageData)
    {
        m_frameWidth = frameWidth;
        m_cornerZone = cornerZone;
        m_kind = kind;
        m_drawMode = drawMode;
    }

    /// @brief Draws resized image
    ///
    /// @param x int, the x coordinate to draw image
    /// @param y int, the y coordinate to draw image
    /// @param w int, the width to draw image
    /// @param h int, the height to draw image
    /// @param drawBackground bool, if true then the internal area of the image is used for background

    void drawResized(int x, int y, int w, int h, bool drawBackground)
    {
        CPngImage::drawResized(x, y, w, h, (int) m_cornerZone, m_drawMode, drawBackground);
    }

    /// @brief Returns frame width, to keep widgets inside the frame w/o overlaping

    uint32_t frameWidth() const
    {
        return m_frameWidth;
    }

    /// @brief Returns corner zone width to draw the frame corners without changes

    uint32_t cornerZone() const
    {
        return m_cornerZone;
    }

    /// @brief Returns an expected usage of the frame

    CFrameKind kind() const
    {
        return m_kind;
    }
};

/// @brief Frame images collection

class CFrames
{
    typedef std::map<std::string, CFrame*> CFrameMap; ///< String (frame name) to frame map
    typedef std::map<Fl_Boxtype, CFrame*> CFltkFrameMap; ///< Box type to frame map

    CFrameMap m_frames; ///< All the registered frames
    CFltkFrameMap m_fltkFrames; ///< All the frames that are FLTK standard frames replacement

    static const CStrings frameTypeNames;
    static const Fl_Boxtype frameTypes[4];

public:
    /// @brief Constructor

    CFrames()
    {
    }

    /// @brief Destructor

    ~CFrames()
    {
        clear();
    }

    /// @brief Clears the frame collection
    ///
    /// All the memory allocated for images is deleted
    void clear();

    /// @brief Loads the frames from the tar archive by the XML description
    /// @param tar CTar&, tar archive with the images
    /// @param frameNode CXmlNode*, XML description of the frames
    void load(CTar& tar, CXmlNode* frameNode);

    /// @brief Registers a single frame image in the collection
    /// @param frameName std::string, symbolic name for the frame
    /// @param frame CFrame*, a frame image
    /// @param frameType Fl_Boxtype, FLTK frame type if applicable
    void registerFrame(std::string frameName, CFrame* frame, Fl_Boxtype frameType = FL_NO_BOX);

    /// @brief Returns a standard FLTK frame image, or NULL if not defined in the collection
    /// @param frameType Fl_Boxtype, standard FLTK frame type to find
    CFrame* find(Fl_Boxtype frameType) const;

    /// @brief Returns a frame image, or NULL if not defined in the collection
    /// @param frameName std::string, symbolic frame name
    CFrame* find(std::string frameName) const;
};
/// @}
}

#endif
