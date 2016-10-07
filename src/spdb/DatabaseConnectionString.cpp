/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseConnectionString.cpp - description            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/db/DatabaseConnectionString.h>
#include <sptk5/Strings.h>

using namespace std;
using namespace sptk;

static const Strings driverNames("sqlite3|postgres|postgresql|oracle|mysql|firebird|odbc|mssql", "|");

void DatabaseConnectionString::parse() THROWS_EXCEPTIONS
{
    size_t pos;
    string connStr(m_connectionString);

    // Find extra parameters
    pos = connStr.find_first_of("?");
    if (pos != string::npos) {
        Strings parameters(connStr.substr(pos + 1),"&");
        for (Strings::iterator item = parameters.begin(); item != parameters.end(); ++item) {
            Strings pair(*item, "='", Strings::SM_ANYCHAR);
            if (pair.size() == 2)
                m_parameters[ pair[0] ] = pair[1];
        }
        connStr.erase(pos);
    }

    pos = connStr.find("://");
    if (pos != string::npos) {
        m_driverName = connStr.substr(0, pos);
        connStr.erase(0, pos + 3);
        if (driverNames.indexOf(m_driverName) < 0)
            throwDatabaseException("Driver name is unknown: " + m_connectionString);
    } else
        throwDatabaseException("Driver name is missing: " + m_connectionString);

    pos = connStr.find("@");
    if (pos != string::npos) {
        Strings usernameAndPassword(connStr.substr(0, pos),":");
        m_userName = usernameAndPassword[0];
        if (usernameAndPassword.size() > 1)
            m_password = usernameAndPassword[1];
        connStr.erase(0, pos + 1);
    }

    pos = connStr.find("/");
    if (pos != string::npos) {
        m_databaseName = connStr.substr(pos + 1);
        connStr.erase(pos);
        if (m_databaseName.find("/") != string::npos)
            m_databaseName = "/" + m_databaseName;
    }

    Strings hostAndPort(connStr, ":");
    m_hostName = hostAndPort[0];
    if (hostAndPort.size() > 1)
        m_portNumber = (uint16_t) atoi(hostAndPort[1].c_str());
}
