/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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

#include <fstream>
#include <sptk5/Printer.h>
#include <sptk5/wsdl/SourceModule.h>
#include <utility>

using namespace std;
using namespace sptk;

SourceModule::SourceModule(String moduleName, String modulePath)
    : m_name(std::move(moduleName))
    , m_path(std::move(modulePath))
{
}

void SourceModule::reset()
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

    const filesystem::path fileName = m_path + "/" + fileNameAndExtension;

    try
    {
        existingData.loadFromFile(fileName);
    }
    catch (const Exception&)
    {
        existingData.bytes(0);
    }

    if (existingData == data)
    {
        return;
    }

    const auto dirname = fileName.parent_path();
    if (!filesystem::exists(dirname))
    {
        if (!filesystem::create_directories(dirname))
        {
            throw Exception("Cannot create directory " + dirname.string());
        }
    }

    data.saveToFile(fileName);
}

void SourceModule::writeOutputFiles()
{
    writeFile(m_name + ".h", Buffer(m_header.str()));
    writeFile(m_name + ".cpp", Buffer(m_source.str()));
}
