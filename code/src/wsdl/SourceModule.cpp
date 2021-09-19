/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <fstream>
#include <sptk5/Printer.h>
#include <sptk5/wsdl/SourceModule.h>
#include <utility>

using namespace std;
using namespace sptk;

SourceModule::SourceModule(String moduleName, const String& modulePath)
    : m_name(std::move(moduleName))
    , m_path(modulePath)
{
}

void SourceModule::open()
{
    if (m_path.empty())
    {
        m_path = ".";
    }
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
    Buffer existingData((const uint8_t*) "", 1);

    if (m_path.empty())
    {
        m_path = ".";
    }
    String fileName = m_path + "/" + fileNameAndExtension;

    try
    {
        existingData.loadFromFile(fileName.c_str());
    }
    catch (const Exception&)
    {
        existingData.bytes(0);
    }

    if (strcmp(existingData.c_str(), data.c_str()) == 0)
    {
        return;
    }

    data.saveToFile(fileName.c_str());
}

void SourceModule::writeOutputFiles()
{
    writeFile(m_name + ".h", Buffer(m_header.str()));
    writeFile(m_name + ".cpp", Buffer(m_source.str()));
}
