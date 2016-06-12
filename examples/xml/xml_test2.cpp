/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       xml_test2.cpp - description                            ║
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

#include <sptk5/sptk.h>
#include <sptk5/cxml>
#include <iostream>
#include <fstream>

using namespace std;
using namespace sptk;

int main()
{
    ostream& out = cout;
    //out.open("xml_test3.log",ofstream::out);

    try {
        CXmlDoc doc;

        CDateTime start = CDateTime::Now();
        CDateTime end;
        double duration;
        CBuffer *buf = new CBuffer;

        out << "The XML document generation test started:" << endl;
        out << "Size of CXmlNode is : " << sizeof(CXmlNode) << endl;
        out << "Size of CXmlElement is : " << sizeof(CXmlElement) << endl;
        out << "Size of CXmlAttributes is : " << sizeof(CXmlAttributes) << endl;
        out << "Size of CXmlNodeList is : " << sizeof(CXmlNodeList) << endl;
        char buffer[64];
        string rowTag("row"), cellTag("cell");
        unsigned nodesPerRow = 7;
        for (unsigned i = 0; i < 100000; i++) {
            CXmlNode* row = new CXmlElement(doc, rowTag);

            sprintf(buffer, "%i", i);
            CXmlNode* cell1 = new CXmlElement(*row, cellTag);
            cell1->setAttribute("column", 1);
            new CXmlText(*cell1, buffer);

            sprintf(buffer, "A pretty long string number %i", i);
            CXmlNode* cell2 = new CXmlElement(*row, cellTag);
            cell2->setAttribute("column", 2);
            new CXmlText(*cell2, buffer);
        }

        out.setf(ios::fixed);
        out.precision(2);

        end = CDateTime::Now();
        duration = (end - start) * 3600 * 24;
        start = end;

        out << "The document is ready (" << doc.size() * nodesPerRow << " nodes): " << duration << " seconds" << endl;

        doc.save(*buf);
        buf->saveToFile("0.xml");

        end = CDateTime::Now();
        duration = (end - start) * 3600 * 24;
        start = end;
        out << "The document is saved to buffer (" << doc.size() * nodesPerRow << " nodes): " << duration << " seconds" << endl;

        doc.clear();

        end = CDateTime::Now();
        duration = (end - start) * 3600 * 24;
        start = end;
        out << "The document is cleared (" << doc.size() * nodesPerRow << " nodes): " << duration << " seconds" << endl;

        doc.load(*buf);
        end = CDateTime::Now();
        duration = (end - start) * 3600 * 24;
        start = end;
        out << "The document is loaded from the buffer (" << doc.size() * nodesPerRow << " nodes): " << duration << " seconds" << endl;

        doc.save(*buf);

        end = CDateTime::Now();
        duration = (end - start) * 3600 * 24;
        start = end;
        out << "The document is saved to buffer(" << doc.size() * nodesPerRow << " nodes): " << duration << " seconds" << endl;

        buf->saveToFile("1.xml");

        end = CDateTime::Now();
        duration = (end - start) * 3600 * 24;
        start = end;
        out << "The document is saved to disk: " << duration << " seconds" << endl;

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
