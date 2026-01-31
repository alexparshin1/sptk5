/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Buffer.h>
#include <sptk5/SystemException.h>

#include <cerrno>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

TEST(SPTK_SystemException, openFile)
{
    try
    {
        Buffer buffer;
        buffer.loadFromFile("/xx.xx");
        FAIL() << "MUST FAIL";
    }
    catch (const Exception& e)
    {
        if (String(e.what()).find("xx.xx") == string::npos)
            FAIL() << e.what();
    }
}

TEST(SPTK_SystemException, osError_returns_message)
{
#ifdef _WIN32
    SetLastError(ERROR_FILE_NOT_FOUND);
#else
    errno = ENOENT;
#endif
    const String error = SystemException::osError();
    EXPECT_FALSE(error.empty());
}
