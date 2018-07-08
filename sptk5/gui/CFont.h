/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFont.h - description                                  ║
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

#ifndef __CFONT_H__
#define __CFONT_H__

#include <sptk5/sptk.h>
#include <sptk5/String.h>
#include <map>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Font information
 */
class CFont
{
    /**
     * Font name
     */
    std::string m_name;

    /**
     * Font size
     */
    uint32_t    m_size;

    /**
     * Font color
     */
    uint32_t    m_color;

    /**
     * Font index in fonts collection
     */
    uint32_t    m_index;

    /**
     * Optional font attributes
     */
    uint32_t    m_attributes;

public:
    /**
     * @brief Constructor
     * @param name std::string, full font name
     * @param size uint32_t, font size
     * @param color uint32_t, font color
     * @param index uint32_t, font index in the fonts collection
     * @param attributes uint32_t, optional font attributes
     */
    CFont(const String& name="arial",uint32_t size=10,uint32_t color=0,uint32_t index=0,uint32_t attributes=0)
    {
        m_name = name;
        m_size = size;
        m_color = color;
        m_index = index;
        m_attributes = attributes;
    }

    /**
     * @brief Copy constructor
     * @param font const CFont&, font to copy from
     */
    CFont(const CFont& font)
    {
        m_name = font.m_name;
        m_size = font.m_size;
        m_color = font.m_color;
        m_index = font.m_index;
        m_attributes = font.m_attributes;
    }

    /**
     * @brief Returns font name
     */
    std::string name() const
    {
        return m_name;
    }

    /**
     * @brief Returns font size
     */
    uint32_t size() const
    {
        return m_size;
    }

    /**
     * @brief Returns font color
     */
    uint32_t color() const
    {
        return m_color;
    }

    /**
     * @brief Returns font index in the fonts collection
     */
    uint32_t index() const
    {
        return m_index;
    }

    /**
     * @brief Returns font attributes
     */
    uint32_t attributes() const
    {
        return m_attributes;
    }

    /**
     * @brief Returns unique font id
     *
     * Font id combines all the font attributes separated with "|"
     */
    std::string id() const;
};

/**
 * @brief CFontsMap is a map font names to fonts
 *
 * The map contains just pointers to the fonts stored in outside list.
 */
class CFontsMap : public std::map<std::string,CFont *>
{
public:
    /**
     * @brief Constructor
     */
    CFontsMap() {}

    /**
     * @brief Destructor
     */
    ~CFontsMap() 
    {
        clear();
    }

    /**
     * @brief Clears the fonts
     *
     * All the memory allocated for CFont objects is released
     */
    void clear();
};

/**
 * @brief CFontsVector is a map font names to fonts
 */
class CFontsVector
{
    /**
     * Fonts vector
     */
    std::vector<CFont *>    m_items;

    /**
     * Font names map
     */
    CFontsMap               m_index;

public:
    /**
     * @brief Constructor
     */
    CFontsVector() noexcept {}

    /**
     * @brief Destructor
     */
    ~CFontsVector()
    {
        clear();
    }

    /**
     * @brief Returns font by index
     */
    CFont* operator[] (uint32_t index)
    {
        return m_items[index];
    }

    /**
     * @brief Returns font by index
     */
    const CFont* operator[] (uint32_t index) const
    {
        return m_items[index];
    }

    /**
     * @brief Returns font by its name
     */
    CFont* find(std::string fontName) const;

    /**
     * @brief Appends a font to fonts vector
     * @param font CFont*, font to append
     */
    void push_back(CFont* font);

    /**
     * @returns the size of the fonts vector
     */
    uint32_t size() const
    {
        return (uint32_t) m_items.size();
    }

    /**
     * @brief Clears the fonts
     *
     * All the memory allocated for CFont objects is released
     */
    void clear();
};

/**
 * @}
 */
}

#endif
