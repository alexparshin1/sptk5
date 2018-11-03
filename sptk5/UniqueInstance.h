/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       UniqueInstance.h - description                         ║
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

#ifndef __SPTK_UNIQUEINSTANCE_H__
#define __SPTK_UNIQUEINSTANCE_H__

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
#endif

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Unique instance object
 *
 * Tries to create a mutex object and check if it's unique for the
 * program with the instance name. If one instance per user is desired
 * simply incorporate the user name into the instance.
 */
class SP_EXPORT UniqueInstance
{
    /**
     * Instance name
     */
    String      m_instanceName;

    /**
     * Lock is created
     */
    bool         m_lockCreated;


#ifdef _WIN32

    /**
     * The named mutex object
     */
    HANDLE       m_mutex;

#else

    /**
     * The lock file name
     */
    String      m_fileName;


    /**
     * Gets the process ID
     */
    int      read_pid();

    /**
     * Writes the process ID into the lock file
     */
    int      write_pid();

#endif

public:
    /**
     * Constructor
     * @param instanceName CString, instance name
     */
    UniqueInstance(const std::string& instanceName);

    /**
     * Destructor
     */
    ~UniqueInstance();

#ifndef _WIN32
    /**
     * Return lock file name
     * @return lock file name
     */
	const String& lockFileName() const;
#endif // _WIN32

    /**
     * Reports true if the instance is unique
     */
    bool isUnique();
}
;
/**
 * @}
 */
}

#endif
