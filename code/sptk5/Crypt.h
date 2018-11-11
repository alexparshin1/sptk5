/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                           Crypt.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Tusday July 18 2017                                    ║
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

#ifndef __SPTK_CRYPT_H__
#define __SPTK_CRYPT_H__

#include <sptk5/Buffer.h>

namespace sptk
{

/**
 * Simple wrapper for OpenSSL encryption functions
 */
class Crypt
{
public:
    /**
     * Encrypt data using OpenSSL
     * @param dest Buffer&, Destination buffer
     * @param src const Buffer&, Source buffer
     * @param key const std::string&, Encryption key
     * @param iv const std::string&, Initialization vector
     */
    static void encrypt(Buffer& dest, const Buffer& src, const std::string& key, const std::string& iv);

    /**
     * Decrypt data using OpenSSL
     * @param dest Buffer&, Destination buffer
     * @param src const Buffer&, Source buffer
     * @param key const std::string&, Encryption key
     * @param iv const std::string&, Initialization vector
     */
    static void decrypt(Buffer& dest, const Buffer& src, const std::string& key, const std::string& iv);
};

}

#endif
