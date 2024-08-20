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

#include <sptk5/CommandLine.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

class CommandLineTestData
{
public:
    static vector<const char*> testCommandLineArgs;
    static vector<const char*> testCommandLineArgs2;
    static vector<const char*> testCommandLineArgs3;
};

vector<const char*> CommandLineTestData::testCommandLineArgs = {"testapp", "connect", "--host",
                                                                "'ahostname'", "-p", "12345", "--verbose",
                                                                "--description",
                                                                "'This", "is", "a", "quoted", "argument'"};

vector<const char*> CommandLineTestData::testCommandLineArgs2 = {"testapp", "connect", "--host",
                                                                 "host name", "-p", "12345",
                                                                 "--verbose"};

vector<const char*> CommandLineTestData::testCommandLineArgs3 = {"testapp", "connect", "--host",
                                                                 "ahostname", "-p", "12345",
                                                                 "--verbotten", "--gtest_test"};

shared_ptr<CommandLine> createTestCommandLine()
{
    auto commandLine = make_shared<CommandLine>("test 1.00", "This is test command line description.",
                                                "testapp <action> [options]");
    commandLine->defineArgument("action", "Action to perform");
    commandLine->defineParameter("host", "h", "hostname", "^[\\S]+$", CommandLine::Visibility(""), "",
                                 "Hostname to connect");
    commandLine->defineParameter("port", "p", "port #", "^\\d{2,5}$", CommandLine::Visibility(""), "80",
                                 "Port to connect");
    commandLine->defineParameter("port2", "r", "port #", "^\\d{2,5}$", CommandLine::Visibility(""), "80",
                                 "Port to connect");
    commandLine->defineParameter("description", "d", "text", "", CommandLine::Visibility(""), "",
                                 "Operation description");
    commandLine->defineOption("verbose", "v", CommandLine::Visibility(""), "Verbose messages");
    return commandLine;
}

TEST(SPTK_CommandLine, Visibility)
{
    const CommandLine::Visibility visibility1("");
    const CommandLine::Visibility visibility2("^\\d+$");

    EXPECT_TRUE(visibility1.any());
    EXPECT_TRUE(visibility2.matches("123"));
}

TEST(SPTK_CommandLine, CommandLineElement)
{
    const CommandLine::CommandLineElement testElement("test", "t", "short help", CommandLine::Visibility("^test|not-test$"));

    EXPECT_FALSE(testElement.hasValue());
    EXPECT_TRUE(testElement.useWithCommand("not-test"));
}

TEST(SPTK_CommandLine, ctor)
{
    const auto commandLine = createTestCommandLine();
    commandLine->init(CommandLineTestData::testCommandLineArgs.size(),
                      CommandLineTestData::testCommandLineArgs.data());

    EXPECT_EQ(static_cast<size_t>(1), commandLine->arguments().size());
    EXPECT_STREQ("ahostname", commandLine->getOptionValue("host").c_str());
    EXPECT_STREQ("12345", commandLine->getOptionValue("port").c_str());
    EXPECT_STREQ("80", commandLine->getOptionValue("port2").c_str());
    EXPECT_STREQ("true", commandLine->getOptionValue("verbose").c_str());
    EXPECT_STREQ("", commandLine->getOptionValue("bad").c_str());
    EXPECT_STREQ("This is a quoted argument", commandLine->getOptionValue("description").c_str());
    EXPECT_STREQ("connect", commandLine->arguments()[0].c_str());
    EXPECT_TRUE(commandLine->hasOption("verbose"));

    EXPECT_THROW(commandLine->setOptionValue("something", "xxx"), Exception);
}

TEST(SPTK_CommandLine, wrongArgumentValue)
{
    const auto commandLine = createTestCommandLine();

    EXPECT_THROW(
        commandLine->init(CommandLineTestData::testCommandLineArgs2.size(),
                          CommandLineTestData::testCommandLineArgs2.data()),
        Exception);
}

TEST(SPTK_CommandLine, wrongOption)
{
    const auto commandLine = createTestCommandLine();

    EXPECT_THROW(
        commandLine->init(CommandLineTestData::testCommandLineArgs3.size(),
                          CommandLineTestData::testCommandLineArgs3.data()),
        Exception);
}

TEST(SPTK_CommandLine, setOption)
{
    const auto commandLine = createTestCommandLine();

    commandLine->init(CommandLineTestData::testCommandLineArgs.size(),
                      CommandLineTestData::testCommandLineArgs.data());
    EXPECT_STREQ(commandLine->getOptionValue("host").c_str(), "ahostname");
    commandLine->setOptionValue("host", "www.x.com");
    EXPECT_STREQ(commandLine->getOptionValue("host").c_str(), "www.x.com");
}

TEST(SPTK_CommandLine, printHelp)
{
    const auto commandLine = createTestCommandLine();

    commandLine->init(CommandLineTestData::testCommandLineArgs.size(),
                      CommandLineTestData::testCommandLineArgs.data());

    constexpr size_t terminalWidth {80};
    commandLine->printHelp(terminalWidth);
}
