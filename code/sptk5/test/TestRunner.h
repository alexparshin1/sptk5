/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TestRunner.cpp - description                           ║
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

#ifndef __TEST_RUNNER_H__
#define __TEST_RUNNER_H__

#include <sptk5/db/DatabaseConnectionString.h>

namespace sptk {

/**
 * GTest run test wrapper
 */
class TestRunner
{
    int&        m_argc;     ///< Reference to command line arguments number
    char**&     m_argv;     ///< Reference to command line arguments array
public:
    /**
     * Constructor
     * @param argc              Command line arguments number
     * @param argv              Command line arguments
     */
    TestRunner(int& argc, char**& argv);

    /**
     * Define database connection for DB module that should be tested
     * @param connectionString
     */
    void addDatabaseConnection(const DatabaseConnectionString& connectionString);

    /**
     * Actually run tests
     * @return 0 if tests were successful
     */
    int runAllTests();
};

} // namespace sptk

#endif
