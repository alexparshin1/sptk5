/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SourceModule.cpp - description                         ║
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

#include <sptk5/wsdl/SourceModule.h>
#include <fstream>

using namespace std;
using namespace sptk;

SourceModule::SourceModule(const String& moduleName, const String& modulePath)
: m_name(moduleName), m_path(modulePath)
{
}

SourceModule::~SourceModule()
{
    writeFile(m_name + ".h", Buffer(m_header.str()));
    writeFile(m_name + ".cpp", Buffer(m_source.str()));
}

void SourceModule::open()
{
    if (m_path.empty())
        m_path = ".";
    m_header.str("");
    m_source.str("");
}

ostream& SourceModule::header()
{
    return m_header;
}

ostream& SourceModule::source()
{
    return m_source;
}

void SourceModule::writeFile(const String& fileNameAndExtension, const Buffer& data)
{
    Buffer existingData;

    if (m_path.empty())
        m_path = ".";
    String fileName = m_path + "/" + fileNameAndExtension;

    try {
        existingData.loadFromFile(fileName);
    }
    catch (const Exception&) {
        existingData.bytes(0);
    }

    if (strcmp(existingData.c_str(), data.c_str()) == 0)
        return;

    ofstream file;
    file.open(fileName.c_str(), ofstream::out | ofstream::trunc);
    if (!file.is_open())
        throw Exception("Can't create file " + fileNameAndExtension);
    file.write(data.c_str(), data.bytes());
    file.close();
}
