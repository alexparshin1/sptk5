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

#include <sptk5/db/OracleEnvironment.h>

using namespace std;
using namespace sptk;
using namespace oracle::occi;

String OracleEnvironment::clientVersion()
{
    int major = 0;
    int minor = 0;
    int update = 0;
    int patch = 0;
    int portUpdate = 0;
    Environment::getClientVersion(major, minor, update, patch, portUpdate);
    string version = "Oracle " + int2string(major) + "." + int2string(minor);
    if (update)
    {
        version += " update " + int2string(update);
    }
    return version;
}

oracle::occi::Connection* OracleEnvironment::createConnection(const DatabaseConnectionString& connectionString)
{
    String host = connectionString.hostName();
    if (connectionString.portNumber())
    {
        host += ":" + int2string(connectionString.portNumber());
    }
    if (connectionString.databaseName().length())
    {
        host += "/" + connectionString.databaseName();
    }
    return m_handle->createConnection(connectionString.userName(), connectionString.password(), host);
}

void OracleEnvironment::terminateConnection(oracle::occi::Connection* connection)
{
    m_handle->terminateConnection(connection);
}
