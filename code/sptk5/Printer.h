/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Printer.h - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday November 14 2018                              ║
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
#ifndef __SCREEN_STREAM_H__
#define __SCREEN_STREAM_H__

#include <sptk5/String.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace sptk {

/**
 * Thread-safe stream printer.
 * Designed to be used with stdout and stderr.
 */
class Printer
{
    static std::mutex   m_mutex;    ///< Mutex that protects file access
    FILE*               m_stream;   ///< Output file
public:
    /**
     * Constructor
     * @param stream            Output file
     */
    explicit Printer(FILE* stream);

    /**
     * Thread-safe print
     * @param text              Text to print
     */
    void print(const String& text);
};

extern Printer __stdout;
extern Printer __stderr;

#define COUT(a) { std::stringstream _printstream; _printstream << a; sptk::__stdout.print(_printstream.str().c_str()); }
#define CERR(a) { std::stringstream _printstream; _printstream << a; sptk::__stderr.print(_printstream.str().c_str()); }

}

#endif
