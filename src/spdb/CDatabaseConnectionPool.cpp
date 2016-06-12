/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDatabaseConnectionPool.cpp - description              ║
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

#include <sptk5/db/CDatabaseConnectionPool.h>
#ifndef WIN32
    /// *nix only
    #include <dlfcn.h>
#else
#endif

using namespace std;
using namespace sptk;

class DriverLoaders : public map<string, CDatabaseDriver*, CCaseInsensitiveCompare>
{
public:
    DriverLoaders() {}
    ~DriverLoaders()
    {
        for (iterator itor = begin(); itor != end(); itor++)
            delete itor->second;
    }
};

static DriverLoaders m_loadedDrivers;

CDatabaseConnectionPool::CDatabaseConnectionPool(std::string connectionString, unsigned maxConnections) :
    CDatabaseConnectionString(connectionString),
    m_driver(0),
    m_maxConnections(maxConnections)
{
}

bool closeConnectionCB(CDatabaseConnection*& item, void* data)
{
    CDatabaseConnection* connection = item;
    CDatabaseConnectionPool* connectionPool = (CDatabaseConnectionPool*)data;
    connectionPool->destroyConnection(connection,false);
    return true;
}

CDatabaseConnectionPool::~CDatabaseConnectionPool()
{
    m_connections.each(closeConnectionCB,this);
}

void CDatabaseConnectionPool::load() THROWS_EXCEPTIONS
{
    SYNCHRONIZED_CODE;

    string driverName = lowerCase(m_driverName);

    CDatabaseDriver* loadedDriver = m_loadedDrivers[driverName];
    if (loadedDriver) {
        m_driver = loadedDriver;
        m_createConnection = loadedDriver->m_createConnection;
        m_destroyConnection = loadedDriver->m_destroyConnection;
        return;
    }

    // Load the library
#ifdef WIN32
    string driverFileName = "spdb5_"+driverName+".dll";
    CDriverHandle handle = LoadLibrary(driverFileName.c_str());
    if (!handle)
        throw CDatabaseException("Cannot load library: " + driverFileName);
#else
    string driverFileName = "libspdb5_"+driverName+".so";

    CDriverHandle handle = dlopen(driverFileName.c_str(), RTLD_NOW);
    if (!handle)
        throw CDatabaseException("Cannot load library: " + string(dlerror()));
#endif

    // Creating the driver instance
    string create_connectionFunctionName(driverName + "_create_connection");
    string destroy_connectionFunctionName(driverName + "_destroy_connection");
#ifdef WIN32
    CCreateDriverInstance* createConnection = (CCreateDriverInstance*) GetProcAddress(handle, create_connectionFunctionName.c_str());
    if (!createConnection)
        throw CDatabaseException("Cannot load driver " + driverName + ": no function " + create_connectionFunctionName);

    CDestroyDriverInstance* destroyConnection = (CDestroyDriverInstance*) GetProcAddress(handle, destroy_connectionFunctionName.c_str());
    if (!destroyConnection)
        throw CDatabaseException("Cannot load driver " + driverName + ": no function " + destroy_connectionFunctionName);
#else
    // reset errors
    dlerror();

    // workaround for deficiency of C++ standard
    union {
        CCreateDriverInstance*  create_func_ptr;
        CDestroyDriverInstance* destroy_func_ptr;
        void*                   void_ptr;
    } conv;

    // load the symbols
    conv.void_ptr = dlsym(handle, create_connectionFunctionName.c_str());
    CCreateDriverInstance* createConnection = conv.create_func_ptr;

    CDestroyDriverInstance* destroyConnection = NULL;
    const char* dlsym_error = dlerror();
    if (!dlsym_error) {
        conv.void_ptr = dlsym(handle, destroy_connectionFunctionName.c_str());
        destroyConnection = conv.destroy_func_ptr;
        dlsym_error = dlerror();
    }

    if (dlsym_error) {
        m_createConnection = 0;
        dlclose(handle);
        throw CDatabaseException("Cannot load driver " + driverName + ": " + string(dlsym_error));
    }

#endif
    CDatabaseDriver* driver = new CDatabaseDriver;
    driver->m_handle = handle;
    driver->m_createConnection = createConnection;
    driver->m_destroyConnection = destroyConnection;

    m_createConnection = createConnection;
    m_destroyConnection = destroyConnection;

    // Registering loaded driver in the map
    m_loadedDrivers[driverName] = driver;
}

CDatabaseConnection* CDatabaseConnectionPool::createConnection() THROWS_EXCEPTIONS
{
    if (!m_driver)
        load();
    CDatabaseConnection* connection = NULL;
    if (m_connections.size() < m_maxConnections && m_pool.empty()) {
        connection = m_createConnection(str().c_str());
        m_connections.push_back(connection);
        return connection;
    }
    m_pool.pop(connection, 10000);
    return connection;
}

void CDatabaseConnectionPool::releaseConnection(CDatabaseConnection* connection)
{
    m_pool.push(connection);
}

void CDatabaseConnectionPool::destroyConnection(CDatabaseConnection* connection, bool unlink)
{
    if (unlink)
        m_connections.remove(connection);
    connection->close();
    m_destroyConnection(connection);
}
