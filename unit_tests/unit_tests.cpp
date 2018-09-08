/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CBase64.cpp - description                              ║
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

#include <gtest/gtest.h>
#include <iostream>
#include <sptk5/db/DatabaseTests.h>

#include <sptk5/cutils>
#include <sptk5/cthreads>
#include <sptk5/cnet>
#include <sptk5/JWT.h>
#include <sptk5/Crypt.h>
#include <sptk5/CommandLine.h>
#include <sptk5/DirectoryDS.h>
#include <sptk5/threads/Timer.h>
#include <sptk5/Tar.h>
#include <sptk5/Base64.h>
#include <sptk5/db/DatabaseConnectionPool.h>

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

int main(int argc, char* argv[])
{
#ifdef _WIN32
	// Make sure Winsock is initialized
	TCPSocket socket;
#endif

	// All database connections below assume the database is loacted on host 'dbhost',
	// and user 'test' has password 'test#123'.

	//databaseTests.addConnection(DatabaseConnectionString("postgresql://test:test#123@dbhost_pg:5432/gtest"));
    databaseTests.addConnection(DatabaseConnectionString("mysql://gtest:test#123@127.0.0.1:3306/gtest"));
	//databaseTests.addConnection(DatabaseConnectionString("oracle://gtest:test#123@oracledb:1521/protis"));
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
