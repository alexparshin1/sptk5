/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/ReadBuffer.h"

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

TEST(SPTK_ReadBuffer, read)
{
    ReadBuffer buffer;

    for (int i = 0; i < 3; ++i)
    {
        buffer.append(i);
    }

    String test1(":test1:");
    buffer.append(test1);

    constexpr int dataLength {5};
    for (int i = 3; i < dataLength; ++i)
    {
        buffer.append(i);
    }

    EXPECT_EQ(size_t(27), buffer.available());
    EXPECT_EQ(size_t(0), buffer.readOffset());

    for (int i = 0; i < dataLength; ++i)
    {
        int x = 0;
        buffer.read(x);
        EXPECT_EQ(i, x);
        if (i == 2)
        {
            String test;
            buffer.read(test, test1.length());
        }
    }

    EXPECT_EQ(size_t(0), buffer.available());
    EXPECT_EQ(size_t(0), buffer.readOffset());
}
