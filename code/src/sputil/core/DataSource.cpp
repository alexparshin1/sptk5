/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DataSource.cpp - description                           ║
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

#include <sptk5/DataSource.h>

using namespace std;
using namespace sptk;

bool DataSource::load()
{
    // Loading data into DS
    return loadData();
}

bool DataSource::save()
{
    // Storing data from DS
    return saveData();
}

void DataSource::rowToXML(xml::Node& node, bool compactXmlMode) const
{
    uint32_t cnt = fieldCount();
    for (uint32_t i = 0; i < cnt; i++) {
        const Field& field = operator[](i);
        field.toXML(node, compactXmlMode);
    }
}

void DataSource::toXML(xml::Node& parentNode, const string& nodeName, bool compactXmlMode)
{
    try {
        open();
        while (!eof()) {
            xml::Node& node = *(new xml::Element(parentNode, nodeName));
            rowToXML(node, compactXmlMode);
            next();
        }
        close();
    }
    catch (const Exception&) {
        close();
        throw;
    }
}
