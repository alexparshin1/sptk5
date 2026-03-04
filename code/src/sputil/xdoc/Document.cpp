/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-04                                             ║
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

#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

namespace {

DataFormat autoDetectFormat(const char* data)
{
    switch (const auto skip = strspn(data, "\n\r\t ");
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

} // namespace

Document::Document(Node::Type rootType)
    : m_root(std::make_shared<Node>("", rootType))
{
}

Document::Document(const Document& other)
    : m_root(make_shared<Node>(""))
{
    Buffer buffer;
    other.exportTo(DataFormat::JSON, buffer, false);
    load(buffer);
}

Document::Document(Document&& other)
    : m_root(std::move(other.m_root))
{
    other.m_root = make_shared<Node>("");
}

Document& Document::operator=(const Document& other)
{
    if (this != &other)
    {
        Buffer buffer;
        other.exportTo(DataFormat::JSON, buffer, false);
        load(buffer);
    }
    return *this;
}

Document& Document::operator=(Document&& other)
{
    if (this != &other)
    {
        m_root = std::move(other.m_root);
        other.m_root = make_shared<Node>("");
    }
    return *this;
}

void Document::load(const Buffer& data, const bool xmlKeepFormatting)
{
    m_root->load(autoDetectFormat(data.c_str()), data, xmlKeepFormatting);
}

void Document::load(const String& data, bool xmlKeepFormatting)
{
    m_root->load(autoDetectFormat(data.c_str()), data, xmlKeepFormatting);
}
