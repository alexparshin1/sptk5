/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTreeView.h - description                              ║
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

#ifndef __CTREEVIEW_H__
#define __CTREEVIEW_H__

#include <sptk5/gui/CPngImage.h>
#include <sptk5/gui/CTreeControl.h>
#include <sptk5/gui/CDataControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Tree view widget.
 *
 * Shows tree items with icons. Can use different icons for opened/closed folders.
 */
class SP_EXPORT CTreeView : public CControl
{

    /**
     * @brief The actual tree control
     */
    CTreeControl* m_treeControl;

    /**
     * @brief Constructor initializer
     */
    void ctor_init();

protected:

    /**
     * @brief Internal tree control callback
     */
    static void internal_callback(Fl_Widget*, void*);

public:

    /**
     * @brief Constructor in SPTK style
     * @param label             Label
     * @param layoutSize        Widget align in layout
     * @param layoutAlign       Widget align in layout
     */
    CTreeView(const char* label = 0, int layoutSize = 10, CLayoutAlign layoutAlign = SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CTreeView(int x,int y,int w,int h,const char *label=0);
#endif

    /**
     * @brief Destructor
     */
    ~CTreeView() = default;

    /**
     * @brief Returns controls' kind (internal SPTK RTTI).
     */
    CControlKind kind() const override
    {
        return DCV_TREEVIEW;
    }

    /**
     * @brief Returns controls' class name (internal SPTK RTTI).
     */
    virtual String className() const override
    {
        return "CTreeView";
    }

    /**
     * @brief Adds a child item to the item. If the closedImage parameter is omitted the openedImage is used instead.
     * @param label             The item label
     * @param openedImage       The image for the opened state
     * @param closedImage       The image for the closed state
     * @param data void         The user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem* addItem(const String& label, const Fl_Image* openedImage, const Fl_Image* closedImage = nullptr,
                       void* data = nullptr)
    {
        return m_treeControl->addItem(label.c_str(), openedImage, closedImage, data);
    }

    /**
     * @brief Adds a child item to the item using the path. The required path items are created automatically.
     * @param path              The path to the new child item relatively to the item
     * @param openedImage       The image for the opened folder
     * @param closedImage       The image for the closed folder
     * @param itemImage         The image for the child item
     * @param data              The user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem* addPath(const std::vector<String>& path, const Fl_Image* openedImage, const Fl_Image* closedImage,
                       const Fl_Image* itemImage = nullptr, void* data = nullptr)
    {
        return m_treeControl->addPath(path, openedImage, closedImage, itemImage, data);
    }

    /**
     * @brief Adds a child item to the item using the default folder images.
     * @param path              The item full path in the tree starting with '/'
     * @param itemImage         The image for the child item
     * @param data              The user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem* addPath(const std::vector<String>& path, const Fl_Image* itemImage = 0L, void* data = 0L)
    {
        return m_treeControl->addPath(path, itemImage, itemImage, itemImage, data);
    }

    /**
     * @brief Removes an item from the parent tree item
     */
    void removeItem(CTreeItem* item)
    {
        m_treeControl->removeItem(item);
    }

    /**
     * @brief Returns currently selected item
     */
    CTreeItem* selected() const
    {
        return m_treeControl->selected();
    }

    /**
     * @brief Returns currently selected item path in the tree
     */
    String selectedPath() const;

    /**
     * @brief Removes all the tree items
     */
    virtual void clear()
    {
        m_treeControl->clear();
    }

    /**
     * @brief Relayouts the tree. May be necessary after you've changed items data a lot
     */
    void relayout()
    {
        m_treeControl->relayout();
        redraw();
    }

    /**
     * @brief Resizes the control and inside widgets.
     * @param x                 X-position
     * @param y                 Y-position
     * @param w i               Width
     * @param h                 Height
     */
    void resize(int x, int y, int w, int h) override;

    /**
     * @brief Returns the currently selected item ID (or user_data)
     */
    Variant data() const override;

    /**
     * @brief Selects the item with matching ID (or user_data)
     */
    void data(const Variant v) override;

    /**
     * @brief Data connection isn't implemented yet
     */
    void load(Query*) override;

    /**
     * @brief Data connection isn't implemented yet
     */
    void save(Query*) override;

    /**
     * @brief Loads the the widget from XML node
     *
     * The widget information may include widget attributes
     * and widget data
     * @param node              XML node
     */
    void load(const xml::Node* node) override;

    /**
     * @brief Saves the the widget to XML node
     *
     * The widget information may include widget attributes
     * and widget data
     * @param node              XML node
     */
    void save(xml::Node* node) const override;

    /**
     * @brief Returns true if data is valid
     */
    bool valid() const override
    {
        return true;
    }

    /**
     * @brief Creates a widget based on the XML node information
     * @param node              XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node);
};
/**
 * @}
 */
}
#endif
