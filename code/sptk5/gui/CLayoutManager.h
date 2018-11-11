/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CLayoutManager.h - description                         ║
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

#ifndef __CLAYOUTMANAGER_H__
#define __CLAYOUTMANAGER_H__

#include <string>
#include <map>
#include <sptk5/cxml>
#include <sptk5/gui/CFrame.h>
#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/Exception.h>

class Fl_Image;

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief A callback that creates a control of the required type
 *
 * The information (layout,style,etc) is provided by the XML node
 */
typedef CLayoutClient* (*createControlCallback)(xml::Node *node);

/**
 * @brief A callback map that stores type names and control create functions
 */
typedef std::map<std::string,createControlCallback> createControlCallbackMap;

/**
 * @brief a map of widget names to widgets
 */
typedef std::map<std::string,Fl_Widget*> CWidgetNamesMap;

/**
 * @brief Layout manager grow mode.
 *
 * Allows or disables the layout to
 * grow in horizontal and/or vertical direction
 */
enum CLayoutGrowMode {
    /**
     * Don't allow layout to grow
     */
    LGM_NONE,

    /**
     * Allow horizontal grow only
     */
    LGM_HORIZONTAL_GROW = 1,

    /**
     * Allow vertical grow only
     */
    LGM_VERTICAL_GROW = 2,

    /**
     * Allow both horizontal and vertical grow
     */
    LGM_AUTO_GROW = 3

};

/**
 * @brief Layout Manager
 *
 * Maintains the position and size of the CLayoutClient-derived
 * objects in the attached group. It uses the CLayoutClient::layoutAlign() property
 * of such objects and the output of CLayoutClient::preferredSize() to determine
 * the widget's size and position.
 * @see CLayoutClient
 */
class CLayoutManager : public CLayoutClient {
    friend class CLayoutManagerInitializer;
protected:
    /**
     * An extra space between widgets
     */
    int               m_layoutSpacing;

    /**
     * Layout grow mode, @see CLayoutGrowMode
     */
    CLayoutGrowMode   m_layoutGrowMode;

    /**
     * Attached group to manage
     */
    Fl_Group         *m_group;

    /**
     * The group frame, including optional background image
     */
    CFrame           *m_frame;

    /**
     * Should we draw background
     */
    bool              m_frameDrawBackground;

    /**
     * If true - do not read or write any XML
     */
    bool              m_noXml;


    /**
     * A map of box type names and corresponding constants
     */
    static std::map<std::string,Fl_Boxtype> m_boxTypeNames;

    /**
     * A map of control types and callbacks that create them
     */
    static createControlCallbackMap controlCreator;


    /**
     * Does the auto layout computations and optional actual widget resizing
     * @param x int, the layout x position, input
     * @param y int, the layout y position, input
     * @param w int&, the layout width, input/output
     * @param h int&, the layout height, input/output
     * @param resizeWidgets bool, resize widgets if true, or just compute the layout size
     * @returns true if the size is stable (doesn't depend on input sizes w,h)
     */
    bool autoLayout(int x,int y,int& w,int& h,bool resizeWidgets) const;

    /**
     * Paints background in tiles mode
     */
    virtual void paintBackground();
public:

    /**
     * Constructor
     * @param group Fl_Group*, a group widget to manage (CGroup, CScroll, or CWindow)
     * @param layoutSize int, the size of the widget in layout. See m_layoutSize for more information.
     * @param ca CLayoutAlign, widget align in layout
     */
    CLayoutManager(Fl_Group* group,int layoutSize,CLayoutAlign ca);

    /**
     * Destructor
     */
    virtual ~CLayoutManager() {}

    /**
     * @brief Removes all the children
     */
    virtual void clear();

    /**
     * Returns current layout spacing between widgets
     */
    int  layoutSpacing() const   {
        return m_layoutSpacing;
    }

    /**
     * Defines layout spacing between widgets
     * @param ls int, new layout spacing
     */
    void layoutSpacing(int ls)   {
        m_layoutSpacing = ls;
    }

    /**
     * Defines layout grow mode
     * @param gm CLayoutGrowMode, new layout grow mode
     * @see CLayoutGrowMode
     */
    void layoutGrowMode(CLayoutGrowMode gm=LGM_AUTO_GROW) {
        m_layoutGrowMode = gm;
    }

    /**
     * Returns current layout grow mode
     * @see CLayoutGrowMode
     */
    CLayoutGrowMode layoutGrowMode() const {
        return m_layoutGrowMode;
    }

    /**
     * Relayouts the attached group. It may be necessary if the inside widgets have changed
     * content or shown/hidden.
     */
    void relayout();

    /**
     * @brief Defines frame.
     * @param frame CFrame *, frame to use
     * @param drawBackground bool, if true then background is paint
     */
    void frame(CFrame *frame,bool drawBackground=true) {
        m_frame = frame;
        m_frameDrawBackground = drawBackground;
    }

    /**
     * @brief Returnes the current frame.
     */
    const CFrame *frame() const {
        return m_frame;
    }

    /**
     * @brief Returns frame width
     */
    uint32_t frameWidth() const;

    /**
     * @brief Loads group controls data from XML node
     *
     * @param node const xml::Node*, node to load data from
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be loaded
     */
    virtual void loadLayout(const xml::Node* node,CLayoutXMLmode xmlMode);

    /**
     * @brief Saves group controls data into XML node
     *
     * @param node const xml::Node*, node to save data into
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void saveLayout(xml::Node* node,CLayoutXMLmode xmlMode) const;

    /**
     * @brief Builds an index of internal widget names
     *
     * The index is filled in with pairs of widget name - widget.
     * Only CLayoutClient-derived widgets with names are included.
     * @param index CWidgetNamesMap&, the index to fill in
     * @param recursive bool, if true also processes the children
     * @param clean bool, if true, the list is cleaned prior to fill
     */
    void nameIndex(CWidgetNamesMap& index,bool recursive,bool clean=true) const;

    /**
     * @brief Returns flag of blocking XML processing
     *
     * If this flag is set then no XML is loaded or saved
     */
    bool noXml() const {
        return m_noXml;
    }

    /**
     * @brief Sets flag of blocking XML processing
     *
     * If this flag is set then no XML is loaded or saved.
     * This flag may be used, for instance, if you want to exclude some control
     * (widget) from being stored in XML.
     * @param noXml bool, if true - no XML is red or written
     */
    void noXml(bool noXml) {
        m_noXml = noXml;
    }

    /**
     * @brief Register control type for loading layouts
     *
     * The control name can be anything as far as in can be used as XML tag name.
     * The creator callback creates a control (widget) using the XML information
     * passed to the callback by the layout manager.
     * @param typeName std::string, the type of the (control) widget
     * @param creator createControlCallback, a callback that creates the required widget
     */
    static void registerControl(std::string typeName,createControlCallback creator) noexcept {
        controlCreator[typeName] = creator;
    }

    /**
     * @brief Returns a read-only map of box type names and corresponding constants
     */
    static const std::map<std::string,Fl_Boxtype>& boxTypeNames() {
        return m_boxTypeNames;
    }
};
/**
 * @}
 */
}
#endif
