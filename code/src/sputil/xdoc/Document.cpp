/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Printer.h>
#include <sptk5/StopWatch.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

static DataFormat autoDetectFormat(const char* data)
{
    switch (auto skip = strspn(data, "\n\r\t ");
            data[skip])
    {
        case '<':
            return DataFormat::XML;
        case '[':
        case '{':
            return DataFormat::JSON;
        default:
            break;
    }
    throw Exception("Invalid character at the data start");
}

void Document::load(const Buffer& data, bool xmlKeepFormatting) const
{
    m_root->load(autoDetectFormat(data.c_str()), data, xmlKeepFormatting);
}

void Document::load(const String& data, bool xmlKeepFormatting) const
{
    m_root->load(autoDetectFormat(data.c_str()), data, xmlKeepFormatting);
}

