/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTreeControl.h - description                           ║
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

#ifndef __CTREECONTROL_H__
#define __CTREECONTROL_H__

#include <FL/Fl_Widget.H>
#include <FL/Fl_Pixmap.H>
#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/gui/CScroll.h>
#include <sptk5/gui/CGroup.h>

#include <string>
#include <vector>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

class CTreeItem;
class CTreeControl;

/**
 * @brief A callback that allows to create a widget of required type and use it as an item body
 *
 * An item body should be any CLayoutClient-derived widget.
 * By default (if that callback isn't redefined, SPTK creates CBox widget.
 * @param item CTreeItem*, a tree item that would be a parent for the new item
 */
typedef CLayoutClient* (*CTreeItemCreator) (CTreeItem *item);

/**
 * @brief Tree widget item.
 *
 * A group widget with the extra information about item pixmaps
 * and current state.
 */
class CTreeItem : public CGroup
{
    /**
     * Returns the height of the item text
     */
    int             m_itemHeight;

    /**
     * The width of the indent zone
     */
    int             m_indent;

    /**
     * The width of the label computed by fl_measure
     */
    int             m_labelWidth;

    /**
     * The height of the label computed by fl_measure
     */
    int             m_labelHeight;

    /**
     * Is the item selected (controlled by parent tree control)
     */
    bool            m_selected;

    /**
     * Item colors (text,bg) before selection
     */
    Fl_Color        m_itemColor[2];


protected:
    /**
     * The widget that represents the item without a child
     */
    CLayoutClient  *m_body;

    /**
     * The image for the open state
     */
    const Fl_Image *m_openedImage;

    /**
     * The image for the closed state
     */
    const Fl_Image *m_closedImage;

    /**
     * The item's state
     */
    bool            m_opened;

    /**
     * The tree control
     */
    CTreeControl   *m_tree;


    /** Adds a child item to the item using the path. The required path items are created automatically.
     * This method is used internally only.
     * @param pathFolders const std::vector<std::string>&, the path to the new child item relatively to the item
     * @param offset uint32_t, the offset in pathFolders where the path starts
     * @param openedImage Fl_Image, the image for the opened folder
     * @param closedImage Fl_Image, the image for the closed folder
     * @param itemImage Fl_Image, the image for the child item
     * @param data void *, the user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem *addPathOffset (const std::vector<std::string>& pathFolders,uint32_t offset,const Fl_Image *openedImage,const Fl_Image *closedImage,const Fl_Image *itemImage=0L,void *data=0L);

public:
    /**
     * Default image of the opened tree
     */
    static const Fl_Image* treeOpened;

    /**
     * Default image of the closed tree
     */
    static const Fl_Image* treeClosed;

    /**
     * Default image of the opened floder
     */
    static const Fl_Image* folderOpened;

    /**
     * Default image of the closed floder
     */
    static const Fl_Image* folderClosed;

    /**
     * Default image of the document
     */
    static const Fl_Image* document;


public:
    /** Constructor. If the closedImage parameter is omitted the openedImage is used instead.
     * @param label const char *, the item label
     * @param openedImage Fl_Image, the image for the opened state
     * @param closedImage Fl_Image, the image for the closed state
     * @param data void *, the user data or ID attached to the item
     */
    CTreeItem (const char *label,const Fl_Image *openedImage=0L,const Fl_Image *closedImage=0L,void *data=0L);

    /** Adds a child item to the item. If the closedImage parameter is omitted the openedImage is used instead.
     * @param label const char *, the item label
     * @param openedImage Fl_Image, the image for the opened state
     * @param closedImage Fl_Image, the image for the closed state
     * @param data void *, the user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem *addItem (const char *label,const Fl_Image *openedImage=0L,const Fl_Image *closedImage=0L,void *data=0L);

    /** Adds a child item to the item using the path. The required path items are created automatically.
     * @param path const std::vector<std::string>&, the path to the new child item relatively to the item
     * @param openedImage Fl_Image, the image for the opened folder
     * @param closedImage Fl_Image, the image for the closed folder
     * @param itemImage Fl_Image, the image for the child item
     * @param data void *, the user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem *addPath (const std::vector<std::string>& path,const Fl_Image *openedImage,const Fl_Image *closedImage,const Fl_Image *itemImage=0L,void *data=0L);

    /** Adds a child item to the item using the path. The required path items are created automatically.
     * The default images are used for the folders in the path.
     * @param path const std::vector<std::string>&, the path to the new child item relatively to the item
     * @param itemImage Fl_Image, the image for the child item
     * @param data void *, the user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem *addPath (const std::vector<std::string>& path,const Fl_Image *itemImage=0L,void *data=0L);

    /**
     * @brief The widget that represents the item without children
     */
    CLayoutClient *body()
    {
        return m_body;
    }

    /**
     * @brief Sets the item's caption
     * @param lbl const char *, the new item caption
     */
    void label (const char *lbl);

    /**
     * @brief Returns the item's caption
     */
    const char * label() const
    {
        return m_body->label().c_str();
    }

    /**
     * @brief Removes all the child items of the item
     */
    virtual void clear();

    /**
     * @brief Finds an item among the child items
     * @returns the child item (if found) or NULL
     */
    CTreeItem *findItem (const char *label) const;

    /**
     * @brief Finds an item by the item user data in the whole tree
     * @param data void *, user data to find
     * @returns the child item (if found) or NULL
     */
    CTreeItem *findData (const void *data) const;

    /**
     * @brief Removes an item from the child items.
     *
     * Only the intermediate children are considered
     * @param item CTreeItem *, item to remove
     */
    void removeItem (CTreeItem *item);

    /**
     * @brief Returns the parent item
     * @returns the parent item or NULL (for the root item)
     */
    CTreeItem *parentItem() const;

    /**
     * @brief Returns the tree control this item belongs to
     */
    CTreeControl *tree() const
    {
        return m_tree;
    }

    /**
     * @brief Moves the immediate child before another immediate child
     *
     * If beforeItem is 0L then moves to the end of the children
     * @param item CTreeItem *, the item to move
     * @param beforeItem CTreeItem *, the item to insert before
     */
    void moveItem (CTreeItem *item,CTreeItem *beforeItem=0L);

    /**
     * @brief Moves the immediate child up or down in the list of items
     * @param item CTreeItem *, the item to move
     * @param direction int, the direction to move. -1 to move up, 1 to move down
     */
    void moveItem (CTreeItem *item,int direction);

    /**
     * @brief Shows or hides all the child items. Changes the item state accordingly.
     * @param vc bool, show(true) or hide(false) children
     */
    void visibleChildren (bool vc);

    /**
     * @brief Shows all the child items. Changes the item state accordingly.
     */
    void open()
    {
        visibleChildren (true);
    }

    /**
     * @brief Hides all the child items. Changes the item state accordingly.
     */
    void close()
    {
        visibleChildren (false);
    }

    /**
     * @brief Reports the item state - opened or closed.
     */
    bool opened() const
    {
        return m_opened;
    }

    /**
     * @brief Reports the item indent from the left.
     */
    uint32_t indent() const
    {
        return m_indent;
    }
    
    /**
     * @brief Sets image(s) for the item
     * @param openedImage Fl_Image *, the image for the opened state
     * @param closedImage Fl_Image *, the image for the closed state
     */
    void setImage (const Fl_Image *openedImage,const Fl_Image *closedImage=0L)
    {
        m_openedImage = openedImage;
        m_closedImage = closedImage;
    }

    /**
     * @brief The event handle function. Internal. See Fl_Widget for details.
     * @param event int, event type.
     * @returns true if event was processed.
     */
    int handle (int event);

    /**
     * @brief The draw function. Internal. See Fl_Widget for details.
     */
    virtual void draw();

    /**
     * @brief Computes the preferred size of the item based on the font of the parent widget, the image size, and the text (label) of the item.
     * @param w int, input/output desirable widget width
     * @param h int, input/output desirable widget heigth
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize (int& w,int& h);

    /**
     * @brief Resizes item and sub items
     */
    void resize (int xx,int yy,int ww,int hh);

    /**
     * @brief Selects or unselects item
     * @param flag bool, true is item shoud be selected
     */
    virtual void select (bool flag);

    /**
     * @brief Returns selection state of the item
     */
    bool selected() const
    {
        return m_selected;
    }

    /**
     * @brief Selects the next item in the tree
     */
    bool selectNext();

    /**
     * @brief Selects the prior item in the tree
     */
    bool selectPrior();

    /**
     * @brief Selects the last item in the tree
     */
    bool selectFirst();

    /**
     * @brief Selects the first item in the tree
     */
    bool selectLast();

    /**
     * @brief Selects the first visible item in the child tree
     */
    CTreeItem* findFirst() const;

    /**
     * @brief Selects the last visible item in the child tree
     * @param recursive bool, should we search the child items
     */
    CTreeItem* findLast (bool recursive) const;

    /**
     * @brief Selects the next visible item in the child tree
     * @param recursive bool, should we search the child items
     */
    CTreeItem* findNext (bool recursive) const;

    /**
     * @brief Selects the prior visible item in the child tree
     * @param recursive bool, should we search the child items
     */
    CTreeItem* findPrior (bool recursive) const;

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual std::string className() const
    {
        return "treeitem";
    }
};

/**
 * @brief Vector of CTreeItem pointers
 */
typedef std::vector<CTreeItem*> CTreeItemVector;

/**
 * @brief Tree widget.
 *
 * Designed to be used inside
 * CTreeView widget, but also can be used by itself. It's missing for data connection
 * support of CTreeView, though.
 */
class CTreeControl : public CScroll
{
    friend class CTreeItem;
private:
    /**
     * The selected items
     */
    CTreeItemVector   m_selectedItems;

    /**
     * The root item
     */
    CTreeItem*        m_root;

protected:
    /**
     * Internal flag to temporarely block tree items focus acceptance
     */
    bool              m_tabPressed;

    /**
     * The tree item creator creates a tree item' body, @see CTreeItemCreator
     */
    CTreeItemCreator  m_itemCreator;


    /**
     * @brief The default tree item creator
     *
     * Creates a tree item' body as CBox, @see CTreeItemCreator
     */
    static CLayoutClient* defaultItemCreator (CTreeItem *item);
public:
    /**
     * @brief The constructor.
     * @param label const char *, the widget's label
     * @param layoutSize int, size of widget in layout. See CLayoutClient for details
     * @param align CLayoutAlign, widget align in the layout
     */
    CTreeControl (const char *label,int layoutSize=50,CLayoutAlign align=SP_ALIGN_TOP);

    /**
     * @brief Adds a child item to the root item. If the closedImage parameter is omitted the openedImage is used instead.
     * @param label const char *, the item label
     * @param openedImage Fl_Image, the image for the opened state
     * @param closedImage Fl_Image, the image for the closed state
     * @param data void *, the user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem *addItem (const char *label,const Fl_Image *openedImage=0L,const Fl_Image *closedImage=0L,void *data=0L);

    /**
     * @brief Adds a child item to the root item using the path. 
     *
     * The required path items are created automatically.
     * Path elements are separated with '/'. The default images are used for the folders in the path.
     * @param path const std::vector<std::string>&, the path to the new child item
     * @param openedImage Fl_Image, the image for the folders in opened state
     * @param closedImage Fl_Image, the image for the folders in closed state
     * @param itemImage Fl_Image, the image for the child item
     * @param data void *, the user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem *addPath (const std::vector<std::string>& path,const Fl_Image *openedImage,const Fl_Image *closedImage,const Fl_Image *itemImage=0L,void *data=0L);

    /** Adds a child item to the root item using the path. The required path items are created automatically.
     * Path elements are separated with '/'. The default images are used for the folders in the path.
     * @param path const std::vector<std::string>&, the path to the new child item
     * @param itemImage Fl_Image, the image for the child item
     * @param data void *, the user data or ID attached to the item
     * @returns the new child item
     */
    CTreeItem *addPath (const std::vector<std::string>& path,const Fl_Image *itemImage=0L,void *data=0L);

    /**
     * @brief Finds an item by the item label in the whole tree
     * @param item const char *, item label to find
     * @returns the child item (if found) or NULL
     */
    CTreeItem *findItem (const char *item) const;

    /**
     * @brief Finds an item by the item user data in the whole tree
     * @param data void *, user data to find
     * @returns the child item (if found) or NULL
     */
    CTreeItem *findData (const void *data) const;

    /**
     * @brief Removes all the items in the tree.
     */
    virtual void clear()
    {
        m_selectedItems.clear();
        m_root->clear();
    }

    /**
     * @brief Moves the item in the tree.
     * @param item CTreeItem *, item to insert
     * @param beforeItem CTreeItem *, item before the insert point. If 0L - it's moved to the very last position in parent item list of items.
     */
    void moveItem (CTreeItem *item,CTreeItem *beforeItem=0L);

    /**
     * @brief Removes all the item and underlying structure from the tree.
     * @param item CTreeItem *, item to remove
     */
    void removeItem (CTreeItem *item);

    /**
     * @brief Reports the root item.
     */
    CTreeItem *root() const
    {
        return m_root;
    }

    /**
     * @brief Reports the currently selected item.
     */
    CTreeItem *selected() const
    {
        if (m_selectedItems.empty())
            return 0;

        return m_selectedItems[0];
    }

    /**
     * @brief Selects only one item.
     * @param item CTreeItem, item to select
     * @param giveFocus bool, should the item be focused?
     */
    void selectOnly (CTreeItem *item,bool giveFocus=false);

    /**
     * @brief Makes the item visible if it's outside the visible area.
     * @param item CTreeItem, item to make visible
     */
    void makeVisible (CTreeItem *item);

    /**
     * @brief Special handle() function.
     * @param event int, an FLTK event
     * @returns true, if event was processed
     */
    int handle (int event);

    /**
     * @brief The tree item creator creates a tree item' body, @see CTreeItemCreator
     */
    void itemCreator (CTreeItemCreator ic)
    {
        m_itemCreator = ic;
    }

    /**
     * @brief Loads group controls data from XML node
     *
     * @param node const XMLNode&, node to load data from
     * @param autoCreate bool, create widgets if they are not found
     */
    virtual void load (const XMLNode& node,bool autoCreate=false) THROWS_EXCEPTIONS;

    /**
     * @brief Loads group controls data from XML node
     *
     * @param node const XMLNode*, node to load data from
     * @param autoCreate bool, create widgets if they are not found
     */
    virtual void load (const XMLNode* node,bool autoCreate=false) THROWS_EXCEPTIONS
    {
        load (*node,autoCreate);
    }

    /**
     * @brief Saves group controls data into XML node
     *
     * @param node const XMLNode&, node to save data into
     */
    virtual void save (XMLNode& node) const;

    /**
     * @brief Saves group controls data into XML node
     *
     * @param node const XMLNode*, node to save data into
     */
    virtual void save (XMLNode* node) const
    {
        save (*node);
    }

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual std::string className() const
    {
        return "tree";
    }
};
/**
 * @}
 */
}
#endif
