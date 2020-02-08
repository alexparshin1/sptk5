/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ReadBuffer.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday Jan 9 2018                                   ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

using namespace std;
using namespace sptk;

bool ReadBuffer::read(void* data, size_t length)
{
    if (bytes() - m_readOffset < length)
        return false;
    if (data != nullptr)
        memcpy(data, c_str() + m_readOffset, length);
    m_readOffset += length;
    compact();
    return true;
}

bool ReadBuffer::read(String& data, size_t length)
{
    data.resize(length);
    return read(&data[0], length);
}

bool ReadBuffer::read(Buffer& data, size_t length)
{
    data.checkSize(length);
    return read(data.data(), length);
}

#if USE_GTEST

TEST(SPTK_ReadBuffer, read)
{
    ReadBuffer  buffer;

    for (int i = 0; i < 3; i++)
        buffer.append(i);

    String test1(":test1:");
    buffer.append(test1);

    for (int i = 3; i < 5; i++)
        buffer.append(i);

    EXPECT_EQ(size_t(27), buffer.available());
    EXPECT_EQ(size_t(0), buffer.readOffset());

    for (int i = 0; i < 5; i++) {
        int x;
        buffer.read(x);
        EXPECT_EQ(i, x);
        if (i == 2) {
            String test;
            buffer.read(test, test1.length());
        }
    }

    EXPECT_EQ(size_t(0), buffer.available());
    EXPECT_EQ(size_t(0), buffer.readOffset());
}

#endif
