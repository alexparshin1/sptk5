/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CStrings.cpp  -  description
                             -------------------
    begin                : Thu August 11 2005
    copyright            : (C) 2005-2012 by Alexey Parshin. All rights reserved.
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

using namespace std;
using namespace sptk;

void CStrings::parseString(const char *src,const char *delimitter) {
    if (!src || !src[0])
        return;
    string buffer(src);
    clear();
    uint32_t dlen = (uint32_t) strlen(delimitter);
    if (!dlen)
        return;
    char *p = (char *)buffer.c_str();
    for (;;) {
        char *end = strstr(p,delimitter);
        if (end) {
            char sc = *end;
            *end = 0;
            push_back(p);
            *end = sc;
            p = end + dlen;
        } else {
            push_back(p);
            break;
        }
    }
}

string CStrings::asString(const char *separator) const {
    string result;
    const_iterator itor = begin();
    if (itor != end()) {
        result = *itor;
        itor++;
    }
    while (itor != end()) {
        result += separator + *itor;
        itor++;
    }
    return result;
}

int CStrings::indexOf(string s) const {
    const_iterator itor = find(begin(),end(),s.c_str());
    if (itor == end())
        return -1;
#ifdef __SUNPRO_CC
    int d;
    distance(begin(),itor,d);
    return d;
#else
    return (int) std::distance(begin(),itor);
#endif
}

void CStrings::saveToFile(string fileName) const throw(CException) {
#ifdef WIN32
    /// I'm using FILE IO here, since the ofstream implementation
    /// in Visual C++ works about 20 times slower
    FILE *f = fopen(fileName.c_str(),"w+t");
    if (!f)
        throw CException("Can't open file "+fileName+" for writing");
    char *buffer = new char[16384];
    setvbuf(f,buffer,_IONBF,16384);

    const_iterator itor = begin();
    for (; itor != end(); itor++) {
        const idstring& str = *itor;
        if (fputs(str.c_str(),f) == EOF) {
            delete [] buffer;
            throw CException("Can't write to file "+fileName);
        }
        if (fputs("\n",f) == EOF) {
            delete [] buffer;
            throw CException("Can't write to file "+fileName);
        }
    }
    fclose(f);
    delete [] buffer;
#else
    ofstream file;
    file.open(fileName.c_str());
    if (!file.is_open())
        throw CException("Can't open file "+fileName+" for reading");
    const_iterator itor = begin();
    for (; itor != end(); itor++) {
        const idstring& str = *itor;
        file << str << endl;
        if (!file.good())
            throw CException("Can't write to file "+fileName);
    }
    file.close();
#endif
}

void CStrings::loadFromFile(string fileName) throw(CException)  {
    string line;
    clear();
    ifstream infile;
    infile.open(fileName.c_str());
    if (!infile.is_open())
        throw CException("Can't open file "+fileName+" for reading");
    while (getline(infile,line,'\n'))
        push_back (line);
    infile.close();
}
