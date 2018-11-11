/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CGtkThemeLoader.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/sptk.h>

#include "CGtkThemeLoader.h"

using namespace std;
using namespace sptk;

namespace sptk {

    static const Strings notGroupingTags("styles;style;engine", ";");

    xml::Node* CGtkThemeParser::parseParameter(const String& row, xml::Node* parentNode, bool createAttributes)
    {
        try {
            size_t pos = row.find_first_of(":[ \t=");
            String name = row.substr(0, pos);

            String subName;
            pos = row.find_first_not_of(" \t", pos);
            if (pos == STRING_NPOS)
                throw Exception("value not found");

            switch (row[pos]) {
                case ':': {
                    if (row[pos + 1] != ':')
                        throw Exception("single ':' found");
                    pos += 2;
                    size_t pos2 = row.find_first_of(" \t=", pos);
                    if (pos2 == STRING_NPOS)
                        throw Exception("value not found");
                    subName = row.substr(pos, pos2 - pos);
                    pos = pos2 + 1;
                    break;
                }
                case '[': {
                    pos++;
                    size_t pos2 = row.find_first_of("]", pos);
                    if (pos2 == STRING_NPOS)
                        throw Exception("matching ']' not found");
                    subName = row.substr(pos, pos2 - pos);
                    pos = pos2 + 1;
                    break;
                }
                case '\"': {
                    pos++;
                    size_t pos2 = row.find_first_of("\"", pos);
                    if (pos2 == STRING_NPOS)
                        throw Exception("matching '\"' not found");
                    subName = row.substr(pos, pos2 - pos);
                    pos = pos2 + 1;
                    break;
                }
                default:
                    break;
            }

            bool valueMode = false;
            size_t pos2 = row.find_first_of("=", pos);
            if (pos2 != STRING_NPOS) {
                valueMode = true;
                pos = pos2 + 1;
                pos2 = row.find_first_not_of(" \t", pos);
                if (pos2 == STRING_NPOS)
                    throw Exception("error parsing value");
                pos = pos2;
            }
            int maxValueSize = 16384;
            if (row[pos] == '\"') {
                pos++;
                pos2 = row.find_first_of("\"", pos);
                if (pos2 == STRING_NPOS)
                    throw Exception("Error parsing value for " + name + " in row " + row);
                maxValueSize = int(pos2 - pos);
            }
            xml::Node* node = nullptr;
            String value = trim(row.substr(pos, (unsigned) maxValueSize));
            bool attemptGrouping = notGroupingTags.indexOf(name) < 0;
            if (!attemptGrouping)
                valueMode = false;
            if (valueMode) {
                if (createAttributes) {
                    parentNode->setAttribute(name, value);
                    node = parentNode;
                } else {
                    node = parentNode->findFirst(name);
                    if (!node)
                        node = new xml::Element(parentNode, name.c_str());
                    if (!subName.empty())
                        node->setAttribute(subName, value);
                    else
                        node->setAttribute("value", value);
                }
            } else {
                if (attemptGrouping)
                    node = parentNode->findFirst(name);
                if (!node)
                    node = new xml::Element(parentNode, name.c_str());
                if (!subName.empty())
                    node->setAttribute("name", subName);
                if (!value.empty())
                    node->setAttribute("value", value);
            }
            return node;
        }
        catch (exception& e) {
            throw Exception("Error parsing row '" + row + "'\n" + string(e.what()));
        }
        return nullptr;
    }

    void CGtkThemeParser::parseImage(const Strings& gtkrc, unsigned& currentRow, xml::Node* parentNode)
    {
        if (gtkrc[currentRow] != "image")
            throw Exception("Expecting 'image' in row " + gtkrc[currentRow]);
        currentRow++;
        if (gtkrc[currentRow] != "{")
            throw Exception("Expecting '{' in row '" + gtkrc[currentRow]);
        currentRow++;
        xml::Node* imageNode = new xml::Element(parentNode, "image");
        while (gtkrc[currentRow] != "}") {
            parseParameter(gtkrc[currentRow], imageNode, true);
            currentRow++;
            if (currentRow == gtkrc.size())
                throw Exception("Expecting '}' after row '" + gtkrc[currentRow - 1]);
        }
    }

    void CGtkThemeParser::parseEngine(const Strings& gtkrc, unsigned& currentRow, xml::Node* parentNode)
    {
        if (gtkrc[currentRow].find("engine") != 0)
            throw Exception("Expecting 'engine' in row " + gtkrc[currentRow]);
        xml::Node* engineNode = parseParameter(gtkrc[currentRow++], parentNode);
        try {
            if (gtkrc[currentRow] != "{")
                throw Exception("Expecting '{' in row '" + gtkrc[currentRow] + "'");
            currentRow++;
            while (gtkrc[currentRow] != "}") {
                if (gtkrc[currentRow] == "image")
                    parseImage(gtkrc, currentRow, engineNode);
                else
                    parseParameter(gtkrc[currentRow], engineNode);
                currentRow++;
                if (currentRow == gtkrc.size())
                    throw Exception("Expecting '}' after row '" + gtkrc[currentRow - 1] + "'");
            }
        }
        catch (exception& e) {
            cerr << "Error parsing engine '" << engineNode->getAttribute("name", "").str() << "': " << e.what() << endl;
        }
    }

    void CGtkThemeParser::parseStyle(const Strings& gtkrc, unsigned& currentRow, xml::Node* parentNode)
    {
        //const string& styleRow = gtkrc[currentRow];
        if (gtkrc[currentRow].find("style") != 0)
            throw Exception("Expecting 'style' in row " + gtkrc[currentRow]);
        xml::Node* styleNode = parseParameter(gtkrc[currentRow++], parentNode);
        if (styleNode->getAttribute("name").str() == "scrollbar")
            styleNode->setAttribute("name", "scrollbars");
        if (gtkrc[currentRow] != "{")
            throw Exception("Expecting '{' in row '" + gtkrc[currentRow] + "'");
        currentRow++;
        while (gtkrc[currentRow] != "}") {
            const string& str = gtkrc[currentRow];
            if (str.compare(0, 7, "engine ") == 0)
                parseEngine(gtkrc, currentRow, styleNode);
            else
                parseParameter(str, styleNode);
            currentRow++;
            if (currentRow == gtkrc.size())
                throw Exception("Expecting '}' after row '" + gtkrc[currentRow - 1] + "'");
        }
    }

    void CGtkThemeParser::parse(const Strings& gtkrc)
    {
        Buffer buffer;
        m_xml.clear();
        xml::Node* stylesNode = new xml::Element(&m_xml, "styles");
        //Node* paramsNode = new Element(&m_xml,"styles");
        for (unsigned row = 0; row < gtkrc.size(); row++) {
            const string& str = gtkrc[row];
            if (str.compare(0, 6, "style ") == 0)
                parseStyle(gtkrc, row, stylesNode);
            else
                parseParameter(str, &m_xml);
        }
        m_xml.save(buffer, true);
        buffer.saveToFile("gtkrc.xml");
    }

    void CGtkThemeParser::load(const string& themeName)
    {
        m_themeFolder = Registry::homeDirectory() + ".themes/" + themeName + "/gtk-2.0/";
        string gtkrcFile = m_themeFolder + "gtkrc";
        Strings gtkrcSource;

        try {
            gtkrcSource.loadFromFile(gtkrcFile);
        }
        catch (...) {
            m_themeFolder = "/usr/share/themes/" + themeName + "/gtk-2.0/";
            gtkrcFile = m_themeFolder + "gtkrc";
            gtkrcSource.loadFromFile(gtkrcFile);
        }

        Strings gtkrc;

        for (unsigned i = 0; i < gtkrcSource.size(); i++) {
            String s = trim(gtkrcSource[i]);

            size_t pos = 0;

            // Remove comments (if any)
            if (s.find_first_of('#') != STRING_NPOS) {
                for (;;) {
                    pos = s.find_first_of("#\"", pos + 1);
                    if (pos == string::npos)
                        break;
                    if (s[pos] == '\"') { // Find a matching double quote
                        pos = s.find_first_of('\"', pos + 1);
                        if (pos == STRING_NPOS)
                            throw Exception("Unmatched {\"} found in row " + int2string(i));
                    } else {
                        if (pos)
                            s = trim(s.substr(0, pos - 1));
                        else
                            s = "";
                        break;
                    }
                }
            }

            pos = 0;
            while (pos != STRING_NPOS && !s.empty()) {
                if (s[s.size() - 1] == '{') {
                    s = trim(s.substr(0, s.size() - 1));
                    if (!s.empty())
                        gtkrc.push_back(s);
                    gtkrc.push_back("{");
                    pos = STRING_NPOS;
                    continue;
                }
                if (!s.empty())
                    gtkrc.push_back(s);
                break;
            }
        }

        parse(gtkrc);
    }

}
