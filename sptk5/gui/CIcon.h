/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CIcon.h  -  description
                             -------------------
    begin                : Sun Aug 20 2006
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CICON_H__
#define __CICON_H__

#include <sptk5/gui/CPngImage.h>
#include <sptk5/CCaseInsensitiveCompare.h>
#include <sptk5/CTar.h>
#include <sptk5/cxml>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// @brief Icon size variations
enum CIconSize
{
    IS_COMBO_ICON,  ///< Combo box icon, 12x12
    IS_SMALL_ICON,  ///< Small button icon, 16x16
    IS_LARGE_ICON,  ///< Large button icon, 22x22
    IS_DIALOG_ICON  ///< Dialog icon, 48x48
};

/// @brief Class CIcon provides a storage of PNG image
class CIcon
{
    CPngImage*  m_image;  ///< Icon image
    std::string m_name;   ///< Icon image symbolic name
public:
    bool        m_shared; ///< Is this icon shared (stock) icon
public:
    /// @brief Constructor
    CIcon(std::string name = "", CPngImage* image = 0L, bool shared = true) :
        m_image(image),
        m_name(name),
        m_shared(shared)
    {
    }

    /// @brief Destructor
    ~CIcon()
    {
        if (m_image && !m_shared)
            delete m_image;
    }

    /// @brief Loads a PNG image from buffer
    /// @param imageData const CBuffer&, an image data to load PNG image from
    void load(const CBuffer& imageData);

    /// @brief Returns an icon image
    CPngImage* image()
    {
        return m_image;
    }

    /// @brief Returns an icon symbolic name
    const std::string name() const
    {
        return m_name;
    }

    /// @brief Sets the image pointer
    /// @param img CPngImage*, an image pointer
    /// @param shared bool, if false then memory should be maintained by the class
    void image(CPngImage* img, bool shared)
    {
        if (m_image && !m_shared)
            delete m_image;
        m_image = img;
        m_shared = shared;
    }
};

/// @brief A map of theme icon names to theme icons
class CIconMap: public std::map<std::string, CIcon*, CCaseInsensitiveCompare>
{
    bool m_shared;
public:
    /// @brief Default constructor
    /// @param shared bool, if true then memory, allocated for the icons, isn't managed
    CIconMap(bool shared = false)
    {
        m_shared = shared;
    }

    /// @brief Destructor
    ~CIconMap()
    {
        clear();
    }

    /// @brief Removes all the allocated images
    void clear();

    /// @brief Inserts a new icon into icon map
    /// @param icon CIcon*, a pointer to an icon
    void insert(CIcon* icon);

    /// @brief Loads a collection of images using information from XML node
    ///
    /// Any incorrect image information is ignored.
    /// @param tar CTar&, the loaded tar archive where images are located
    /// @param node CXmlNode*, an XML node describing the theme icon images
    void load(CTar& tar, CXmlNode* node);
};

/// @}
}
#endif
