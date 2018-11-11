/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       xml_test2.cpp - description                            ║
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

#include <sptk5/sptk.h>
#include <sptk5/cxml>
#include <fstream>

using namespace std;
using namespace sptk;
using namespace chrono;

int main()
{
    ostream& out = cout;
    //out.open("xml_test3.log",ofstream::out);

    try {
        xml::Document doc;

        DateTime start = DateTime::Now();
        DateTime end;
        auto *buf = new Buffer;

        out << "The XML document generation test started:" << endl;
        out << "Size of Node is : " << sizeof(xml::Node) << endl;
        out << "Size of Element is : " << sizeof(xml::Element) << endl;
        out << "Size of Attributes is : " << sizeof(xml::Attributes) << endl;
        out << "Size of NodeList is : " << sizeof(xml::NodeList) << endl;
        char buffer[64];
        string rowTag("row"), cellTag("cell");
        unsigned nodesPerRow = 7;
        for (unsigned i = 0; i < 100000; i++) {
            xml::Node* row = new xml::Element(doc, rowTag);

            snprintf(buffer, sizeof(buffer), "%u", i);
            xml::Node* cell1 = new xml::Element(*row, cellTag);
            cell1->setAttribute("column", 1);
            new xml::Text(*cell1, buffer);

            snprintf(buffer, sizeof(buffer), "A pretty long string number %u", i);
            xml::Node* cell2 = new xml::Element(*row, cellTag);
            cell2->setAttribute("column", 2);
            new xml::Text(*cell2, buffer);
        }

        out.setf(ios::fixed);
        out.precision(2);

        end = DateTime::Now();
        DateTime::duration duration = end - start;
        start = end;

        out << "The document is ready (" << doc.size() * nodesPerRow << " nodes): " << duration_cast<milliseconds>(duration).count() / 1000.0 << " seconds" << endl;

        doc.save(*buf, true);
        buf->saveToFile("0.xml");

        end = DateTime::Now();
        duration = end - start;
        start = end;
        out << "The document is saved to buffer (" << doc.size() * nodesPerRow << " nodes): " << duration_cast<milliseconds>(duration).count() / 1000.0 << " seconds" << endl;

        doc.clear();

        end = DateTime::Now();
        duration = end - start;
        start = end;
        out << "The document is cleared (" << doc.size() * nodesPerRow << " nodes): " << duration_cast<milliseconds>(duration).count() / 1000.0 << " seconds" << endl;

        doc.load(*buf);
        end = DateTime::Now();
        duration = end - start;
        start = end;
        out << "The document is loaded from the buffer (" << doc.size() * nodesPerRow << " nodes): " << duration_cast<milliseconds>(duration).count() / 1000.0 << " seconds" << endl;

        doc.save(*buf, true);

        end = DateTime::Now();
        duration = end - start;
        start = end;
        out << "The document is saved to buffer(" << doc.size() * nodesPerRow << " nodes): " << duration_cast<milliseconds>(duration).count() / 1000.0 << " seconds" << endl;

        buf->saveToFile("1.xml");

        end = DateTime::Now();
        duration = end - start;
        start = end;
        out << "The document is saved to disk: " << duration_cast<milliseconds>(duration).count() / 1000.0 << " seconds" << endl;

        out << "The XML document generation test completed." << endl;

        // Releasing the buffer memory
        delete buf;

    }
    catch (std::exception& e) {
        out << e.what() << endl;
        return 1;
    }
    return 0;
}
