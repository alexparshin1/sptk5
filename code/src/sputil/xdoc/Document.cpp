/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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

Document::Document(const Node::Type rootType)
    : m_root(Node::createNode("", rootType))
{
}

Document::Document(const Document& other)
    : m_root(Node::createNode(other.m_root))
{
}

Document::Document(Document&& other) noexcept
    : m_root(std::move(other.m_root))
{
    other.m_root = Node::createNode("");
}

Document& Document::operator=(const Document& other)
{
    if (this != &other)
    {
        const auto root = Node::createNode(other.m_root);
        m_root = root;
    }
    return *this;
}

Document& Document::operator=(Document&& other) noexcept
{
    if (this != &other)
    {
        m_root = std::move(other.m_root);
        other.m_root = Node::createNode("");
    }
    return *this;
}

void Document::load(const Buffer& data, const bool xmlKeepFormatting)
{
    m_root->load(autoDetectFormat(data.c_str()), data, xmlKeepFormatting);
}

void Document::load(const String& data, const bool xmlKeepFormatting)
{
    m_root->load(autoDetectFormat(data.c_str()), data, xmlKeepFormatting);
}
