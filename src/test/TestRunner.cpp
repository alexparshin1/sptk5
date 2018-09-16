/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       TestRunner.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Monday September 10, 2018                              ║
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

#include <sptk5/db/DatabaseTests.h>
#include <sptk5/Strings.h>
#include <googletest/include/gtest/gtest.h>
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/ServerConnection.h>
#include <sptk5/JWT.h>
#include <sptk5/CommandLine.h>
#include <sptk5/threads/ThreadPool.h>
#include <sptk5/DirectoryDS.h>
#include <sptk5/threads/Timer.h>
#include <sptk5/threads/RWLock.h>
#include <sptk5/Tar.h>
#include <sptk5/net/HttpConnect.h>
#include <sptk5/Crypt.h>
#include <sptk5/Base64.h>
#include <sptk5/db/DatabaseConnectionPool.h>
#include <sptk5/test/TestRunner.h>

using namespace std;
using namespace sptk;

void acallback(void*) {}

class StubServer : public TCPServer
{
protected:
    ServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer) override
    {
        return nullptr;
    }
public:
};

// Hints to linker that we need other modules.
// Otherwise, Visual Studio doesn't include any tests
void stub()
{
    DateTime			dt;
    JWT					jwt;
    RegularExpression	regexp(".*");
    CommandLine			cmd("", "", "");
    DirectoryDS			dir("");
    ThreadPool			threads;
    Timer				timer(acallback);
    MD5					md5;
    StubServer			tcpServer;
    Tar					tar;
    FieldList			fieldList(false);
    SharedStrings		sharedStrings;
    Variant				v;
    RWLock				lock;

    TCPSocket			socket;
    HttpConnect			connect(socket);

    string text("The quick brown fox jumps over the lazy dog.ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    string key("01234567890123456789012345678901");
    string iv("0123456789012345");

    Buffer intext(text), outtext;
    cout << "Encrypt text (" << text.length() << " bytes)." << endl;
    Crypt::encrypt(outtext, intext, key, iv);

    Buffer b1, b2("xxx");
    Base64::encode(b1, b2);

    DatabaseConnectionPool 		connectionPool("");
}

TestRunner::TestRunner(int& argc, char**& argv)
: m_argc(argc), m_argv(argv)
{
}

void TestRunner::addDatabaseConnection(const DatabaseConnectionString& connectionString)
{
    databaseTests.addDatabaseConnection(connectionString);
}

static String excludeDatabasePatterns(const std::vector<DatabaseConnectionString>& definedConnections)
{
    map<String,String> excludeDrivers = {
            { "postgresql", "PostgreSQL" },
            { "mysql", "MySQL" },
            { "mssql", "MSSQL" },
            { "oracle", "Oracle" }
    };

    for (auto& connection: definedConnections)
        excludeDrivers.erase(connection.driverName());

    Strings excludePatterns;
    for (auto itor: excludeDrivers)
        excludePatterns.push_back("SPTK_" + itor.second + "*.*");

    return excludePatterns.join(":");
}

int TestRunner::runAllTests()
{
#ifdef _WIN32
    // Make sure Winsock is initialized
	TCPSocket socket;
#endif

	String excludeDBDriverPatterns = excludeDatabasePatterns(databaseTests.connectionStrings());

	size_t filterArgumentIndex = 0;
	for (int i = 1; i < m_argc; i++) {
		if (strstr(m_argv[i], "--gtest_filter=")) {
			filterArgumentIndex = i;
			break;
		}
	}

	vector<char*> argv(m_argv, m_argv + m_argc);
	String filter;

	if (!excludeDBDriverPatterns.empty()) {
		if (filterArgumentIndex != 0)
			filter = argv[filterArgumentIndex];

		if (filter.empty())
			filter = "-" + excludeDBDriverPatterns;
		else
			filter += ":-" + excludeDBDriverPatterns;

		if (filterArgumentIndex == 0) {
			argv.push_back((char*)filter.c_str());
			m_argc++;
		}
		else
			argv[filterArgumentIndex] = (char*) filter.c_str();
	}

	::testing::InitGoogleTest(&m_argc, &argv[0]);

	return RUN_ALL_TESTS();
}
