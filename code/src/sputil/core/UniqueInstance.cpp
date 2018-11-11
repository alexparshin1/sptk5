/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       UniqueInstance.cpp - description                       ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <sptk5/Exception.h>
#include <sptk5/UniqueInstance.h>

#ifndef _WIN32
#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED
#endif
#endif

using namespace std;
using namespace sptk;

// Constructor
UniqueInstance::UniqueInstance(const std::string& instanceName)
{
    m_lockCreated = false;
    m_instanceName = instanceName;
#ifndef _WIN32
    std::string home = getenv("HOME");
    m_fileName = home + "/" + m_instanceName + ".lock";
    if (read_pid() == 0)
        write_pid();
#else
    m_mutex = CreateMutex(NULL,true,m_instanceName.c_str());
    if (GetLastError() == 0)
        m_lockCreated = true;
#endif
}

// Destructor
UniqueInstance::~UniqueInstance()
{
    // Cleanup
    if (m_lockCreated) {
#ifndef _WIN32
        unlink(m_fileName.c_str());
#else

        CloseHandle(m_mutex);
#endif

    }
}

#ifndef _WIN32
// Get the existing process id (if any) from the file
int UniqueInstance::read_pid()
{
    // Try to read process id from the file
    int pid = 0;
    ifstream lockfile(m_fileName.c_str());
    lockfile >> pid;
    if (lockfile.bad())
        pid = 0; // Lock file doesn't exist, or doesn't contain pid
    lockfile.close();
    if (pid == 0)
        return 0; // Lock file exists, but there is no process id int

    int rc = getsid(pid);
    if (rc < 0 || rc == ESRCH)
        return 0; // No such process - stale lock file.

    return pid;
}

int UniqueInstance::write_pid()
{
    ofstream lockfile(m_fileName.c_str());
    int pid = getpid();
    lockfile << pid;
    lockfile.close();

    m_lockCreated = true;
    return pid;
}

const String& UniqueInstance::lockFileName() const
{
	return m_fileName;
}
#endif

bool UniqueInstance::isUnique()
{
    return m_lockCreated;
}

#ifdef USE_GTEST
#include <gtest/gtest.h>

#ifndef _WIN32
TEST(SPTK_UniqueInstance, create)
{
    UniqueInstance uniqueInstance("unit_tests");
    EXPECT_TRUE(uniqueInstance.isUnique());

    // Simulate lock file with non-existing process
    ofstream lockFile(uniqueInstance.lockFileName());
    lockFile << 123456;
    lockFile.close();

    UniqueInstance uniqueInstance2("unit_tests");
    EXPECT_TRUE(uniqueInstance2.isUnique());

    // Get pid of existing process
    FILE* pipe1 = popen("pidof systemd", "r");
    if (pipe1 != nullptr) {
        char buffer[64];
        const char* data = fgets(buffer, sizeof(buffer), pipe1);
        if (data) {
            int pid = string2int(data);
            if (pid > 0) {
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
#endif
