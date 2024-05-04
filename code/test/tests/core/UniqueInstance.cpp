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

#include <cstdio>
#include <fstream>

#include <sptk5/Exception.h>
#include <sptk5/UniqueInstance.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

#ifndef _WIN32

TEST(SPTK_UniqueInstance, create)
{
    UniqueInstance uniqueInstance("unit_tests");
    EXPECT_TRUE(uniqueInstance.isUnique());

    // Simulate lock file with non-existing process
    ofstream lockFile(uniqueInstance.lockFileName());
    constexpr int testPID = 123456;
    lockFile << testPID;
    lockFile.close();

    UniqueInstance uniqueInstance2("unit_tests");
    EXPECT_TRUE(uniqueInstance2.isUnique());

    // Get pid of existing process
    if (FILE* pipe1 = popen("pidof systemd", "r"); pipe1 != nullptr)
    {
        constexpr int bufferSize = 64;
        array<char, bufferSize> buffer {};
        if (const char* data = fgets(buffer.data(), sizeof(buffer), pipe1); data != nullptr)
        {
            int pid = string2int(data);
            if (pid > 0)
            {
                lockFile.open(uniqueInstance.lockFileName());
                lockFile << pid;
                lockFile.close();
                UniqueInstance uniqueInstance3("unit_tests");
                EXPECT_FALSE(uniqueInstance3.isUnique());
            }
        }
        pclose(pipe1);
    }
}

#endif
