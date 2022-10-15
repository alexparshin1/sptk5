/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>
#include <sptk5/FileLogEngine.h>

using namespace std;
using namespace sptk;

TEST(SPTK_FileLogEngine, create)
{
    const fs::path logFileName("/tmp/file_log_test.tmp");

    unlink(logFileName.string().c_str());

    auto logEngine = make_shared<FileLogEngine>(logFileName);
    auto logger = make_shared<Logger>(*logEngine, "(Test application) ");
    logger->debug("Test started");
    logger->critical("Critical message");
    logger->error("Error message");
    logger->warning("Warning message");
    logger->info("Test completed");

    this_thread::sleep_for(chrono::milliseconds(10));

    logger.reset();
    logEngine.reset();

    Strings content;
    content.loadFromFile(logFileName);

    EXPECT_EQ(content.size(), 5U);
    for (String level: {"critical", "error", "warning", "info", "debug"})
    {
        const auto messages = content.grep(level.toUpperCase());
        EXPECT_EQ(messages.size(), 1U);
    }
}
