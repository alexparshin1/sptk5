/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       xml_test1.cpp - description                            ║
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

#include <FL/Fl.H>

#include <sptk5/DateTime.h>
#include <sptk5/cutils>
#include <sptk5/cgui>

using namespace std;
using namespace sptk;

void build_tree(xml::Element *n, CTreeControl *tree, CTreeItem *item)
{
    if (!n)
        return;

    CTreeItem *w = nullptr;
    CTreeItem *newItem = nullptr;
    if (!n->empty() || n->type() & (xml::Node::DOM_CDATA_SECTION | xml::Node::DOM_COMMENT)) {
        // Create a new item group
        if (item)
            newItem = item->addItem("", nullptr, nullptr, n);
        else
            newItem = tree->addItem("", nullptr, nullptr, n);
        w = newItem;
        if (n->type() & (xml::Node::DOM_CDATA_SECTION | xml::Node::DOM_COMMENT)) {
            w->label(n->name().c_str());
            w = newItem->addItem("", nullptr, nullptr, n);
        }
    } else {
        if (item)
            newItem = item->addItem("", nullptr, nullptr, n);
        else
            newItem = tree->addItem("", nullptr, nullptr, n);
        w = newItem;
    }

    String label;
    const xml::Attributes &attr_map = n->attributes();

    switch (n->type())
    {
    case xml::Node::DOM_ELEMENT:
        label = n->name();
        if (n->hasAttributes()) {
            auto it = attr_map.begin();
            for (; it != attr_map.end(); it++)
                label += string(" ") + (*it)->name() + string("=*") + (*it)->value() + "*";
        }
        break;

    case xml::Node::DOM_PI:
        label = n->name();
        label += ": ";
        label += n->value();
        break;

    case xml::Node::DOM_DOCUMENT:
        label = n->name();
        break;

    default:
        label = n->value();
        break;
    }

    w->label(label.c_str());

    auto itor = n->begin();
    auto iend = n->end();
    for (; itor != iend; ++itor) {
        auto* node = dynamic_cast<xml::Element*>(*itor);
        if (node)
            build_tree(node, tree, newItem);
    }
}

xml::Document *build_doc()
{
    auto* doc = new xml::Document();

    xml::Node *rootNode = new xml::Element(*doc, "MyDocument");
    xml::Node *hello = new xml::Element(*rootNode, "HelloTag");
    new xml::Element(*hello, "Hello all!");

    try {
        Buffer savebuffer;
        doc->save(savebuffer, true);
        savebuffer.saveToFile("MyXML2.xml");
    }
    catch (const Exception& e) {
        Fl::warning(e.what());
    }

    return doc;
}

double diffSeconds(DateTime start, DateTime end)
{
    return chrono::duration_cast<chrono::milliseconds>(end-start).count() / 1000.0;
}

void saveDocument(const shared_ptr<xml::Document>& doc)
{
    try {
            DateTime start("now");
            Buffer savebuffer;
            doc->save(savebuffer, true);
            savebuffer.saveToFile("MyXML.xml");
            DateTime end("now");
            COUT("XML Test - saved for " << diffSeconds(start, end) << " sec" << endl)
        }
        catch (const Exception& e) {
            Fl::warning(e.what());
        }
}

int main(int argc, char **argv)
{
    try {
        // Initialize themes
        CThemes themes;

        String fileName;
        if (argc == 2) {
            fileName = argv[1];
        } else {
            CFileOpenDialog dialog;
            dialog.directory(".");
            dialog.addPattern("XML Files", "*.xml");
            dialog.addPattern("All Files", "*.*");
            dialog.setPattern("XML Files");
            if (dialog.execute())
                fileName = dialog.fullFileName();
        }

        if (fileName.empty())
            return -1;

        Buffer buffer;
        buffer.loadFromFile(fileName.c_str());

        auto* window = new CWindow(700, 200, 300, 300);
        window->resizable(window);
        window->begin();

        auto* tree = new CTreeControl("Tree", 10, SP_ALIGN_CLIENT);
        window->end();

        DateTime start = DateTime::Now();
        shared_ptr<xml::Document> doc(new xml::Document);
        doc->load(buffer);
        DateTime end = DateTime::Now();

        stringstream message;
        message << "XML Test - loaded file in " << diffSeconds(start, end) << " sec";
        window->label(message.str());
        COUT(message.str() << endl)

        build_tree(doc.get(), tree, nullptr);
        start = DateTime::Now();
        tree->relayout();
        end = DateTime::Now();
        COUT("XML Test - relayouted tree in " << diffSeconds(start, end) << " sec" << endl)

        saveDocument(doc);

        window->show();

        Fl::run();

        COUT("--------------------------------" << endl)
    }
    catch (const Exception& e) {
        CERR(e.what() << endl)
        return 1;
    }
    return 0;
}
