/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       xml_test1.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <FL/fl_ask.H>
#include <FL/Fl.H>

#include <sptk5/DateTime.h>
#include <sptk5/cgui>
#include <sptk5/cxml>

using namespace std;
using namespace sptk;

void build_tree(XMLElement *n, CTreeControl *tree, CTreeItem *item)
{
    if (!n)
        return;

    CTreeItem *w = 0;
    CTreeItem *newItem = 0;
    if (n->size() || n->type() & (XMLNode::DOM_CDATA_SECTION | XMLNode::DOM_COMMENT)) {
        // Create a new item group
        if (item)
            newItem = item->addItem("", 0L, 0L, n);
        else
            newItem = tree->addItem("", 0L, 0L, n);
        w = newItem;
        if (n->type() & (XMLNode::DOM_CDATA_SECTION | XMLNode::DOM_COMMENT)) {
            w->label(n->name().c_str());
            w = newItem->addItem("", 0L, 0L, n);
        }
    } else {
        if (item)
            newItem = item->addItem("", 0L, 0L, n);
        else
            newItem = tree->addItem("", 0L, 0L, n);
        w = newItem;
    }

    string label;
    const XMLAttributes &attr_map = n->attributes();

    switch (n->type())
    {
    case XMLNode::DOM_ELEMENT:
        label = n->name();
        if (n->hasAttributes()) {
            XMLAttributes::const_iterator it = attr_map.begin();
            for (; it != attr_map.end(); it++)
                label += string(" ") + (*it)->name() + string("=*") + (*it)->value() + "*";
        }
        break;

    case XMLNode::DOM_PI:
        label = n->name();
        label += ": ";
        label += n->value();
        break;

    case XMLNode::DOM_DOCUMENT:
        label = n->name();
        break;

    default:
        label = n->value();
        break;
    };

    w->label(label.c_str());

    XMLNode::iterator itor = n->begin();
    XMLNode::iterator iend = n->end();
    for (; itor != iend; itor++) {
        XMLElement* node = dynamic_cast<XMLElement*>(*itor);
        if (node)
            build_tree(node, tree, newItem);
    }
}

XMLDocument *build_doc()
{
    XMLDocument *doc = new XMLDocument();

    XMLNode *rootNode = new XMLElement(*doc, "MyDocument");
    XMLNode *hello = new XMLElement(*rootNode, "HelloTag");
    new XMLElement(*hello, "Hello all!");

    try {
        Buffer savebuffer;
        doc->save(savebuffer);
        savebuffer.saveToFile("MyXML2.xml");
    }
    catch (...) {
        Fl::warning("Error!");
    }

    return doc;
}

extern int autoLayoutCounter;

int main(int argc, char **argv)
{
    // Initialize themes
    CThemes themes;

    string fileName;
    if (argc == 2) {
        fileName = argv[1];
    } else {
        CFileOpenDialog dialog;
        dialog.directory(".");
        dialog.addPattern("XML Files", "*.xml");
        dialog.addPattern("All Files", "*.*");
        dialog.setPattern("XML Files");
        dialog.execute();
        fileName = dialog.fullFileName();
    }

    if (fileName.empty())
        return -1;

    Buffer buffer;
    try {
        buffer.loadFromFile(fileName.c_str());
    }
    catch (exception& e) {
        puts(e.what());
        return 12;
    }

    CWindow *window = new CWindow(700, 200, 300, 300);
    window->resizable(window);
    window->begin();

    CTreeControl *tree = new CTreeControl("Tree", 10, SP_ALIGN_CLIENT);
    window->end();

    try {

        DateTime start = DateTime::Now();
        XMLDocument *doc = new XMLDocument;
        doc->load(buffer);
        DateTime end = DateTime::Now();

        char message[128];
        snprintf(message, sizeof(message), "XML Test - loaded file in %0.2f sec", ((double) end - double(start)) * 24 * 3600);
        window->label(message);
        puts(message);

        build_tree(doc, tree, 0L);
        start = DateTime::Now();
        tree->relayout();
        end = DateTime::Now();
        printf("XML Test - relayouted tree in %0.2f sec\n", ((double) end - double(start)) * 24 * 3600);

        try {
            DateTime start = DateTime::Now();
            Buffer savebuffer;
            doc->save(savebuffer);
            savebuffer.saveToFile("MyXML.xml");
            end = DateTime::Now();
            printf("XML Test - saved for %0.2f sec\n", ((double) end - double(start)) * 24 * 3600);
        }
        catch (...) {
            Fl::warning("Error!");
        }
        delete doc;
    }
    catch (std::exception &e) {
        puts("");
        puts("------------------------");
        puts(e.what());
        puts("------------------------");
    }
    catch (...) {
        puts("");
        puts("------------------------");
        puts("Unknown error");
        puts("------------------------");
    }

    //window->relayout();

    window->show();

    Fl::run();

    cout << "--------------------------------" << endl;
    cout << "There were " << autoLayoutCounter << " calls to autoLayout()" << endl;

    return 0;
}
