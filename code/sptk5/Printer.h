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

#include <sptk5/DateTime.h>
#include <sptk5/sptk.h>

#include <iostream>
#include <mutex>

namespace sptk {

class SP_EXPORT Console
{
public:
    static std::mutex& printMutex();
};
} // namespace sptk

#define COUT(a)                                                        \
    do                                                                 \
    {                                                                  \
        const std::scoped_lock printLock(sptk::Console::printMutex()); \
        std::cout << a << "\n"                                         \
                  << std::flush;                                       \
    } while (false)

#define COUT_TS(a)                                                                            \
    do                                                                                        \
    {                                                                                         \
        const std::scoped_lock printLock(sptk::Console::printMutex());                        \
        std::cout << DateTime::Now().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS) \
                  << " " << a << "\n"                                                         \
                  << std::flush;                                                              \
    } while (false)

#define CERR(a)                                                        \
    do                                                                 \
    {                                                                  \
        const std::scoped_lock printLock(sptk::Console::printMutex()); \
        std::cerr << a << "\n";                                        \
    } while (false)

#define CERR_TS(a)                                                                            \
    do                                                                                        \
    {                                                                                         \
        const std::scoped_lock printLock(sptk::Console::printMutex());                        \
        std::cerr << DateTime::Now().isoDateTimeString(DateTime::PrintAccuracy::MILLISECONDS) \
                  << " " << a << "\n";                                                        \
    } while (false)
