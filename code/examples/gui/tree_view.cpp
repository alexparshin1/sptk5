/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       tree_view.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <cstdio>
#include <FL/Fl.H>

#include <sptk5/cutils>
#include <sptk5/cgui>

using namespace std;
using namespace sptk;

CTreeView* tree;

void exit_cb(Fl_Widget* w, void*)
{
    w->window()->hide();
}

void changed_cb(Fl_Widget*, void*)
{
    CTreeItem* item = tree->selected();
    if (item)
        COUT(item->label().c_str() << endl);
}

void add_item_cb(Fl_Widget*, void*)
{
    CTreeItem* selectedItem = tree->selected();

    CDialog dlg(300, 140, "Add new item");

    CComboBox typeCombo("Item Type:");
    typeCombo.labelWidth(80);
    Strings typeChoices;
    typeChoices.push_back(String("Folder", 1));
    typeChoices.push_back(String("Document", 2));
    typeCombo.addRows("type", typeChoices);
    typeCombo.columns()[(unsigned) 0].width(150);
    typeCombo.dataMode(LV_DATA_KEY);
    typeCombo.data(1);

    CComboBox modeCombo("Add Mode:");
    modeCombo.labelWidth(80);
    Strings modeChoices;
    modeChoices.push_back(String("To the root level", 1));
    modeChoices.push_back(String("To selected item", 2));
    modeCombo.addRows("type", modeChoices);
    modeCombo.columns()[(unsigned) 0].width(150);
    modeCombo.dataMode(LV_DATA_KEY);
    modeCombo.data(1);

    CInput inp("Item Name:");
    inp.labelWidth(80);
    dlg.end();
    if (dlg.showModal()) {
        CTreeItem* node = nullptr;
        int mode = int(typeCombo.data()) + int(modeCombo.data()) * 2;
        switch (mode) {
            case 3: // add folder to the root
                node = tree->addItem(inp.data().asString(), CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
                break;
            case 4: // add item to the root
                node = tree->addItem(inp.data().asString(), CTreeItem::getDocument());
                break;
            case 5: // add folder to the current
                if (selectedItem)
                    node = selectedItem->addItem(inp.data().asString().c_str(), CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
                break;
            case 6: // add item to the current
                if (selectedItem)
                    node = selectedItem->addItem(inp.data().asString().c_str(), CTreeItem::getDocument());
                break;
            default:
                break;
        }
        if (node)
            Fl::focus(node);
        tree->relayout();
    }
}

void remove_item_cb(Fl_Widget*, void*)
{
    CTreeItem* item = tree->selected();
    tree->removeItem(item);
}

int main(int argc, char* argv[])
{
    try {
        // Initialize themes
        CThemes themes;

        CWindow window(400, 300);
        window.resizable(window);

        tree = new CTreeView("", 10, SP_ALIGN_CLIENT);
        tree->end();

        CTreeItem* node;

        // Add some nodes with icons -- some open, some closed.

        node = tree->addItem("aaa", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->addItem("bbb 1", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->open();

        node = tree->addItem("bbb 2", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->close();
        node = node->addItem("ccc", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->addItem("ddd", CTreeItem::getDocument());

        node = tree->addItem("eee", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->addItem("fff", CTreeItem::getDocument());

        node = tree->addItem("ggg", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node = node->addItem("hhh", CTreeItem::getDocument());
        node->close();
        node->addItem("iii", CTreeItem::getDocument());

        node = tree->addItem("jjj", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->addItem("kkk", CTreeItem::getDocument());

        tree->addItem("lll", CTreeItem::getDocument());
        node = tree->addItem("mmm", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->close();
        node = node->addItem("nnn", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->addItem("ooo", CTreeItem::getDocument());

        node = tree->addItem("ppp", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->addItem("qqq", CTreeItem::getDocument());

        node = tree->addItem("rrr", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node = node->addItem("sss", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->addItem("ttt", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());

        node = tree->addItem("uuu", CTreeItem::getFolderOpened(), CTreeItem::getFolderClosed());
        node->addItem("vvv", CTreeItem::getDocument());

        node = tree->addItem("www", CTreeItem::getDocument());
        node = node->addItem("xxx", CTreeItem::getDocument());
        node = node->addItem("yyy", CTreeItem::getDocument());
        node->addItem("zzz", CTreeItem::getDocument());

        tree->callback(changed_cb);

        CGroup group("", 10, SP_ALIGN_BOTTOM);
        group.box(FL_THIN_DOWN_BOX);
        group.color(FL_LIGHT1);

        CButton btn1(SP_EXIT_BUTTON, SP_ALIGN_RIGHT);
        btn1.callback(exit_cb);

        CButton btn2(SP_DELETE_BUTTON, SP_ALIGN_RIGHT);
        btn2.callback(remove_item_cb);

        CButton btn3(SP_ADD_BUTTON, SP_ALIGN_RIGHT);
        btn3.callback(add_item_cb);

        window.end();
        window.resizable(&window);

        window.show(argc, argv);

        CThemes::set("OSX");

        Fl::run();

        return 0;
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
        return 1;
    }
}
