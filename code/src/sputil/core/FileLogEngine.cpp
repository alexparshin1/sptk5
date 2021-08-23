/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/FileLogEngine.h>

using namespace std;
using namespace sptk;

void FileLogEngine::saveMessage(const Logger::UMessage& message)
{
    lock_guard lock(m_mutex);

    if (auto _options = (uint32_t) options(); (_options & LO_ENABLE) == LO_ENABLE)
    {
        if (!m_fileStream.is_open())
        {
            m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::app);
            if (!m_fileStream.is_open())
            {
                throw Exception("Can't append or create log file '" + m_fileName.string() + "'", __FILE__, __LINE__);
            }
        }

        if ((_options & LO_DATE) == LO_DATE)
        {
            m_fileStream << message->timestamp.dateString() << " ";
        }

        if ((_options & LO_TIME) == LO_TIME)
        {
            m_fileStream << message->timestamp.timeString(true) << " ";
        }

        if ((_options & LO_PRIORITY) == LO_PRIORITY)
        {
            m_fileStream << "[" << priorityName(message->priority) << "] ";
        }

        m_fileStream << message->message << endl;
    }

    if (m_fileStream.bad())
    {
        throw Exception("Can't write to log file '" + m_fileName.string() + "'", __FILE__, __LINE__);
    }
}

FileLogEngine::FileLogEngine(const fs::path& fileName)
    : LogEngine("FileLogEngine"), m_fileName(fileName), m_fileStream(fileName.c_str())
{
}

FileLogEngine::~FileLogEngine()
{
    sleep_for(chrono::milliseconds(1000));
    terminate();
    join();
    lock_guard lock(m_mutex);
    if (m_fileStream.is_open())
    {
        m_fileStream.close();
    }
}

void FileLogEngine::reset()
{
    lock_guard lock(m_mutex);
    if (m_fileStream.is_open())
    {
        m_fileStream.close();
    }
    if (m_fileName.empty())
    {
        throw Exception("File name isn't defined", __FILE__, __LINE__);
    }
    m_fileStream.open(m_fileName.c_str(), ofstream::out | ofstream::trunc);
    if (!m_fileStream.is_open())
    {
        throw Exception("Can't open log file '" + m_fileName.string() + "'", __FILE__, __LINE__);
    }
}

#ifdef USE_GTEST

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

#endif
