/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CStrings.cpp  -  description
                             -------------------
    begin                : Thu August 11 2005
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#include <fstream>
#include <string.h>
#include <sptk5/CStrings.h>
#include <sptk5/string_ext.h>
#include <sptk5/CBuffer.h>

using namespace std;
using namespace sptk;

void CStrings::splitByDelimiter(const string& src, const char *delimitter)
{
    size_t  pos = 0, end = 0;
    size_t  dlen = strlen(delimitter);
    while (true) {
        end = src.find(delimitter, pos);
        if (end != string::npos) {
            push_back(src.substr(pos, end - pos));
            pos = end + dlen;
        } else {
            push_back(src.substr(pos));
            break;
        }
    }
}

void CStrings::splitByAnyChar(const string& src, const char *delimitter)
{
    size_t  pos = 0, end = 0;
    while (true) {
        end = src.find_first_of(delimitter, pos);
        if (end != string::npos) {
            push_back(src.substr(pos, end - pos));
            pos = src.find_first_not_of(delimitter, end + 1);
        } else {
            push_back(src.substr(pos));
            break;
        }
    }
}

void CStrings::fromString(const string& src, const char *delimitter, SplitMode mode) {
    clear();
    if (mode == SM_DELIMITER)
        splitByDelimiter(src, delimitter);
    else
        splitByAnyChar(src, delimitter);
}

string CStrings::asString(const char *delimiter) const 
{
    string result;
    for (const_iterator str = begin(); str != end(); str++) {
        if (result.empty())
            result = *str;
        else
            result += delimiter + *str;
    }
    return result;
}

int CStrings::indexOf(string s) const {
    const_iterator itor = find(begin(),end(),s.c_str());
    if (itor == end())
        return -1;
    return (int) distance(begin(),itor);
}

void CStrings::saveToFile(string fileName) const THROWS_EXCEPTIONS
{
    CBuffer buffer;
    for (const_iterator str = begin(); str != end(); str++) {
        buffer.append(*str);
        buffer.append("\n");
    }
    buffer.saveToFile(fileName);
}

void CStrings::loadFromFile(string fileName) THROWS_EXCEPTIONS
{
    CBuffer buffer;
    buffer.loadFromFile(fileName);
    
    // Load text
    string text(buffer.c_str(), buffer.bytes());
    
    // Determine delimiter
    size_t pos1 = text.find_first_of("\n\r");
    size_t pos2 = text.find_first_of("\n\r", pos1 + 1);
    string delimiter = text.substr(pos1, 1);
    if (pos1 + 1 == pos2) {
        if (text[pos1] != text[pos2]) // Two chars delimiter
            delimiter = text.substr(pos1, 2);
    }
    
    splitByDelimiter(text, delimiter.c_str());
}
