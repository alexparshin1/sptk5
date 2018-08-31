/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTreeControl.cpp - description                         ║
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

#include <sptk5/sptk.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <sptk5/gui/CDataControl.h>
#include <sptk5/gui/CTreeControl.h>
#include <sptk5/gui/CThemes.h>

using namespace std;
using namespace sptk;

const Fl_Image* CTreeItem::treeOpened;
const Fl_Image* CTreeItem::treeClosed;
const Fl_Image* CTreeItem::folderClosed;
const Fl_Image* CTreeItem::folderOpened;
const Fl_Image* CTreeItem::document;

CTreeItem::CTreeItem(const char* lbl, const Fl_Image* openedImage, const Fl_Image* closedImage, void* data)
: CGroup("", 10, SP_ALIGN_TOP), m_labelWidth(0), m_labelHeight(0), m_body(nullptr)
{
    m_drawClipped = false;
    m_selected = false;
    m_tree = dynamic_cast<CTreeControl*> (parent());
    auto par = dynamic_cast<CTreeItem*> (parent());
    layoutSpacing(0);
    bool root = false;

    if (m_tree != nullptr) {
        // This widget is the root item
        m_itemHeight = 0;
        m_indent = 18;
        root = true;
    } else {
        if (par != nullptr)
            m_tree = par->tree();

        m_itemHeight = 16;
        m_indent = 18;
    }

    m_opened = true;

    m_openedImage = openedImage;

    if (closedImage != nullptr)
        m_closedImage = closedImage;
    else
        m_closedImage = openedImage;

    if (!root) {
        if (m_openedImage) {
            if (m_indent < m_openedImage->w() + 2)
                m_indent = m_openedImage->w() + 2;

            if (m_itemHeight < m_openedImage->h() + 2)
                m_itemHeight = m_openedImage->h() + 2;
        }

        if (m_closedImage) {
            if (m_indent < m_closedImage->w() + 2)
                m_indent = m_closedImage->w() + 2;

            if (m_itemHeight < m_closedImage->h() + 2)
                m_itemHeight = m_closedImage->h() + 2;
        }
    }

    user_data(data);

    if (root) {
        m_body = m_tree->m_itemCreator(this);
        Fl_Widget* widget = m_body->widget();
        widget->hide();
    } else if (par)
        m_body = par->m_tree->m_itemCreator(this);

    label(lbl);
}

bool CTreeItem::preferredSize(int& ww, int& hh)
{
    if (m_lastPreferredW == ww && m_lastPreferredH == hh)
        return false;

    int offsetX = m_indent + 2;
    int www = ww - (offsetX + 2);

    if (www < 0)
        www = 20;

    bool rc = autoLayout(offsetX, 0, www, hh, false);
    ww = www + (offsetX + 2);
    m_lastPreferredW = ww;
    m_lastPreferredH = hh;
    return rc;
}

void CTreeItem::resize(int xx, int yy, int ww, int hh)
{
    int offsetX = m_indent + 2;
    int www = ww - (offsetX + 2);

    if (www < 0)
        www = 20;

    autoLayout(xx + offsetX, yy, www, hh, true);
    ww = www + (offsetX + 2);
    Fl_Widget::resize(xx, yy, ww, hh);
}

void CTreeItem::label(const char* lbl)
{
    auto abox = dynamic_cast<CBox*> (m_body);

    if (abox) {
        abox->data(lbl);
    } else {
        auto control = dynamic_cast<CControl*> (m_body);

        if (control)
            control->label(lbl);
    }
}

int CTreeItem::handle(int event)
{
    bool relayout = false;

    switch (event) {

        case FL_PUSH:
            tree()->m_tabPressed = false;

            if (opened() && Fl::event_inside(x(), y() + m_itemHeight, m_indent, h() - m_itemHeight)) {
                auto cnt = (unsigned) children();

                for (unsigned i = 1; i < cnt; i++) {
                    auto item = (CTreeItem*) child(i);

                    if (item->y() + 12 >= Fl::event_y()) {
                        if (item->opened())
                            item->close();
                        else
                            item->open();

                        relayout = true;
                        break;
                    }
                }
            } else if (Fl::event_inside(m_indent, y(), w() - m_indent, m_itemHeight)) {
                handle(FL_FOCUS);
                //return 0;
            }

            break;

        case FL_KEYBOARD:
            switch (Fl::event_key()) {
                case FL_Tab:
                    tree()->m_tabPressed = true;
                    break;

                case '+':
                case '=':
                    if (Fl::event_shift()) {
                        visibleChildren(true);
                        relayout = true;
                    }

                    break;

                case '-':
                    visibleChildren(false);
                    relayout = true;
                    break;

                case FL_Down:
                    selectNext();
                    return 1;

                case FL_Up:
                    selectPrior();
                    return 1;
                    //case FL_Page_Down:
                    //        return select_next_page();
                    //case FL_Page_Up:
                    //        return select_prior_page();
                    //case FL_Home:
                    //        return select_first();
                    //case FL_End:
                    //        return select_last();

                default:
                    return 0;
            }

            break;

        case FL_UNFOCUS:
            redraw();
            return 1;

        case FL_FOCUS: {
            if (tree()->m_tabPressed)
                return 0;

            CTreeItem* old_selection = tree()->selected();
            tree()->selectOnly(this, true);
            tree()->makeVisible(this);
            redraw();

            if (old_selection)
                old_selection->redraw();
        }

            return 1;

        default:
            return 0;
    }

    if (relayout) {
        tree()->relayout();
        tree()->redraw();
        return 1;
    }

    return CGroup::handle(event);
}

CTreeItem* CTreeItem::parentItem() const
{
    auto par = dynamic_cast<CTreeItem*> (parent());
    return par;
}

CTreeItem* CTreeItem::addItem(const char* label, const Fl_Image* openedImage, const Fl_Image* closedImage, void* data)
{
    if (!closedImage)
        closedImage = openedImage;

    Fl_Group* saveCurrent = Fl_Group::current();
    begin();
    auto item = new CTreeItem(label, openedImage, closedImage, data);
    //insert(*item,children());

    if (!m_opened)
        item->hide();

    Fl_Group::current(saveCurrent);

    return item;
}

void CTreeItem::draw()
{

    int xx = x();
    int yy = y();

    bool isRoot = dynamic_cast<CTreeControl*> (parent()) != nullptr;

    const Fl_Image* image = nullptr;

    if (!isRoot) {
        if (m_opened) {
            image = m_openedImage;

            if (!image) {
                if (children())
                    image = folderOpened;
                else
                    image = document;
            }
        } else {
            image = m_closedImage;

            if (!image) {
                if (children())
                    image = folderClosed;
                else
                    image = document;
            }
        }

        fl_font(labelfont(), labelsize());
    }

    Fl_Color backClr = tree()->color();
    draw_box(FL_FLAT_BOX, x(), y(), w(), h(), backClr);

    CGroup::draw();

    if (isRoot)
        fl_push_clip(x(), y(), w(), h());

    // draw the lines
    int lx = x() + m_indent / 2;
    int sly = y() + m_itemHeight / 2;
    auto cnt = (unsigned) children();
    int cly = sly;
    fl_color(0x40404000);

    if (m_opened) {
        for (unsigned i = 1; i < cnt; i++) {
            Fl_Widget* item = child(i);
            cly = item->y() + item->labelsize() / 2 + 1;

            if (isRoot && i == 1)
                sly = cly;

            fl_line(lx, cly, item->x() - 1, cly);
        }

        if (cnt > 1)
            fl_line(lx, sly, lx, cly);

        for (unsigned j = 1; j < cnt; j++) {
            auto item = (CTreeItem*) child(j);

            if (item->children() < 2)
                continue;

            cly = item->y() + item->labelsize() / 2 + 1;
            draw_box(FL_THIN_DOWN_BOX, lx - 5, cly - 5, 10, 10, backClr);
            fl_color(0x40404000);
            fl_line(lx - 3, cly, lx + 3, cly);

            if (!item->opened())
                fl_line(lx, cly - 3, lx, cly + 3);
        }
    }

    if (!isRoot) {
        auto img = (Fl_Image*) image;

        if (img)
            img->draw(xx, yy, image->w(), image->h(), 0, 0);
    }

    if (isRoot)
        fl_pop_clip();
}

CTreeItem* CTreeItem::findItem(const char* label) const
{
    auto cnt = (unsigned) children();
    string slabel = label;

    for (unsigned i = 0; i < cnt; i++) {
        auto item = (CTreeItem*) child(i);

        if (item->m_body->label() == slabel)
            return item;

        if (item->children()) {
            CTreeItem* fitem = item->findItem(label);

            if (fitem)
                return fitem;
        }
    }

    return nullptr;
}

CTreeItem* CTreeItem::findData(const void* itemData) const
{
    auto cnt = (unsigned) children();

    for (unsigned i = 0; i < cnt; i++) {
        auto item = (CTreeItem*) child(i);

        if (item->user_data() == itemData)
            return item;

        if (item->children()) {
            item = item->findData(itemData);

            if (item)
                return item;
        }
    }

    return nullptr;
}

CTreeItem* CTreeItem::addPathOffset(const vector<String>& pathFolders, unsigned offset, const Fl_Image* openedImage,
                                    const Fl_Image* closedImage, const Fl_Image* itemImage, void* data)
{
    CTreeItem* node = this;
    auto cnt = (int) pathFolders.size();

    for (int i = offset; i < cnt - 1; i++) {
        const string& folder = pathFolders[i];
        CTreeItem* n = node->findItem(folder.c_str());

        if (!n)
            n = node->addItem(folder.c_str(), openedImage, closedImage);

        node = n;
    }

    if (cnt - 1 >= 0)
        return node->addItem(pathFolders[cnt - 1].c_str(), itemImage, itemImage, data);
    else
        return nullptr;
}

CTreeItem* CTreeItem::addPath(const vector<String>& pathFolders, const Fl_Image* openedImage, const Fl_Image* closedImage,
                              const Fl_Image* itemImage, void* data)
{
    return addPathOffset(pathFolders, 0, openedImage, closedImage, itemImage, data);
}

static Fl_Image* getIconImage(const char* iconName)
{
    return CThemes::getIconImage(iconName, IS_SMALL_ICON);
}

CTreeItem* CTreeItem::addPath(const vector<String>& pathFolders, const Fl_Image* itemImage, void* data)
{
    if (!itemImage)
        itemImage = getIconImage("document");

    return addPath(pathFolders, getIconImage("folder_opened"), getIconImage("folder_closed"), itemImage, data);
}

void CTreeItem::visibleChildren(bool show)
{
    m_opened = show;
    auto cnt = (unsigned) children();

    for (unsigned i = 1; i < cnt; i++) {
        auto item = dynamic_cast<CTreeItem*> (child(i));

        if (!item)
            continue;

        if (show) {
            item->show();

            if (item->m_opened)
                item->visibleChildren(true);
        } else {
            item->hide();
            item->visibleChildren(false);
        }
    }
}

void CTreeItem::moveItem(CTreeItem* item, CTreeItem* beforeItem)
{
    insert(*item, beforeItem);
}

void CTreeItem::moveItem(CTreeItem* item, int direction)
{
    int itemPosition = find(item);

    if (itemPosition == children())
        return;

    if (direction == 0)
        return;

    if (direction < 0) {                          // move item up
        if (itemPosition == 0)
            return;             // can't move - at the start already

        insert(*item, child(itemPosition - 1));
    } else {                                      // move item down
        if (itemPosition == children() - 1)
            return;  // can't move - at the end already

        insert(*item, itemPosition + 1);
    }
}

void CTreeItem::removeItem(CTreeItem* item)
{
    int itemPosition = find(item);

    if (itemPosition == children())
        return;

    Fl_Widget* newSelected = nullptr;
    bool wasFocused = (Fl::focus() == item);

    if (item->m_selected || wasFocused) {
        if (itemPosition < children() - 1) {
            newSelected = (CTreeItem*) child(itemPosition + 1);
        } else {
            if (children() > 1)
                newSelected = (CTreeItem*) child(itemPosition - 1);
            else
                newSelected = this;
        }
    }

    remove
        (item);
    delete item;

    if (newSelected)
        tree()->selectOnly((CTreeItem*) newSelected, wasFocused);
}

void CTreeItem::clear()
{
    while (children() > 1) {
        Fl_Widget* widget = child(1);
        remove
            (*widget);
        delete widget;
    }
}

void CTreeItem::select(bool flag)
{
    if (m_selected == flag)
        return;

    m_selected = flag;
    auto widget = (CGroup*) m_body->widget();

    if (m_selected) {
        m_itemColor[0] = widget->labelcolor();
        m_itemColor[1] = widget->color();
        widget->color(FL_DARK2);
    } else {
        widget->labelcolor(m_itemColor[0]);
        widget->color(m_itemColor[1]);
    }

    redraw();
}

CTreeItem* CTreeItem::findFirst() const
{
    if (opened() && children() > 1)
        return (CTreeItem*) child(1);

    return nullptr;
}

CTreeItem* CTreeItem::findLast(bool recursive) const
{
    if (opened() && children() > 1) {
        auto lastChild = (CTreeItem*) child(children() - 1);

        if (recursive) {
            CTreeItem* foundLast = lastChild->findLast(recursive);

            if (foundLast)
                lastChild = foundLast;
        }

        return lastChild;
    }

    return nullptr;
}

CTreeItem* CTreeItem::findNext(bool recursive) const
{
    if (recursive && opened()) {
        CTreeItem* firstChild = findFirst();

        if (firstChild)
            return firstChild;
    }

    int index = parent()->find(this);

    if (index < parent()->children() - 1)
        return (CTreeItem*) parent()->child(index + 1);

    return nullptr;
}

CTreeItem* CTreeItem::findPrior(bool recursive) const
{
    CTreeItem* currentItem;
    CTreeItem* lastChild;
    int index = parent()->find(this);

    if (index > 1) {
        currentItem = (CTreeItem*) parent()->child(index - 1);

        if (recursive) {
            lastChild = currentItem->findLast(recursive);

            if (lastChild)
                currentItem = lastChild;
        }

        return currentItem;
    }

    if (!recursive)
        return nullptr;

    currentItem = parentItem();

    if (!currentItem) // root item?
        return nullptr;

    if (currentItem->parentItem())
        return currentItem;

    return nullptr;
}

bool CTreeItem::selectNext()
{
    CTreeItem* nextItem = findNext(true);

    if (nextItem) {
        tree()->selectOnly(nextItem, true);
        return true;
    }

    CTreeItem* currentItem = this;

    while (currentItem && currentItem->parentItem()) {
        nextItem = currentItem->findNext(false);

        if (nextItem) {
            tree()->selectOnly(nextItem, true);
            return true;
        }

        currentItem = currentItem->parentItem();
    }

    return false;
}

bool CTreeItem::selectPrior()
{
    CTreeItem* priorItem = findPrior(true);

    if (priorItem) {
        tree()->selectOnly(priorItem, true);
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------
CTreeControl::CTreeControl(const char* label, int layoutSize, CLayoutAlign align)
    : CScroll(label, layoutSize, align)
{
    m_tabPressed = false;
    m_itemCreator = defaultItemCreator;
    box(FL_THIN_DOWN_BOX);
    layoutSpacing(0);
    begin();
    m_root = new CTreeItem("");
    end();
}

CLayoutClient* CTreeControl::defaultItemCreator(CTreeItem*)
{
    auto box = new CBox("", 16, SP_ALIGN_TOP);
    box->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    return box;
}

CTreeItem*
CTreeControl::addItem(const char* label, const Fl_Image* openedImage, const Fl_Image* closedImage, void* data)
{
    return m_root->addItem(label, openedImage, closedImage, data);
}

CTreeItem* CTreeControl::addPath(const vector<String>& path, const Fl_Image* openedImage, const Fl_Image* closedImage,
                                 const Fl_Image* itemImage, void* data)
{
    return m_root->addPath(path, openedImage, closedImage, itemImage, data);
}

CTreeItem* CTreeControl::addPath(const vector<String>& path, const Fl_Image* itemImage, void* data)
{
    return addPath(path, getIconImage("folder_opened"), getIconImage("folder_closed"), itemImage, data);
}

void CTreeControl::moveItem(CTreeItem* item, CTreeItem* beforeItem)
{
    CTreeItem* parentItem = item->parentItem();
    parentItem->moveItem(item, beforeItem);
}

void CTreeControl::removeItem(CTreeItem* item)
{
    CTreeItem* parentItem = item->parentItem();
    parentItem->removeItem(item);
    relayout();
    redraw();
}

CTreeItem* CTreeControl::findItem(const char* label) const
{
    return m_root->findItem(label);
}

CTreeItem* CTreeControl::findData(const void* itemData) const
{
    return m_root->findData(itemData);
}

void CTreeControl::selectOnly(CTreeItem* item, bool giveFocus)
{
    if (item->tree() != this)
        return;

    if (giveFocus)
        Fl::focus(item);

    /// Unselect currently selected items:
    auto itor = m_selectedItems.begin();
    auto iend = m_selectedItems.end();

    for (; itor != iend; ++itor) {
        CTreeItem* treeItem = *itor;

        if (treeItem != item)
            treeItem->select(false);
    }

    m_selectedItems.clear();

    /// Select the only item
    m_selectedItems.push_back(item);
    makeVisible(item);

    if (!item->selected()) {
        item->select(true);
        do_callback();
    }
}

void CTreeControl::makeVisible(CTreeItem* item)
{
    if (item->tree() != this)
        return;

    int dh = 0;

    if (hscrollbar.visible())
        dh = hscrollbar.h();

    /// Is the bottom of the item visible?
    int itemY = item->y();
    int dy = y() + h() - dh - (itemY + item->h());
    int ypos = yposition();

    if (dy < 0) {
        ypos -= dy;
        itemY += dy;
    }

    /// Is the top of the item visible?
    dy = y() - itemY;

    if (dy > 0)
        ypos -= dy;

    position(xposition(), ypos);

    redraw();
}

int CTreeControl::handle(int event)
{
    if (event == FL_PUSH)
        return CScroll::handle(event);

    if (event == FL_FOCUS) {
        m_tabPressed = false;
        CTreeItem* old_selection = selected();

        if (old_selection) {
            Fl::focus(old_selection);
            redraw();
            return true;
        }
    }

    return CScroll::handle(event);
}

void CTreeControl::load(const xml::Node& groupNode, bool /*autoCreate*/)
{
    if (m_noXml)
        return;

    if (!(bool) groupNode.getAttribute("visible", "Y"))
        m_group->hide();
    else
        m_group->show();

    if (!(bool) groupNode.getAttribute("enable", "Y"))
        m_group->deactivate();
    else
        m_group->activate();
}

void CTreeControl::save(xml::Node& groupNode) const
{
    groupNode.clear();
    groupNode.name("tree");

    if (m_noXml)
        return;

    String label = m_group->label();

    if (!label.empty())
        groupNode.setAttribute("label", label);

    if (!visible())
        groupNode.setAttribute("visible", "N");

    if (!active())
        groupNode.setAttribute("enable", "N");
}
