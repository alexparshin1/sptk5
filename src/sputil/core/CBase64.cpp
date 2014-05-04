/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CBase64.cpp  -  description
                             -------------------
    begin                : July 19 2003
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
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

#include <sptk5/CBase64.h>

using namespace std;
using namespace sptk;

// Following structures are originally from mutt
// Best regards to mutt developers!
static int Index_64[128] = {
                               -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                               -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
                               -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, 63,-1,-1,-1,
                               52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
                               -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
                               15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
                               -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
                               41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
                           };

static char B64Chars[64] = {
                               'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
                               'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
                               'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
                               't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
                               '8', '9', '+', '/'
                           };

//#define base64val(c) Index_64[(uint32_t)(c)]
#define base64chars(c) B64Chars[(uint32_t)((c) & 0x3F)]

void CBase64::encode(CBuffer& bufDest, const CBuffer& bufSource)
{
    char   c;
    char*  current = bufSource.data();
    uint32_t len = (uint32_t) bufSource.bytes();

    while (len >= 3) {
        c = base64chars((current[0] & 0xFC) >> 2);
        bufDest.append(c);

        c = base64chars(((current[0] & 0x03) << 4) | ((current[1] & 0xF0) >> 4));
        bufDest.append(c);

        c = base64chars(((current[1] & 0x0F) << 2) | ((current[2] & 0xC0) >> 6));
        bufDest.append(c);

        c = base64chars(current[2] & 0x3F);
        bufDest.append(c);

        len     -= 3;
        current += 3;   /* move pointer 3 characters forward */
    }

    /// Now we should clean up remainder
    if (len > 0) {
        c = base64chars(current[0] >> 2);
        bufDest.append(&c, 1);
        if (len > 1) {
            c = base64chars(((current[0] & 0x03) << 4) | (current[1] >> 4));
            bufDest.append(&c, 1);
            c = base64chars((current[1] & 0x0f) << 2);
            bufDest.append(&c, 1);
            c = '=';
            bufDest.append(&c, 1);
        } else {
            c = base64chars((current[0] & 0x03) << 4);
            bufDest.append(&c, 1);
            c = '=';
            bufDest.append(&c, 1);
            c = '=';
            bufDest.append(&c, 1);
        }
    }
}

void CBase64::encode(string& strDest, const CBuffer& bufSource)
{
    CBuffer bufOut;
    encode(bufOut, bufSource);

    strDest = bufOut.data();
}

static int internal_decode(CBuffer &bufDest, const unsigned char *src, uint32_t src_len)
{
    const unsigned char *current = src;
    unsigned char c;
    int ch, j = 0;

    if (!src_len)
        return 0;

    if ((src_len % 4) != 0)
        throw CException("Can't decode Base64 data - Source buffer length must be dividable by 4");

    bufDest.reset();

    for (unsigned i = 0; i < src_len; i++) {
        ch = current[i];

        if (ch == '=')
            break;
        if (ch == ' ')
            ch = '+';
        ch = Index_64[ch];
        if (ch < 0)
            continue;

        switch (i % 4)
        {
        case 0:
            c = (unsigned char) ((ch << 2) & 0xFF);
            bufDest.append((char *) &c, 1);
            break;
        case 1:
            bufDest.data()[j] |= ((ch >> 4) & 0xFF);
            j++;
            if (current[i + 1] != '=') {
                c = (unsigned char) ((ch << 4) & 0xFF);
                bufDest.append((char *) &c, 1);
            } /* if */
            break;
        case 2:
            bufDest.data()[j] |= ((ch >> 2) & 0x0f);
            j++;
            if (current[i + 1] != '=') {
                c = (unsigned char) ((ch << 6) & 0xFF);
                bufDest.append((char *) &c, 1);
            }
            break;
        case 3:
            bufDest.data()[j] |= ch;
            j++;
            break;
        }
    }
    return j;
}

int CBase64::decode(CBuffer &bufDest, const CBuffer& bufSource) THROWS_EXCEPTIONS
{
    return internal_decode(bufDest, (const unsigned char *)bufSource.data(), (uint32_t)bufSource.bytes());
}

int CBase64::decode(CBuffer &bufDest, const string& strSource) THROWS_EXCEPTIONS
{
    return internal_decode(bufDest,(const unsigned char *)strSource.c_str(),(uint32_t)strSource.length());
}
