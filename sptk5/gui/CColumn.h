/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CColumn.h - description                                ║
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

#ifndef __CCOLUMNS_H__
#define __CCOLUMNS_H__

#include <string>
#include <sptk5/Strings.h>
#include <sptk5/Variant.h>
#include <sptk5/cxml>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Class CColumn represents a column descriptor for CListView
 */
class SP_EXPORT CColumn
{
protected:
    /**
     * Column name
     */
    String          m_name;

    /**
     * Column width in pixels
     */
    uint32_t        m_width;

    /**
     * Column data type
     */
    VariantType     m_type;

    /**
     * Column visibility
     */
    bool            m_visible;

    /**
     * Column auto width flag
     */
    bool            m_autoWidth;

public:

    /**
     * @brief Constructor
     * @param cname             Column name
     * @param type              Column data type
     * @param cwidth            Column width, or -1 for auto-width
     * @param cvisible          Column visibility
     */
    CColumn(const std::string& cname = "", VariantType type = VAR_STRING, int32_t cwidth = 100, bool cvisible = true);

    /**
     * @brief Copy constructor
     * @param col               Source column object
     */
    CColumn(const CColumn& col);

    /**
     * @brief Returns column name
     */
    std::string name() const
    {
        return m_name;
    }

    /**
     * @brief Returns column width in pixels
     */
    int16_t width() const
    {
        return (int16_t) m_width;
    }

    /**
     * @brief Sets column width in pixels
     * @param w                 Column width in pixels
     */
    void width(int16_t w)
    {
        m_width = (uint32_t) w;
    }

    /**
     * @brief Returns column visibility
     */
    bool visible() const
    {
        return m_visible;
    }

    /**
     * @brief Sets column visibility
     * @param v                 Visibility flag
     */
    void visible(bool v)
    {
        m_visible = v;
    }

    /**
     * @brief Returns Column auto width flag
     */
    bool autoWidth() const
    {
        return m_autoWidth;
    }

    /**
     * @brief Sets column auto width flag
     * @param v                 Auto-width flag
     */
    void autoWidth(bool v)
    {
        m_autoWidth = v;
    }

    /**
     * @brief Returns column data type
     */
    VariantType type() const
    {
        return m_type;
    }

    /**
     * @brief Loads column information
     *
     * Loads information about column from the registry.
     * @param node              XML node to load column information from
     */
    void load(const xml::Node& node);

    /**
     * @brief Loads column information
     *
     * Loads information about column
     * from the registry.
     * @param node              XML node to load column information from
     */
    void load(const xml::Node* node)
    {
        load(*node);
    }

    /**
     * @brief Saves column information
     *
     * Saves information about column
     * from the registry.
     * @param node              XML node to unload column information to
     */
    void save(xml::Node& node) const;

    /**
     * @brief Saves column information
     *
     * Saves information about column
     * from the registry.
     * @param node              XML node to unload column information to
     */
    void save(xml::Node* node) const
    {
        save(*node);
    }
};

/**
 * Class CColumnList is a list of columns used in CListView
 */
class SP_EXPORT CColumnList: public std::vector<CColumn>
{
public:

    /**
     * @brief Default constructor
     */
    CColumnList()
    {
    }

    /**
     * @brief Finds the column index by column name
     * @param columnName        Column name
     */
    int indexOf(const char *columnName) const;

    /**
     * @brief Loads columns information
     *
     * Loads information about columns from the registry.
     * @param node              XML node to load columns information from
     */
    void load(const xml::Node& node);

    /**
     * @brief Loads columns information
     *
     * Loads information about columns from the registry.
     * @param node              XML node to load columns information from
     */
    void load(const xml::Node* node)
    {
        load(*node);
    }

    /**
     * @brief Saves columns information
     *
     * Saves information about columns from the registry.
     * @param node              XML node to unload columns information to
     */
    void save(xml::Node& node) const;

    /**
     * @brief Saves columns information
     *
     * Saves information about columns from the registry.
     * @param node              XML node to unload columns information to
     */
    void save(xml::Node* node) const
    {
        save(*node);
    }
};
/**
 * @}
 */
}
#endif
