/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CLayoutClient.h - description                          ║
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

#ifndef __CLAYOUTCLIENT_H__
#define __CLAYOUTCLIENT_H__

#include <FL/Fl_Widget.H>
#include <string>
#include <sptk5/cxml>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Possible values for the widget layout aligns
 */
enum CLayoutAlign
{
    SP_ALIGN_NONE,        /// Do not use allignment
    SP_ALIGN_LEFT,        /// Align to the left
    SP_ALIGN_RIGHT,       /// Align to the right
    SP_ALIGN_TOP,         /// Align to the top
    SP_ALIGN_BOTTOM,      /// Align to the bottom
    SP_ALIGN_CLIENT       /// Use the whole available space
};

/**
 * @brief Layout load and save mode
 */
enum CLayoutXMLmode
{
    /**
     * Load and save only controls data
     */
            LXM_DATA = 1,

    /**
     * Load and save only controls layout
     */
            LXM_LAYOUT = 2,

    /**
     * Load and save controls data and layout
     */
            LXM_LAYOUTDATA = 3
};

/**
 * @brief Base layout manager client.
 *
 * Allows CLayoutManager objects to move and resize
 * the CLayoutClient objects in accordancy with they preferredSize() and
 * layout alignment.
 */
class CLayoutClient
{
    friend class CLayoutManager;

protected:
    /**
     * @brief The preferred layout size
     *
     * It makes different sense depending on the widget layout align.
     * For layout align SP_ALIGN_TOP or SP_ALIGN_BOTTOM it is a recomended height of the widget.
     * For layout align SP_ALIGN_LEFT or SP_ALIGN_RIGHT it is a recomended width of the widget.
     * For layout align SP_ALIGN_NONE or SP_ALIGN_CLIENT it is ignored.
     * If the widget isn't in CLayoutManager group - it is ignored.
     * If it doesn't contradict with preferred widget size it will define the final size of the widget.
     */
    int m_layoutSize;

    /**
     * The width as a result of the last call of preferredSize()
     */
    int m_lastPreferredW;

    /**
     * The width as a result of the last call of preferredSize()
     */
    int m_lastPreferredH;


    /**
     * Widget to manage
     */
    Fl_Widget* m_widget;

    /**
     * Widget name (widget id)
     */
    String m_name;

    /**
     * Widget caption storage
     */
    String m_label;

    /**
     * The layout align for the widget in CLayoutManager group
     */
    CLayoutAlign m_layoutAlign;

public:
    /**
     * @brief Constructor
     * @param widget Fl_Widget*, widget to control
     * @param layoutSize int, the size of the widget in layout. See m_layoutSize for more information.
     * @param ca CLayoutAlign, widget align in layout
     */
    CLayoutClient(Fl_Widget* widget, int layoutSize, CLayoutAlign ca);

    /**
     * @brief Destructor
     */
    virtual ~CLayoutClient()
    {}

    /**
     * @brief Returns widget's layout align
     * @see CLayoutAlign
     */
    CLayoutAlign layoutAlign() const
    {
        return m_layoutAlign;
    }

    /**
     * @brief Sets widget's layout align
     * @see CLayoutAlign
     */
    void layoutAlign(CLayoutAlign al)
    {
        m_layoutAlign = al;
    }

    /**
     * @brief Returns widget's layout size
     */
    int layoutSize() const
    {
        return m_layoutSize;
    }

    /**
     * @brief Sets widget's layout size
     */
    void layoutSize(int ls)
    {
        m_layoutSize = ls;
    }

    /**
     * @brief Sets the new label, makes an internal copy of the string
     *
     * @param lbl const char*, new label
     */
    void label(const char* lbl)
    {
        m_label = lbl;
        m_widget->label(m_label.c_str());
    }

    /**
     * @brief Sets label, makes an internal copy of the string
     * @param l const string&, new label
     */
    void label(const String& l)
    {
        m_label = l;
        m_widget->label(m_label.c_str());
    }

    /**
     * @brief Returns the current label
     */
    const String& label() const
    {
        return m_label;
    }

    /**
     * @brief Sets the new widget name
     *
     * @param aname const char*, new widget name
     */
    void name(const char* aname)
    {
        m_name = aname;
    }

    /**
     * @brief Sets the new widget name
     *
     * @param aname             New widget name
     */
    void name(const String& aname)
    {
        m_name = aname;
    }

    /**
     * @brief Returns the current name
     */
    const String& name() const
    {
        return m_name;
    }

    /**
     * @brief Computes widget's preferred size.
     *
     * Should be overriten in derived widget if it has any ideas about it's size limits.
     * Widget may want to change none, one, or both preferred width and height suggested by the
     * CLayoutManager.
     * @param w int&, input/output widget preferred width
     * @param h int&, input/output widget preferred height
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize(int& w, int& h)
    { return false; }

    /**
     * @brief Computes widget's preferred size, and stores size values internally as cache
     *
     * Used internally by CLayoutManager.
     * @param w int&, input/output widget preferred width
     * @param h int&, input/output widget preferred height
     */
    virtual bool computeSize(int& w, int& h)
    {
        bool rc = preferredSize(w, h);
        m_lastPreferredW = w;
        m_lastPreferredH = h;
        return rc;
    }

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual String className() const
    {
        return "Undefined";
    }

    /**
     * @brief Returns widget handled by that object
     */
    Fl_Widget* widget() const
    {
        return m_widget;
    }

    /**
     * @brief Loads layout client information from XML node
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    void load(const xml::Node* node, CLayoutXMLmode xmlMode);

    /**
     * @brief Saves layout client information from XML node
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    void save(xml::Node* node, CLayoutXMLmode xmlMode) const;
};
/**
 * @}
 */
}

#endif
