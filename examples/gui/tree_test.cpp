/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       tree_test.cpp - description                            ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <FL/Fl.H>

#include <sptk5/db/CSQLite3Connection.h>
#include <sptk5/cdatabase>
#include <sptk5/cgui>
#include <math.h>

using namespace std;
using namespace sptk;

class CFileListTreeHeader : public CScroll
{
    CTreeItem* m_treeItem;
    CGroup* m_group;
    CButton* m_title;
    CButton* m_fileSize;
    CButton* m_publisher;
    CButton* m_publishDate;
    CButton* m_progression;

    static void sort_cb(Fl_Widget*, void*);

public:
    static CLayoutClient* itemCreator(CTreeItem* item);

public:
    explicit CFileListTreeHeader(CTreeItem* item);

    void sync(int xpos, unsigned ww);

    virtual bool preferredSize(int& w, int& h);
};

/// A directory item in the tree
class CFileListTreeFolder : public CGroup
{
protected:
    CTreeItem* m_treeItem;
    CBox* m_title;
    CGroup* m_titleGroup;
public:
    static CLayoutClient* itemCreator(CTreeItem* item);

public:
    explicit CFileListTreeFolder(CTreeItem* item);

    void title(string fname)
    {
        m_title->data(fname);
    }

    string title() const
    {
        return m_title->data();
    }

    virtual bool isFolder() const
    {
        return true;
    }
};

/// The following class creates a single file item in CTreeControl
class CFileListTreeItem : public CFileListTreeFolder
{
    CBox* m_fileSize;
    CBox* m_publisher;
    CBox* m_moreInfo;
    CProgressBar* m_progress;
    CSmallButton* m_cancel;
    CBox* m_publishDateBox;
    DateTime m_publishDate;
public:
    static CLayoutClient* itemCreator(CTreeItem* item);

    static void moreInfo_cb(Fl_Widget*, void*);

    static void cancel_cb(Fl_Widget*, void*);

public:
    explicit CFileListTreeItem(CTreeItem* item);

    void fileSize(int64_t fsize)
    {
        m_fileSize->data(fsize);
    }

    string publisher() const
    {
        return m_publisher->data();
    }

    void publisher(string pub)
    {
        m_publisher->data(pub);
    }

    DateTime publishDate() const
    {
        return m_publishDate;
    }

    void publishDate(DateTime dt)
    {
        m_publishDate = dt;
        m_publishDateBox->data(dt.monthName().substr(0, 3) + " " + int2string(dt.day()) + " " + int2string(dt.year()));
    }

    int64_t fileSize() const
    {
        return m_fileSize->data();
    }

    void progress(unsigned perc)
    {
        m_progress->data(perc);
    }

    unsigned progress() const
    {
        return m_progress->data();
    }

    void fileName(string fname)
    {
        m_title->hint(fname);
    }

    string fileName() const
    {
        return m_title->hint();
    }

    virtual bool isFolder() const
    {
        return false;
    }
};

class CFileManagerTree : public CTreeControl
{
    CFileListTreeHeader* m_header;
    CSQLite3Connection m_db;

    static unsigned compareSortColumn;
    static unsigned compareSortInverse;

    static int compareFileItems(const void* ti1, const void* ti2);

    void sortFiles(CTreeItem* parentItem, unsigned column);

public:
    explicit CFileManagerTree(CFileListTreeHeader* header);

    void refreshData();

    CTreeItem* addFile(
            CTreeItem* parent, string title, string fileName, int64_t fileSize, string publisher, DateTime publishDate,
            unsigned progress);

    CTreeItem* addDirectory(CTreeItem* parent, string title);

    void sortFiles(unsigned column);

    virtual void position(int X, int Y);

    virtual void resize(int X, int Y, int W, int H);
};

class CFileManager : public CGroup
{
    CFileListTreeHeader* m_header;
    CFileManagerTree* m_tree;
public:
    CFileManager(const char* label = "", int layoutSize = 50, CLayoutAlign layoutAlign = SP_ALIGN_CLIENT)
            : CGroup(label, layoutSize, layoutAlign)
    {
        layoutSpacing(0);
        m_header = new CFileListTreeHeader(0);
        m_tree = new CFileManagerTree(m_header);
    }

    void refreshData()
    {
        m_tree->refreshData();
    }

    CTreeItem* addFile(
            CTreeItem* parentItem, string title, string fileName, int64_t fileSize, string publisher,
            DateTime publishDate, unsigned progress)
    {
        if (!parentItem)
            parentItem = m_tree->root();
        return m_tree->addFile(parentItem, title, fileName, fileSize, publisher, publishDate, progress);
    }

    CTreeItem* addDirectory(CTreeItem* parentItem, string title)
    {
        if (!parentItem)
            parentItem = m_tree->root();
        return m_tree->addDirectory(parentItem, title);
    }
};

CFileManagerTree* fileManagerWidget;
CFileListTreeHeader* fileManagerHeader;

CFileListTreeHeader::CFileListTreeHeader(CTreeItem* item)
        : CScroll("", 16, SP_ALIGN_TOP)
{
    fileManagerHeader = this;
    m_treeItem = item;
    box(FL_FLAT_BOX);
    type(0); // no scrollbars
    layoutSpacing(0);
    m_group = new CGroup("", 10, SP_ALIGN_TOP);
    m_group->layoutSpacing(0);

    m_title = new CButton("Title", SP_ALIGN_CLIENT);
    m_title->callback(sort_cb, (void*) 0);

    new CBox("", 22, SP_ALIGN_RIGHT);

    m_progression = new CButton("Progress", SP_ALIGN_RIGHT);
    m_progression->layoutSize(100);
    m_progression->callback(sort_cb, (void*) 4);

    m_publishDate = new CButton("Published", SP_ALIGN_RIGHT);
    m_publishDate->layoutSize(100);
    m_publishDate->callback(sort_cb, (void*) 3);

    m_publisher = new CButton("Publisher", SP_ALIGN_RIGHT);
    m_publisher->layoutSize(150);
    m_publisher->callback(sort_cb, (void*) 2);

    m_fileSize = new CButton("File Size", SP_ALIGN_RIGHT);
    m_fileSize->layoutSize(100);
    m_fileSize->callback(sort_cb, (void*) 1);

    end();
}

bool CFileListTreeHeader::preferredSize(int& w, int& h)
{
    int ww = 1024;
    return m_group->preferredSize(ww, h);
}

void CFileListTreeHeader::sync(int xpos, unsigned ww)
{
    if (ww != (unsigned) m_group->w())
        m_group->size(ww, h());
    position(xpos, 0);
    damage(FL_DAMAGE_SCROLL);
}

CFileListTreeFolder::CFileListTreeFolder(CTreeItem* item)
        : CGroup()
{
    layoutSpacing(0);
    layoutSize(16);
    box(FL_FLAT_BOX);
    m_treeItem = item;
    m_titleGroup = new CGroup("", 150, SP_ALIGN_CLIENT);
    m_titleGroup->layoutSpacing(0);
    m_titleGroup->box(FL_NO_BOX);
    m_title = new CBox("", 150, SP_ALIGN_CLIENT);
    m_title->box(FL_NO_BOX);
    end();
}

CFileListTreeItem::CFileListTreeItem(CTreeItem* item)
        : CFileListTreeFolder(item)
{
    //color(FL_BLUE);
    begin();

    m_titleGroup->box(FL_NO_BOX);
    m_titleGroup->begin();
    m_moreInfo = new CBox("more..", 50, SP_ALIGN_RIGHT);
    m_moreInfo->box(FL_NO_BOX);
    m_moreInfo->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    m_moreInfo->labelColor(FL_RED);
    m_moreInfo->callback(moreInfo_cb);
    m_titleGroup->end();

    m_cancel = new CSmallButton(SP_CANCEL_BUTTON, SP_ALIGN_RIGHT);
    m_cancel->callback(cancel_cb, 0);
    m_cancel->buttonImage(SP_CANCEL_BUTTON, IS_SMALL_ICON);
    m_cancel->box(FL_FLAT_BOX);

    m_progress = new CProgressBar("", 100, SP_ALIGN_RIGHT);
    m_progress->labelWidth(0);
    m_progress->data(50);

    m_publishDateBox = new CBox("", 100, SP_ALIGN_RIGHT);
    m_publishDateBox->box(FL_NO_BOX);
    m_publishDateBox->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

    m_publisher = new CBox("", 150, SP_ALIGN_RIGHT);
    m_publisher->box(FL_NO_BOX);
    m_publisher->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    m_fileSize = new CBox("", 100, SP_ALIGN_RIGHT);
    m_fileSize->box(FL_NO_BOX);
    m_fileSize->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

    end();
}

CLayoutClient* CFileListTreeHeader::itemCreator(CTreeItem* item)
{
    return new CFileListTreeHeader(item);
}

CLayoutClient* CFileListTreeFolder::itemCreator(CTreeItem* item)
{
    return new CFileListTreeFolder(item);
}

CLayoutClient* CFileListTreeItem::itemCreator(CTreeItem* item)
{
    return new CFileListTreeItem(item);
}

void CFileListTreeHeader::sort_cb(Fl_Widget* button, void* cnum)
{
    int xpos = fileManagerHeader->xposition();
    fileManagerWidget->sortFiles((uint32_t)(size_t)(cnum));
    fileManagerHeader->position(xpos, 0);
}

void CFileListTreeItem::cancel_cb(Fl_Widget*, void*)
{
    spInformation("<b>Cancel Download</b><p>Not implemented yet, sorry. This is just a demo of the tree widget.</p>");
}

void CFileListTreeItem::moreInfo_cb(Fl_Widget*, void*)
{
    spInformation("<b>File Description</b><p>Not implemented yet, sorry. This is just a demo of the tree widget.</p>");
}

CTreeItem* CFileManagerTree::addDirectory(CTreeItem* parent, string title)
{
    parent->tree()->itemCreator(CFileListTreeFolder::itemCreator);
    CTreeItem* item = parent->addItem(title.c_str(), CTreeItem::folderOpened, CTreeItem::folderClosed);
    item->close();
    CFileListTreeItem* fileItem = (CFileListTreeItem*) item->body();
    fileItem->title(title);
    return item;
}

CTreeItem* CFileManagerTree::addFile(
        CTreeItem* parent, string title, string fileName, int64_t fileSize, string publisher, DateTime publishDate,
        unsigned progress)
{
    parent->tree()->itemCreator(CFileListTreeItem::itemCreator);
    CTreeItem* item = parent->addItem(title.c_str(), CTreeItem::document, CTreeItem::document);
    CFileListTreeItem* fileItem = (CFileListTreeItem*) item->body();
    fileItem->title(title);
    fileItem->fileName(fileName);
    fileItem->fileSize(fileSize);
    fileItem->progress(progress);
    fileItem->publisher(publisher);
    fileItem->publishDate(publishDate);
    return item;
}

void theme_cb(Fl_Widget* w, void*)
{
    try {
        CComboBox* themesCombo = (CComboBox*) w;
        if (themesCombo->eventType() == CE_DATA_CHANGED) {
            std::string themeName = themesCombo->data();

            CThemes::set
                    (themeName);

            CWindow* window = (CWindow*) w->window();
            window->relayout();
            window->redraw();
        }
    } catch (exception& e) {
        puts(e.what());
    }
}

unsigned CFileManagerTree::compareSortColumn = 999;
unsigned CFileManagerTree::compareSortInverse;

int CFileManagerTree::compareFileItems(const void* ti1, const void* ti2)
{
    CTreeItem* treeItem1 = *(CTreeItem**) ti1;
    CTreeItem* treeItem2 = *(CTreeItem**) ti2;
    int rc = 0;
    try {
        CFileListTreeItem* fileItem1 = dynamic_cast<CFileListTreeItem*>(treeItem1->body()->widget());
        CFileListTreeItem* fileItem2 = dynamic_cast<CFileListTreeItem*>(treeItem2->body()->widget());
        if (!fileItem1 || !fileItem2)
            return 0;

        switch (compareSortColumn) {
            case 0:rc = strcmp(fileItem1->title().c_str(), fileItem2->title().c_str());
                break;
            case 1: {
                int64_t rcl = fileItem1->fileSize() - fileItem2->fileSize();
                if (rcl > 0)
                    rc = 1;
                else if (rcl < 0)
                    rc = -1;
                else
                    rc = 0;
            }
                break;
            case 2:rc = strcmp(fileItem1->publisher().c_str(), fileItem2->publisher().c_str());
                break;
            case 3: {
                double rcd = fileItem1->publishDate() - fileItem2->publishDate();
                if (rcd > 0)
                    rc = 1;
                else if (rcd < 0)
                    rc = -1;
                else
                    rc = 0;
            }
                break;
            case 4:rc = fileItem1->progress() - fileItem2->progress();
                break;
        }
    } catch (...) {}
    if (compareSortInverse)
        return -rc;
    return rc;
}

void CFileManagerTree::sortFiles(CTreeItem* parentItem, unsigned column)
{
    unsigned firstFileWidget = 1;
    for (int i = 1; i < parentItem->children(); i++) {
        CTreeItem* treeItem = dynamic_cast<CTreeItem*>(parentItem->child(i));
        if (!treeItem)
            continue;
        CFileListTreeItem* fileItem = dynamic_cast<CFileListTreeItem*>(treeItem->body()->widget());
        if (fileItem) {
            firstFileWidget = i;
            break;
        }
    }
    void* base = (void*) (parentItem->array() + firstFileWidget);
    unsigned nmemb = parentItem->children() - firstFileWidget;
    qsort(base, nmemb, sizeof(void*), compareFileItems);
    //relayout();
}

CFileManagerTree::CFileManagerTree(CFileListTreeHeader* header)
        : CTreeControl("", 10, SP_ALIGN_CLIENT), m_db("local.db")
{
    m_header = header;
    end();
    fileManagerWidget = this;
    color(FL_LIGHT1);
}

void CFileManagerTree::position(int X, int Y)
{
    m_header->sync(X, root()->w());
    CTreeControl::position(X, Y);
}

void CFileManagerTree::resize(int X, int Y, int W, int H)
{
    m_header->sync(X, root()->w());
    CTreeControl::resize(X, Y, W, H);
}


void CFileManagerTree::refreshData()
{
    CTreeItem* item;

    CQuery folders(&m_db, "SELECT DISTINCT channel FROM content WHERE NOT channel='' ORDER BY 1");
    CQuery files(&m_db,
                 "SELECT title, filename, filesize, fileuint32_transferred, publisher, publish_date FROM content WHERE channel=:channel ORDER BY 1");
    folders.open();
    while (!folders.eof()) {
        item = addDirectory(root(), folders["channel"]);
        files.param("channel") = folders["channel"];
        files.open();
        while (!files.eof()) {
            int64_t filesize = files["filesize"];
            int64_t fileuint32_transferred = files["fileuint32_transferred"];
            unsigned percentage = unsigned(filesize ? 0 : fileuint32_transferred * 100.0 / filesize);
            DateTime publishDate = DateTime::convertCTime(files["publish_date"].asInteger());
            addFile(item, files["title"], files["filename"], filesize, files["publisher"], publishDate, percentage);
            files.next();
        }
        files.close();
        folders.next();
    }
    folders.close();

    //files.param("channel").setNull();
    files.sql(
            "SELECT title, filename, filesize, fileuint32_transferred, publisher, publish_date FROM content WHERE channel is NULL ORDER BY 1");
    files.open();
    int counter = 0;
    while (!files.eof()) {
        int64_t filesize = files["filesize"];
        int64_t fileuint32_transferred = files["fileuint32_transferred"];
        unsigned percentage = unsigned(filesize ? 0 : fileuint32_transferred * 100.0 / filesize);
        DateTime publishDate = DateTime::convertCTime(files["publish_date"].asInteger());
        addFile(root(), files["title"], files["filename"], filesize, files["publisher"], publishDate, percentage);
        files.next();
        counter++;
        if (counter > 10)
            break;
    }
    files.close();
    sortFiles(0);
}

void stripeFileItems(CTreeItem* treeItem)
{
    unsigned index = 0;
    for (int i = 0; i < treeItem->children(); i++) {
        try {
            CTreeItem* childItem = dynamic_cast<CTreeItem*>(treeItem->child(i));
            if (!childItem)
                continue;
            Fl_Widget* widget = childItem->body()->widget();
            CFileListTreeItem* item = dynamic_cast<CFileListTreeItem*>(widget);
            if (!item)
                continue;
            if (index % 2)
                widget->color(FL_LIGHT1);
            else
                widget->color(FL_DARK1);
            index++;
        } catch (...) {}
    }
}

void CFileManagerTree::sortFiles(unsigned column)
{
    compareSortInverse = (compareSortColumn == column) ? !compareSortInverse : false;
    compareSortColumn = column;
    int i;
    for (i = 0; i < root()->children(); i++) {
        CTreeItem* treeItem = dynamic_cast<CTreeItem*>(root()->child(i));
        if (!treeItem)
            continue;
        CFileListTreeFolder* folder = dynamic_cast<CFileListTreeFolder*>(treeItem->body()->widget());
        if (!folder)
            continue;
        sortFiles(treeItem, column);
    }
    sortFiles(root(), column);
    for (i = 0; i < root()->children(); i++) {
        CTreeItem* treeItem = 0;
        CFileListTreeFolder* file = 0;
        Fl_Widget* widget = 0;
        try {
            treeItem = dynamic_cast<CTreeItem*>(root()->child(i));
            if (!treeItem)
                continue;
            widget = treeItem->body()->widget();
            file = dynamic_cast<CFileListTreeFolder*>(widget);
            if (!file)
                continue;
        } catch (...) {}

        if (file->isFolder()) {
            file->color(color());
            stripeFileItems(treeItem);
        }
    }
    stripeFileItems(root());
    relayout();
    //redraw();
}

int main(int argc, char* argv[])
{
    try {
        CWindow window(600, 400);

        CFileManager fileManager;
        fileManager.refreshData();

        CComboBox themesCombo("Theme", 10, SP_ALIGN_BOTTOM);
        Strings themes = CThemes::availableThemes();
        themesCombo.addRows("Theme", themes);
        themesCombo.callback(theme_cb);
        themesCombo.data("Default");

        window.relayout();

        window.end();
        window.resizable(&window);

        window.show(argc, argv);
        Fl::run();
    } catch (exception& e) {
        spError(e.what());
    } catch (...) {
        spError("Unknown Error");
    }

    return 0;
}
