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

#ifndef WIN32
#include <arpa/inet.h>
#include <cstdint>
#else
#include <cstdint>
#include <windows.h>
#endif

#include <bit>

namespace sptk {

uint64_t htonq(uint64_t val)
{
    uint64_t    result {0};
    const auto* src = std::bit_cast<uint32_t*>(&val);
    auto*       dst = std::bit_cast<uint32_t*>(&result);
    dst[0] = htonl(src[1]);
    dst[1] = htonl(src[0]);
    return result;
}

uint64_t ntohq(uint64_t val)
{
    uint64_t    result {0};
    const auto* src = std::bit_cast<uint32_t*>(&val);
    auto*       dst = std::bit_cast<uint32_t*>(&result);
    dst[0] = htonl(src[1]);
    dst[1] = htonl(src[0]);
    return result;
}

void htonq_inplace(const uint64_t* input, uint64_t* output)
{
    const auto* src = std::bit_cast<uint32_t*>(input);
    auto*       dst = std::bit_cast<uint32_t*>(output);
    dst[1] = htonl(src[0]);
    dst[0] = htonl(src[1]);
}

} // namespace sptk
