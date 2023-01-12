/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>
#include <sptk5/db/DatabaseConnectionPool.h>

#ifndef WIN32
#include <dlfcn.h>
#endif

using namespace std;
using namespace sptk;

class DriverLoaders
{
public:
    DatabaseDriver* get(const String& driverName)
    {
        auto itor = drivers.find(driverName.toLowerCase());
        if (itor == drivers.end())
        {
            return nullptr;
        }
        return itor->second.get();
    }

    void add(const String& driverName, const shared_ptr<DatabaseDriver>& driver)
    {
        drivers[driverName.toLowerCase()] = driver;
    }

    static DriverLoaders loadedDrivers;

private:
    map<string, shared_ptr<DatabaseDriver>, std::less<>> drivers;
};

DriverLoaders DriverLoaders::loadedDrivers;

DatabaseConnectionPool::DatabaseConnectionPool(const String& connectionString, unsigned maxConnections, chrono::seconds connectionTimeout)
    : DatabaseConnectionString(connectionString)
    , m_maxConnections(maxConnections)
    , m_connectionTimeout(connectionTimeout)
{
}

void DatabaseConnectionPool::load()
{
    scoped_lock lock(*this);

    String driverNameLC = lowerCase(driverName());
    if (driverNameLC == "mssql")
    {
        driverNameLC = "odbc";
    }

    if (auto* loadedDriver = DriverLoaders::loadedDrivers.get(driverNameLC); loadedDriver != nullptr)
    {
        m_driver = loadedDriver;
        m_createConnection = loadedDriver->m_createConnection;
        m_destroyConnection = loadedDriver->m_destroyConnection;
        return;
    }

    // Load the library
#ifdef WIN32
    string driverFileName = "spdb5_" + driverNameLC + ".dll";
    DriverHandle handle = LoadLibrary(driverFileName.c_str());
    if (!handle)
        throw SystemException("Cannot load library " + driverFileName);
#else
    String driverFileName = String("libspdb5_") + driverNameLC + String(".so");

    auto* handle = (DriverHandle) dlopen(driverFileName.c_str(), RTLD_NOW);
    if (handle == nullptr)
    {
        throw DatabaseException("Cannot load library: " + string(dlerror()));
    }
#endif

    // Creating the driver instance
    String create_connectionFunctionName(driverNameLC + String("_create_connection"));
    String destroy_connectionFunctionName(driverNameLC + String("_destroy_connection"));
#ifdef WIN32
    CreateDriverInstance* createConnection = (CreateDriverInstance*) GetProcAddress(handle, create_connectionFunctionName.c_str());
    if (!createConnection)
        throw DatabaseException("Cannot load driver " + driverNameLC + ": no function " + create_connectionFunctionName);

    DestroyDriverInstance* destroyConnection = (DestroyDriverInstance*) GetProcAddress(handle, destroy_connectionFunctionName.c_str());
    if (!destroyConnection)
        throw DatabaseException("Cannot load driver " + driverNameLC + ": no function " + destroy_connectionFunctionName);
#else
    // reset errors
    dlerror();

    // load the symbols
    void* ptr = dlsym(handle, create_connectionFunctionName.c_str());
    auto* createConnection = (CreateDriverInstance*) ptr;

    DestroyDriverInstance* destroyConnection = nullptr;
    const char* dlsym_error = dlerror();
    if (dlsym_error == nullptr)
    {
        ptr = dlsym(handle, destroy_connectionFunctionName.c_str());
        destroyConnection = (DestroyDriverInstance*) ptr;
        dlsym_error = dlerror();
    }

    if (dlsym_error != nullptr)
    {
        m_createConnection = nullptr;
        dlclose(handle);
        throw DatabaseException(String("Cannot load driver ") + driverNameLC + String(": ") + string(dlsym_error));
    }

#endif
    auto driver = make_shared<DatabaseDriver>();
    driver->m_handle = handle;
    driver->m_createConnection = createConnection;
    driver->m_destroyConnection = destroyConnection;

    m_createConnection = createConnection;
    m_destroyConnection = destroyConnection;

    // Registering loaded driver in the map
    DriverLoaders::loadedDrivers.add(driverNameLC, driver);
}

[[nodiscard]] DatabaseConnection DatabaseConnectionPool::getConnection()
{
    return make_shared<AutoDatabaseConnection>(*this);
}

SPoolDatabaseConnection DatabaseConnectionPool::createConnection()
{
    if (m_driver == nullptr)
    {
        load();
    }
    SPoolDatabaseConnection connection;
    if (m_connections.size() < m_maxConnections && m_pool.empty())
    {
        connection = SPoolDatabaseConnection(m_createConnection(toString().c_str(), m_connectionTimeout.count()),
                                             [this](PoolDatabaseConnection* conn) {
                                                 try
                                                 {
                                                     conn->close();
                                                 }
                                                 catch (const Exception& e)
                                                 {
                                                     CERR(e.what() << endl)
                                                 }
                                                 m_destroyConnection(conn);
                                             });
        m_connections.push_back(connection);
        return connection;
    }
    m_pool.pop(connection, std::chrono::seconds(10));
    return connection;
}

void DatabaseConnectionPool::releaseConnection(const SPoolDatabaseConnection& connection)
{
    m_pool.push(connection);
}
