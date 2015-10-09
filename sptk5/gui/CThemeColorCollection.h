/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CThemeColorCollection.h  -  description
                             -------------------
    begin                : Sat Jul 03 2008
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

#ifndef __CTHEMECOLORCOLLECTION_H__
#define __CTHEMECOLORCOLLECTION_H__

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <sptk5/cxml>
#include <sptk5/gui/CThemeImageState.h>
#include <map>
#include <string>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

enum CThemeColorIndex {
    THM_FOREGROUND_COLOR,   ///< Foreground color index
    THM_BACKGROUND_COLOR,   ///< Background color index
    THM_BASE_COLOR,         ///< Base color index
    THM_TEXT_COLOR          ///< Text color index
};

#define THM_MAX_COLOR_INDEX 4

enum CThemeColorState
{
    THM_COLOR_UNDEFINED=-1, ///< Color is undefined
    THM_COLOR_NORMAL=0,     ///< Normal color
    THM_COLOR_PRELIGHT,     ///< Prelight color
    THM_COLOR_SELECTED,     ///< Selection color
    THM_COLOR_ACTIVE,       ///< Active (focused) color
    THM_COLOR_INSENSITIVE   ///< Intensive color
};

#define THM_MAX_COLOR_STATE 5

typedef Fl_Color (*gtk_color_function)(std::string expression);

class CThemeColorCollection 
{
    static std::map<std::string,gtk_color_function>* m_gtkColorFunctionMap;
    static std::map<std::string,Fl_Color> m_colorMap;
    Fl_Color    m_colors[THM_MAX_COLOR_INDEX][MAX_IMAGE_STATES];
    static Fl_Color gtkColorFunction(std::string expression);
    void loadColor(CXmlNode* colorNode,CThemeColorIndex colorIndex);
    void loadColorMap(CXmlDoc& gtkTheme,std::string colorMapXPath);

    static Fl_Color passby(std::string expression);
    static Fl_Color lighter(std::string expression);
    static Fl_Color darker(std::string expression);
    static Fl_Color shade(std::string expression);
    static Fl_Color mix(std::string expression);

public:
    /// @brief Constructor
    CThemeColorCollection();

    /// @brief Loads them from SPTK theme definition
    void loadFromSptkTheme(CXmlDoc& gtkTheme);

    /// @brief Loads them from GTK theme definition
    void loadFromGtkTheme(CXmlDoc& gtkTheme);

    /// @brief Returns normal color
    Fl_Color color(CThemeColorIndex colorIndex,CThemeColorState state) const { return m_colors[colorIndex][state]; }

    /// @brief Returns foreground color
    Fl_Color fgColor(CThemeColorState state) const { return m_colors[THM_FOREGROUND_COLOR][state]; }

    /// @brief Returns background color
    Fl_Color bgColor(CThemeColorState state) const { return m_colors[THM_BACKGROUND_COLOR][state]; }

    /// @brief Returns base color
    Fl_Color baseColor(CThemeColorState state) const { return m_colors[THM_BASE_COLOR][state]; }

    /// @brief Returns text color
    Fl_Color textColor(CThemeColorState state) const { return m_colors[THM_TEXT_COLOR][state]; }
};

/// @}
}

#endif
