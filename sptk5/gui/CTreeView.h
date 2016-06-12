/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTreeView.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

/// @addtogroup gui GUI Classes
/// @{

/// @brief Tree view widget.
///
/// Shows tree items with icons. Can use different icons for opened/closed folders.
class SP_EXPORT CTreeView : public CControl {

    /// @brief The actual tree control
    CTreeControl *m_treeControl;

    /// @brief Constructor initializer
    void ctor_init();
protected:

    /// @brief Internal tree control callback
    static void internal_callback(Fl_Widget *,void *);

public:

    /// @brief Constructor in SPTK style
    /// @param label const char *, label
    /// @param layoutSize int, widget align in layout
    /// @param layoutAlign CLayoutAlign, widget align in layout
    CTreeView(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /// @brief Constructor in FLTK style
    /// @param x int, x-position
    /// @param y int, y-position
    /// @param w int, width
    /// @param h int, height
    /// @param label, const char * label
    CTreeView(int x,int y,int w,int h,const char *label=0);
#endif

    /// @brief Destructor
    ~CTreeView();

    /// @brief Returns controls' kind (internal SPTK RTTI).
    virtual CControlKind kind() const {
        return DCV_TREEVIEW;
    }

    /// @brief Returns controls' class name (internal SPTK RTTI).
    virtual std::string className() const {
        return "CTreeView";
    }

    /// @brief Adds a child item to the item. If the closedImage parameter is omitted the openedImage is used instead.
    /// @param label std::string, the item label
    /// @param openedImage const Fl_Image *, the image for the opened state
    /// @param closedImage const Fl_Image *, the image for the closed state
    /// @param data void *, the user data or ID attached to the item
    /// @returns the new child item
    CTreeItem *addItem(std::string label,const Fl_Image *openedImage,const Fl_Image *closedImage=0L,void *data=0L) {
        return m_treeControl->addItem(label.c_str(),openedImage,closedImage,data);
    }

    /// @brief Adds a child item to the item using the path. The required path items are created automatically.
    /// @param path const CStrings&, the path to the new child item relatively to the item
    /// @param openedImage const Fl_Image *, the image for the opened folder
    /// @param closedImage const Fl_Image *, the image for the closed folder
    /// @param itemImage const Fl_Image *, the image for the child item
    /// @param data void *, the user data or ID attached to the item
    /// @returns the new child item
    CTreeItem *addPath(const std::vector<std::string>& path,const Fl_Image *openedImage,const Fl_Image *closedImage,const Fl_Image *itemImage=0L,void *data=0L) {
        return m_treeControl->addPath(path,openedImage,closedImage,itemImage,data);
    }

    /// @brief Adds a child item to the item using the default folder images.
    /// @param path const CStrings&, the item full path in the tree starting with '/'
    /// @param itemImage const Fl_Image *, the image for the child item
    /// @param data void *, the user data or ID attached to the item
    /// @returns the new child item
    CTreeItem *addPath(const std::vector<std::string>& path,const Fl_Image *itemImage=0L,void *data=0L) {
        return addPath(path,itemImage,data);
    }

    /// @brief Removes an item from the parent tree item
    void removeItem(CTreeItem *item) {
        m_treeControl->removeItem(item);
    }

    /// @brief Returns currently selected item
    CTreeItem *selected() const {
        return m_treeControl->selected();
    }

    /// @brief Returns currently selected item path in the tree
    std::string selectedPath() const;

    /// @brief Removes all the tree items
    virtual void clear() {
        m_treeControl->clear();
    }

    /// @brief Relayouts the tree. May be necessary after you've changed items data a lot
    void relayout() {
        m_treeControl->relayout();
        redraw();
    }

    /// @brief Resizes the control and inside widgets.
    /// @param x int, x-position
    /// @param y int, y-position
    /// @param w int, width
    /// @param h int, height
    virtual void     resize(int x,int y,int w,int h);

    /// @brief Returns the currently selected item ID (or user_data)
    virtual CVariant data() const;

    /// @brief Selects the item with matching ID (or user_data)
    virtual void     data(const CVariant v);

    /// @brief Data connection isn't implemented yet
    virtual void load(CQuery *);

    /// @brief Data connection isn't implemented yet
    virtual void save(CQuery *);

    /// @brief Loads the the widget from XML node
    ///
    /// The widget information may include widget attributes
    /// and widget data
    /// @param node CXmlNode*, XML node
    virtual void load(const CXmlNode *node);

    /// @brief Saves the the widget to XML node
    ///
    /// The widget information may include widget attributes
    /// and widget data
    /// @param node CXmlNode*, XML node
    virtual void save(CXmlNode *node) const;

    /// @brief Returns tru if data is valid
    virtual bool valid() const {
        return true;
    }

    /// @brief Creates a widget based on the XML node information
    /// @param node CXmlNode*, an XML node with widget information
    static CLayoutClient* creator(CXmlNode* node);
};
/// @}
}
#endif
