/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseConnectionPool.cpp - description               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/db/DatabaseConnectionPool.h>
#ifndef WIN32
    /// *nix only
    #include <dlfcn.h>
#else
#endif

using namespace std;
using namespace sptk;

class DriverLoaders : public map<string, DatabaseDriver*, CaseInsensitiveCompare>
{
public:
    DriverLoaders() = default;
    ~DriverLoaders()
    {
        for (auto itor: *this)
            delete itor.second;
    }
};

static DriverLoaders m_loadedDrivers;

DatabaseConnectionPool::DatabaseConnectionPool(const string& connectionString, unsigned maxConnections) :
    DatabaseConnectionString(connectionString),
    m_driver(nullptr),
    m_maxConnections(maxConnections)
{
}

bool closeConnectionCB(DatabaseConnection*& item, void* data)
{
    DatabaseConnection* connection = item;
    auto connectionPool = (DatabaseConnectionPool*)data;
    connectionPool->destroyConnection(connection,false);
    return true;
}

DatabaseConnectionPool::~DatabaseConnectionPool()
{
    m_connections.each(closeConnectionCB,this);
}

void DatabaseConnectionPool::load()
{
    SYNCHRONIZED_CODE;

    string driverName = lowerCase(m_driverName);
    if (driverName == "mssql")
        driverName = "odbc";

    DatabaseDriver* loadedDriver = m_loadedDrivers[driverName];
    if (loadedDriver != nullptr) {
        m_driver = loadedDriver;
        m_createConnection = loadedDriver->m_createConnection;
        m_destroyConnection = loadedDriver->m_destroyConnection;
        return;
    }

    // Load the library
#ifdef WIN32
    string driverFileName = "spdb5_"+driverName+".dll";
    DriverHandle handle = LoadLibrary(driverFileName.c_str());
    if (!handle)
        throw DatabaseException("Cannot load library: " + driverFileName);
#else
    string driverFileName = "libspdb5_"+driverName+".so";

    DriverHandle handle = dlopen(driverFileName.c_str(), RTLD_NOW);
    if (handle == nullptr)
        throw DatabaseException("Cannot load library: " + string(dlerror()));
#endif

    // Creating the driver instance
    string create_connectionFunctionName(driverName + "_create_connection");
    string destroy_connectionFunctionName(driverName + "_destroy_connection");
#ifdef WIN32
    CreateDriverInstance* createConnection = (CreateDriverInstance*) GetProcAddress(handle, create_connectionFunctionName.c_str());
    if (!createConnection)
        throw DatabaseException("Cannot load driver " + driverName + ": no function " + create_connectionFunctionName);

    DestroyDriverInstance* destroyConnection = (DestroyDriverInstance*) GetProcAddress(handle, destroy_connectionFunctionName.c_str());
    if (!destroyConnection)
        throw DatabaseException("Cannot load driver " + driverName + ": no function " + destroy_connectionFunctionName);
#else
    // reset errors
    dlerror();

    // workaround for deficiency of C++ standard
    union {
        CreateDriverInstance*  create_func_ptr;
        DestroyDriverInstance* destroy_func_ptr;
        void*                   void_ptr;
    } conv = {};

    // load the symbols
    conv.void_ptr = dlsym(handle, create_connectionFunctionName.c_str());
    CreateDriverInstance* createConnection = conv.create_func_ptr;

    DestroyDriverInstance* destroyConnection = nullptr;
    const char* dlsym_error = dlerror();
    if (dlsym_error == nullptr) {
        conv.void_ptr = dlsym(handle, destroy_connectionFunctionName.c_str());
        destroyConnection = conv.destroy_func_ptr;
        dlsym_error = dlerror();
    }

    if (dlsym_error != nullptr) {
        m_createConnection = nullptr;
        dlclose(handle);
        throw DatabaseException("Cannot load driver " + driverName + ": " + string(dlsym_error));
    }

#endif
    auto driver = new DatabaseDriver;
    driver->m_handle = handle;
    driver->m_createConnection = createConnection;
    driver->m_destroyConnection = destroyConnection;

    m_createConnection = createConnection;
    m_destroyConnection = destroyConnection;

    // Registering loaded driver in the map
    m_loadedDrivers[driverName] = driver;
}

DatabaseConnection* DatabaseConnectionPool::createConnection()
{
    if (m_driver == nullptr)
        load();
    DatabaseConnection* connection = nullptr;
    if (m_connections.size() < m_maxConnections && m_pool.empty()) {
        connection = m_createConnection(toString().c_str());
        m_connections.push_back(connection);
        return connection;
    }
    m_pool.pop(connection, std::chrono::seconds(10));
    return connection;
}

void DatabaseConnectionPool::releaseConnection(DatabaseConnection* connection)
{
    m_pool.push(connection);
}

void DatabaseConnectionPool::destroyConnection(DatabaseConnection* connection, bool unlink)
{
    if (unlink)
        m_connections.remove(connection);
    connection->close();
    m_destroyConnection(connection);
}
