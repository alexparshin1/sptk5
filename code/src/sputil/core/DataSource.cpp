/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-10                                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
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

void DataSource::exportRowTo(const xdoc::SNode& node, const bool compactXmlMode, const bool nullLargeData)
{
    const auto cnt = fieldCount();
    for (size_t i = 0; i < cnt; ++i)
    {
        const Field& field = operator[](i);
        field.exportTo(node, compactXmlMode, nullLargeData);
    }
}

void DataSource::exportTo(xdoc::Node& parentNode, const String& nodeName, const bool compactXmlMode)
{
    try
    {
        if (open())
        {
            first();
            while (!eof())
            {
                const auto& node = parentNode.pushNode(nodeName, xdoc::Node::Type::Object);
                exportRowTo(node, compactXmlMode, false);
                next();
            }
            close();
        }
    }
    catch (...)
    {
        try
        {
            close();
        }
        catch (...)
        {
            // Prevent re-throwing exceptions from close() method.
        }
        throw;
    }
}
