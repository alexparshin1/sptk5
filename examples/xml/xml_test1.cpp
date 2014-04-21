/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          cxml_test1.cpp  -  description
                             -------------------
    begin                : August 10, 2003
    copyright            : (C) 1999-2013 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <FL/fl_ask.H>
#include <FL/Fl.h>

#include <sptk5/CDateTime.h>
#include <sptk5/cgui>
#include <sptk5/cxml>

using namespace std;
using namespace sptk;

void build_tree(CXmlElement *n, CTreeControl *tree, CTreeItem *item)
{
    if (!n)
        return;

    CTreeItem *w = 0;
    CTreeItem *newItem = 0;
    if (n->size() || n->type() & (CXmlNode::DOM_CDATA_SECTION | CXmlNode::DOM_COMMENT)) {
        // Create a new item group
        if (item)
            newItem = item->addItem("", 0L, 0L, n);
        else
            newItem = tree->addItem("", 0L, 0L, n);
        w = newItem;
        if (n->type() & (CXmlNode::DOM_CDATA_SECTION | CXmlNode::DOM_COMMENT)) {
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
    const CXmlAttributes &attr_map = n->attributes();

    switch (n->type())
    {
    case CXmlNode::DOM_ELEMENT:
        label = n->name();
        if (n->hasAttributes()) {
            CXmlAttributes::const_iterator it = attr_map.begin();
            for (; it != attr_map.end(); it++)
                label += string(" ") + (*it)->name() + string("=*") + (*it)->value() + "*";
        }
        break;

    case CXmlNode::DOM_PI:
        label = n->name();
        label += ": ";
        label += n->value();
        break;

    case CXmlNode::DOM_DOCUMENT:
        label = n->name();
        break;

    default:
        label = n->value();
        break;
    };

    w->label(label.c_str());

    CXmlNode::iterator itor = n->begin();
    CXmlNode::iterator iend = n->end();
    for (; itor != iend; itor++) {
        CXmlElement* node = dynamic_cast<CXmlElement*>(*itor);
        if (node)
            build_tree(node, tree, newItem);
    }
}

CXmlDoc *build_doc()
{
    CXmlDoc *doc = new CXmlDoc();

    CXmlNode *rootNode = new CXmlElement(*doc, "MyDocument");
    CXmlNode *hello = new CXmlElement(*rootNode, "HelloTag");
    new CXmlElement(*hello, "Hello all!");

    try {
        CBuffer savebuffer;
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

    CBuffer buffer;
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

        CDateTime start = CDateTime::Now();
        CXmlDoc *doc = new CXmlDoc;
        doc->load(buffer);
        CDateTime end = CDateTime::Now();

        char message[128];
        sprintf(message, "XML Test - loaded file in %0.2f sec", ((double) end - double(start)) * 24 * 3600);
        window->label(message);
        puts(message);

        build_tree(doc, tree, 0L);
        start = CDateTime::Now();
        tree->relayout();
        end = CDateTime::Now();
        printf("XML Test - relayouted tree in %0.2f sec\n", ((double) end - double(start)) * 24 * 3600);

        try {
            CDateTime start = CDateTime::Now();
            CBuffer savebuffer;
            doc->save(savebuffer);
            savebuffer.saveToFile("MyXML.xml");
            end = CDateTime::Now();
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
