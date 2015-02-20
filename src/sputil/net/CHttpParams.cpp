/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CHttpParams.cpp  -  description
                             -------------------
    begin                : July 22 2003
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

#include <sptk5/net/CHttpParams.h>
#include <sptk5/CBuffer.h>
#include <sptk5/CStrings.h>
#include <sptk5/string_ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <vector>

using namespace std;
using namespace sptk;

string CHttpParams::encodeString(string& str) {
    uint32_t cnt = (uint32_t) str.length();
    const char *src = str.c_str();
    char *buffer = (char *)calloc(cnt*3+1,1);
    char *dest = buffer;
    while (*src) {
        if (isalnum(*src)) {
            *dest = *src;
            dest++;
        } else {
            switch (*src) {
            case ' ':
                *dest = '+';
                dest++;
                break;
            case '.':
            case '-':
                *dest = *src;
                dest++;
                break;
            default:
                sprintf(dest,"%%%02X",(unsigned)*src);
                dest += 3;
                break;
            }
        }
        src++;
    }
    string result(buffer);
    free(buffer);
    return result;
}

int hexCharToInt(unsigned char ch) {
    if (ch > '@')
        return ch - 'A' + 10;
    return ch - '0';
}

string CHttpParams::decodeString(string& str) {
    uint32_t cnt = (uint32_t) str.length();
    const char *src = str.c_str();
    char *buffer = (char *)calloc(cnt+1,1);
    char *dest = buffer;
    while (*src) {
        switch (*src) {
        case '+':
            *dest = ' ';
            src++;
            break;
        default:
            *dest = *src;
            src++;
            break;
        case '%':
            src++;
            *dest = char(hexCharToInt((unsigned char)*src) * 16 + hexCharToInt((unsigned char)src[1]));
            src += 2;
            break;
        }
        dest++;
    }
    string result(buffer);
    free(buffer);
    return result;
}

void CHttpParams::decode(const CBuffer& cb, bool /*lowerCaseNames*/) {
    clear();

    CStrings sl(cb.data(),"&");
    for (unsigned i=0; i < sl.size(); i++) {
        string& s = sl[i];
        size_t pos = s.find("=");
        if (pos != STRING_NPOS) {
            string key = s.substr(0, pos);
            string value = s.substr(pos+1);
            (*this)[key] = decodeString(value);
        }
    }
}

void CHttpParams::encode(CBuffer& result) {
    CHttpParams::iterator itor = begin();
    unsigned cnt = 0;
    for (; itor != end(); itor++) {
        string param;
        param = itor->first + "=" + encodeString( itor->second );
        if (cnt)
            result.append("&",1);
        result.append(param);
        cnt++;
    }
}
