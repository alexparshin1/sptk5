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

#include <sptk5/Exception.h>
#include <sptk5/UniqueInstance.h>

#ifndef _WIN32
#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED
#endif
#endif

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
    char   buffer[32];

    // Try to read process id from the file
    FILE  *f = fopen(m_fileName.c_str(),"r+b");
    if (f == nullptr)
        return 0;
    fgets(buffer,32,f);
    fclose(f);

    // Is it a number?
    buffer[31] = 0;
    char *p = strchr(buffer,'\n');
    if (p != nullptr)
        *p = 0;
    int pid = atoi(buffer);
    if (pid == 0)
        return 0;

    // Does the process with this id exist?
    int sid = getsid(pid);

    if (sid < 0) // No such process - stale lock file.
        return 0;

    return pid;
}

int UniqueInstance::write_pid()
{
    FILE  *f = fopen(m_fileName.c_str(),"w+b");
    if (f == nullptr)
        return 0;
    int pid = getpid();
    fprintf(f,"%i\n",pid);
    fclose(f);

    m_lockCreated = true;
    return pid;
}
#endif

bool UniqueInstance::isUnique()
{
    return m_lockCreated;
}
