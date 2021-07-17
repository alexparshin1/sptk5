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

#pragma once

#include "Node.h"

namespace sptk::xdoc {

class Document
{
public:
    explicit Document(Node::Type rootType = Node::Type::Object)
        : m_root(std::make_shared<Node>("", rootType))
    {
    }

    SNode& root()
    {
        return m_root;
    }

    const SNode& root() const
    {
        return m_root;
    }

    void load(DataFormat dataFormat, const Buffer& data, bool xmlKeepFormatting = false)
    {
        m_root->load(dataFormat, data, xmlKeepFormatting);
    }

    void load(DataFormat dataFormat, const String& data, bool xmlKeepFormatting = false)
    {
        m_root->load(dataFormat, data, xmlKeepFormatting);
    }

    void exportTo(DataFormat dataFormat, Buffer& data, bool formatted) const
    {
        m_root->exportTo(dataFormat, data, formatted);
    }

private:
    SNode m_root;
};

}
