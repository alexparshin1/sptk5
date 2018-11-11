/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CGtkThemeLoader.h - description                        ║
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

#ifndef __CGTKTHEMELOADER_H__
#define __CGTKTHEMELOADER_H__

#include <sptk5/cutils>
#include <sptk5/cxml>

namespace sptk {

    /// @addtogroup gui GUI Classes
    /// @{

    /// @brief This class parses GTK theme configuration file into XML
    class CGtkThemeParser {
        sptk::xml::Document   m_xml;          ///< Internal XML presentation of GTK theme config
        std::string     m_themeFolder;  ///< Path to the theme folder that contains gtkrc file

        /// @brief Reads parameter/value pair from the line of text
        /// @param row const std::string&, text row
        /// @param parentNode sptk::XMLNoode* parentNode, a node to attach resulting node
        /// @returns resulting XML node
        sptk::xml::Node* parseParameter(const String& row, sptk::xml::Node* parentNode, bool createAttributes = false);
        
        /// @brief Parses a group of rows defining GTK image
        /// @param gtkrc const sptk::CStrings&, text of the GTK theme configuration file
        /// @param currentRow unsigned&, current row position in the GTK theme configuration file
        /// @param parentNode sptk::XMLNoode* parentNode, a node to attach resulting node
        void parseImage(const sptk::Strings& gtkrc, unsigned& currentRow, sptk::xml::Node* parentNode);
        
        /// @brief Parses a group of rows defining GTK engine
        /// @param gtkrc const sptk::CStrings&, text of the GTK theme configuration file
        /// @param currentRow unsigned&, current row position in the GTK theme configuration file
        /// @param parentNode sptk::XMLNoode* parentNode, a node to attach resulting node
        void parseEngine(const sptk::Strings& gtkrc, unsigned& currentRow, sptk::xml::Node* parentNode);
        
        /// @brief Parses a group of rows defining GTK style
        /// @param gtkrc const sptk::CStrings&, text of the GTK theme configuration file
        /// @param currentRow unsigned&, current row position in the GTK theme configuration file
        /// @param parentNode sptk::XMLNoode* parentNode, a node to attach resulting node
        void parseStyle(const sptk::Strings& gtkrc, unsigned& currentRow, sptk::xml::Node* parentNode);
        
        /// @brief Parses a group of rows defining GTK theme
        /// @param gtkrc const sptk::CStrings&, text of the GTK theme configuration file
        void parse(const sptk::Strings& gtkrc);
    public:

        /// @brief Default constructor
        CGtkThemeParser() = default;

        /// @brief Loads GTK theme configuration
        void load(const std::string& themeName);
        
        sptk::xml::Document& xml() { return m_xml; }
        
        std::string themeFolder() { return m_themeFolder; }
    };

    /// @}
}

#endif
