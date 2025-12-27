/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/xdoc/Node.h>

namespace sptk::xdoc {

class SP_EXPORT ExportJSON
{
public:
    /**
     * Export to JSON text
     * @param node              Output node
     * @param formatted         Format JSON output
     */
    static void exportToJSON(const Node* node, sptk::Buffer& json, bool formatted);

private:
    struct Formatting
    {
        String firstElement;
        String betweenElements {","};
        String newLineChar;
        String indentSpaces;
    };

    static void exportJsonValueTo(const Node* node, std::ostream& stream, bool formatted, size_t indent);

    static void exportJsonArray(const Node* node, std::ostream& stream, bool formatted, size_t indent,
                                const Formatting& formatting);

    static void exportJsonObject(const Node* node, std::ostream& stream, bool formatted, size_t indent,
                                 const Formatting& formatting);

    static void exportNodeAttributes(const Node* node, std::ostream& stream, bool formatted, const String& firstElement);
};

} // namespace sptk::xdoc
