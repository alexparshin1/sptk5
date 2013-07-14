/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CThemeImageCollection.h  -  description
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

#ifndef __CTHEMEIMAGECOLLECTION_H__
#define	__CTHEMEIMAGECOLLECTION_H__

#include <sptk5/cxml>
#include <sptk5/gui/CPngImage.h>
#include <sptk5/gui/CThemeImageState.h>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// @brief A collection of theme images
///
/// The images are stored for the different states of the object
class CThemeImageCollection {
    CPngImage*  m_images[MAX_IMAGE_STATES];         ///< List of images for different image states
    CPngImage*  m_overlayImages[MAX_IMAGE_STATES];  ///< List of overlay images for different image states
    int         m_border[4];                        ///< Border, for the first image
    bool        m_stretch;                          ///< Stretch flag, for the first image
public:
    /// @brief Default constructor
    CThemeImageCollection();
    
    /// @brief Destructor
    ~CThemeImageCollection() { clear(); }
    
    /// @brief Clears the collection
    void clear();
    
    /// @brief Loads the collection from SPTK theme
    void loadFromSptkTheme(const CStrings& objectNames);
    
    /// @brief Loads the collection from GTK theme
    void loadFromGtkTheme(CXmlDoc& gtkTheme,std::string imagesXPath,std::string attribute="",std::string attributeValue="");
    
    /// @brief Returns border for a paticular index (0..3)
    int border(int ndx) const { return m_border[ndx]; }
    
    int* border() { return m_border; }
    
    /// @brief Returns draw stretch flag
    bool stretch() const { return m_stretch; }

    /// @brief Returns an image for a particular state
    CPngImage* image(CThemeImageState state) const; 
    
    /// @brief Returns an overlay image for a particular state
    CPngImage* overlayImage(CThemeImageState state) const;

    static std::string gtkFullFileName(std::string fileName);
};

/// @}
}

#endif

