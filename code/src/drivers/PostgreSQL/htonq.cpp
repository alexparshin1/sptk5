/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       htonq.cpp - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef WIN32
#include <arpa/inet.h>
#endif

namespace sptk {

    uint64_t htonq(uint64_t val) {
        uint64_t result;
        auto* src = (uint32_t *)(void *)&val;
        auto* dst = (uint32_t *)(void *)&result;
        dst[0] = htonl(src[1]);
        dst[1] = htonl(src[0]);
        return result;
    }

    uint64_t ntohq(uint64_t val) {
        uint64_t result;
        auto* src = (uint32_t *)(void *)&val;
        auto* dst = (uint32_t *)(void *)&result;
        dst[0] = htonl(src[1]);
        dst[1] = htonl(src[0]);
        return result;
    }

    void htonq_inplace(uint64_t* in, uint64_t* out)
    {
        auto* src = (uint32_t *)(void *)in;
        auto* dst = (uint32_t *)(void *)out;
        dst[1] = htonl(src[0]);
        dst[0] = htonl(src[1]);
    }

} // namespace sptk
