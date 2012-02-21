/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CGtkThemeLoader.h  -  description
                             -------------------
    begin                : Thu May 22 2008
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include "CGtkThemeLoader.h"

using namespace std;
using namespace sptk;

namespace sptk {

static const CStrings notGroupingTags("styles;style;engine",";");

CXmlNode* CGtkThemeParser::parseParameter(const std::string& row,CXmlNode* parentNode,bool createAttributes) {
    try {
        int pos = row.find_first_of(":[ \t=");
        string name = row.substr(0,pos);

        string subName;
        pos = row.find_first_not_of(" \t",pos);
        if (pos == string::npos)
            throw runtime_error("value not found");

        switch (row[pos]) {
            case ':': 
            {
                if (row[pos+1] != ':')
                    throw runtime_error("single ':' found");
                pos += 2;
                int pos2 = row.find_first_of(" \t=",pos);
                if (pos2 == string::npos)
                    throw runtime_error("value not found");
                subName = row.substr(pos,pos2-pos);
                pos = pos2 + 1;
                break;
            }
            case '[':
            {
                pos++;
                int pos2 = row.find_first_of("]",pos);
                if (pos2 == string::npos)
                    throw runtime_error("matching ']' not found");
                subName = row.substr(pos,pos2-pos);
                pos = pos2 + 1;
                break;
            }
            case '\"':
            {
                pos++;
                int pos2 = row.find_first_of("\"",pos);
                if (pos2 == string::npos)
                    throw runtime_error("matching '\"' not found");
                subName = row.substr(pos,pos2-pos);
                pos = pos2 + 1;
                break;
            }
        }

        bool valueMode = false;
        int  pos2 = row.find_first_of("=",pos);
        if (pos2 != string::npos) {
            valueMode = true;
            pos = pos2 + 1;
            pos2 = row.find_first_not_of(" \t",pos);
            if (pos2 == string::npos)
                throw runtime_error("error parsing value");
            pos = pos2;
        }
        int maxValueSize = 16384;
        if (row[pos] == '\"') {
            pos++;
            int pos2 = row.find_first_of("\"",pos);
            if (pos2 == string::npos)
                throw runtime_error("Error parsing value for "+ name + " in row " + row);
            maxValueSize = pos2 - pos;
        }
        CXmlNode* node = NULL;
        string value = trim(row.substr(pos,maxValueSize));
        bool attemptGrouping = notGroupingTags.indexOf(name) < 0;
        if (!attemptGrouping)
            valueMode = false;
        if (valueMode) {
            if (createAttributes) {
                parentNode->setAttribute(name,value);
                node = parentNode;
            } else {
                if (attemptGrouping)
                    node = parentNode->findFirst(name);
                if (!node)
                    node = new CXmlElement(parentNode,name.c_str());
                if (!subName.empty())
                    node->setAttribute(subName,value);
                else
                    node->setAttribute("value",value);
            }
        } else {
            if (attemptGrouping)
                node = parentNode->findFirst(name);
            if (!node)
                node = new CXmlElement(parentNode,name.c_str());
            if (!subName.empty())
                node->setAttribute("name",subName);
            if (!value.empty())
                node->setAttribute("value",value);
        }
        return node;
    }
    catch (exception& e) {
        throw runtime_error("Error parsing row '" + row + "'\n" + string(e.what()));
    }
}

void CGtkThemeParser::parseImage(const CStrings& gtkrc,unsigned& currentRow,CXmlNode* parentNode) {
    if (gtkrc[currentRow] != "image")
        throw runtime_error("Expecting 'image' in row "+gtkrc[currentRow]);
    currentRow++;
    if (gtkrc[currentRow] != "{")
        throw runtime_error("Expecting '{' in row '"+gtkrc[currentRow]);
    currentRow++;
    CXmlNode* imageNode = new CXmlElement(parentNode,"image");
    while (gtkrc[currentRow] != "}") {
        parseParameter(gtkrc[currentRow],imageNode,true);
        currentRow++;
        if (currentRow == gtkrc.size())
            throw runtime_error("Expecting '}' after row '"+gtkrc[currentRow-1]);
    }
}

void CGtkThemeParser::parseEngine(const CStrings& gtkrc,unsigned& currentRow,CXmlNode* parentNode) {
    if (gtkrc[currentRow].find("engine") != 0)
        throw runtime_error("Expecting 'engine' in row "+gtkrc[currentRow]);
    CXmlNode* engineNode = parseParameter(gtkrc[currentRow++],parentNode);
    try {
        if (gtkrc[currentRow] != "{")
            throw runtime_error("Expecting '{' in row '"+gtkrc[currentRow]+"'");
        currentRow++;
        while (gtkrc[currentRow] != "}") {
            if (gtkrc[currentRow] == "image")
                parseImage(gtkrc,currentRow,engineNode);
            else
                parseParameter(gtkrc[currentRow],engineNode);
            currentRow++;
            if (currentRow == gtkrc.size())
                throw runtime_error("Expecting '}' after row '"+gtkrc[currentRow-1]+"'");
        }
    }
    catch (exception& e) {
        cerr << "Error parsing engine '" << engineNode->getAttribute("name","") << "': " << e.what() << endl;
    }
}

void CGtkThemeParser::parseStyle(const CStrings& gtkrc,unsigned& currentRow,CXmlNode* parentNode) {
    const string& styleRow = gtkrc[currentRow];
    if (gtkrc[currentRow].find("style") != 0)
        throw runtime_error("Expecting 'style' in row "+gtkrc[currentRow]);
    CXmlNode* styleNode = parseParameter(gtkrc[currentRow++],parentNode);
    if (styleNode->getAttribute("name") == "scrollbar")
        styleNode->setAttribute("name","scrollbars");
    if (gtkrc[currentRow] != "{")
        throw runtime_error("Expecting '{' in row '"+gtkrc[currentRow]+"'");
    currentRow++;
    while (gtkrc[currentRow] != "}") {
        const string& str = gtkrc[currentRow];
        if (str.find("engine ") == 0)
            parseEngine(gtkrc,currentRow,styleNode);
        else
            parseParameter(str,styleNode);
        currentRow++;
        if (currentRow == gtkrc.size())
            throw runtime_error("Expecting '}' after row '"+gtkrc[currentRow-1]+"'");
    }
}

void CGtkThemeParser::parse(const CStrings& gtkrc) {
    CBuffer buffer;
    m_xml.clear();
    CXmlNode* stylesNode = new CXmlElement(&m_xml,"styles");
    CXmlNode* paramsNode = new CXmlElement(&m_xml,"styles");
    for (unsigned row = 0; row < gtkrc.size(); row++) {
        const string& str = gtkrc[row];
        if (str.find("style ") == 0)
            parseStyle(gtkrc,row,stylesNode);
        else
            parseParameter(str,&m_xml);
    }
    m_xml.save(buffer);
    buffer.saveToFile("gtkrc.xml");
}

void CGtkThemeParser::load(std::string themeName) throw (std::exception) {
    m_themeFolder = CRegistry::homeDirectory() + ".themes/" + themeName + "/gtk-2.0/";
    string gtkrcFile = m_themeFolder + "gtkrc";
    CStrings gtkrcSource;

    try {
        gtkrcSource.loadFromFile(gtkrcFile);
    }
    catch (exception& e) {
        m_themeFolder = "/usr/share/themes/" + themeName + "/gtk-2.0/";
        gtkrcFile = m_themeFolder + "gtkrc";
        gtkrcSource.loadFromFile(gtkrcFile);
    }

    CStrings gtkrc;
    
    for (unsigned i = 0; i < gtkrcSource.size(); i++) {
        string s = trim(gtkrcSource[i]);

        int pos = 0;

        // Remove comments (if any)
        if (s.find_first_of("#") != string::npos) {
            pos = -1;
            for (;;) {
                pos = s.find_first_of("#\"",pos+1);
                if (s[pos] == '\"') { // Find a matching double quote
                    pos = s.find_first_of("\"",pos+1);
                    if (pos == string::npos)
                        throw runtime_error("Unmatched {\"} found in row " + int2string(i));
                } else {
                    if (pos)
                        s = trim(s.substr(0,pos-1));
                    else
                        s = "";
                    break;
                }
            }
        }

        pos = 0;
        while (pos != string::npos && !s.empty()) {
            if ( s[s.size()-1] == '{') {
                s = trim(s.substr(0,s.size()-1));
                if (!s.empty())
                    gtkrc.push_back(s);
                gtkrc.push_back("{");
                pos = string::npos;
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
