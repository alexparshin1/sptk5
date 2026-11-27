/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
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

DriverLoaders DriverLoaders::loadedDrivers;

DatabaseConnectionPool::DatabaseConnectionPool(const String& connectionString, const unsigned maxConnections,
                                                 const chrono::milliseconds connectionTimeout)
    : DatabaseConnectionString(connectionString)
    , m_maxConnections(maxConnections)
    , m_connectionTimeout(connectionTimeout)
{
    m_core->connectionString = DatabaseConnectionString(connectionString);
}

DatabaseConnectionPool::~DatabaseConnectionPool()
{
    m_core->shutdown();
}

void DatabaseConnectionPoolCore::shutdown()
{
    const scoped_lock lock(mutex);
    destroyed = true;
    // Closing can throw (busy database, network failure). A destructor must never propagate.
    // Connections still checked out must survive the pool: drop them from the pool's list but
    // leave the shared_ptr held by their AutoDatabaseConnection intact.
    vector<SPoolDatabaseConnection> stillBorrowed;
    connections.each([this, &stillBorrowed](const SPoolDatabaseConnection& connection)
                     {
                         if (borrowed.find(connection.get()) != borrowed.end())
                         {
                             stillBorrowed.push_back(connection);
                             return true;
                         }
                         try
                         {
                             if (connection->active())
                             {
                                 connection->close();
                             }
                         }
                         catch (const Exception& e)
                         {
                             CERR(e.what());
                         }
                         return true;
                     });
    for (auto& connection: stillBorrowed)
    {
        connections.remove(connection);
    }
    pool.clear();
    connections.clear();
}

SPoolDatabaseConnection DatabaseConnectionPoolCore::create(const String& connectionString,
                                                           const size_t maxConnections,
                                                           const chrono::milliseconds connectionTimeout)
{
    // The size check, creation and registration must be one atomic operation: without the lock two
    // threads can both see a free slot and exceed maxConnections.
    const scoped_lock lock(mutex);
    if (destroyed)
    {
        return {};
    }

    if (connections.size() < maxConnections && pool.empty())
    {
        auto connection = SPoolDatabaseConnection(
            createConnection(connectionString.c_str(),
                             chrono::duration_cast<chrono::seconds>(connectionTimeout).count()),
            [self = shared_from_this()](PoolDatabaseConnection* conn)
            {
                try
                {
                    if (conn->active())
                    {
                        conn->close();
                    }
                }
                catch (const Exception& e)
                {
                    CERR(e.what());
                }
                if (self->destroyConnection != nullptr)
                {
                    self->destroyConnection(conn);
                }
            });
        connections.push_back(connection);
        borrowed.insert(connection.get());
        return connection;
    }

    SPoolDatabaseConnection connection;
    if (pool.pop_front(connection, connectionTimeout))
    {
        borrowed.insert(connection.get());
        return connection;
    }
    return {};
}

void DatabaseConnectionPoolCore::release(const SPoolDatabaseConnection& connection)
{
    const scoped_lock lock(mutex);
    borrowed.erase(connection.get());
    if (connection == nullptr || destroyed)
    {
        // The pool is gone: the connection's shared_ptr deleter will release the driver instance.
        return;
    }
    pool.push_back(connection);
}

void DatabaseConnectionPool::load()
{
    const scoped_lock lock(m_core->mutex);

    String driverNameLC = lowerCase(driverName());
    if (driverNameLC == "mssql")
    {
        driverNameLC = "odbc";
    }

    if (const auto loadedDriver = DriverLoaders::loadedDrivers.get(driverNameLC))
    {
        m_core->driver = loadedDriver;
        m_core->createConnection = loadedDriver->m_createConnection;
        m_core->destroyConnection = loadedDriver->m_destroyConnection;
        return;
    }

    // Load the library
#ifdef WIN32
    string       driverFileName = "spdb5_" + driverNameLC + ".dll";
    DriverHandle handle = LoadLibrary(driverFileName.c_str());
    if (!handle)
        throw SystemException("Cannot load library " + driverFileName);
#else
    // By the full soname, which carries the SPTK version: the driver that matches this libspdb5 and
    // no other. The unversioned libspdb5_<driver>.so is a development link - an installation that
    // has two SPTK releases in one directory, or none of the development files, would either load
    // whichever release that link happens to point at or fail to find a driver that is there.
    const String driverFileName = String("libspdb5_") + driverNameLC + String(".so.") + String(SPTK_DRIVER_SONAME_VERSION);

    auto* handle = static_cast<DriverHandle>(dlopen(driverFileName.c_str(), RTLD_NOW));
    if (handle == nullptr)
    {
        throw DatabaseException("Cannot load library: " + string(dlerror()));
    }
#endif

    // Creating the driver instance
    const auto createConnectionFunctionName = driverNameLC + "CreateConnection";
    const auto destroyConnectionFunctionName = driverNameLC + "DestroyConnection";
#ifdef WIN32
    CreateDriverInstance* createConnection = (CreateDriverInstance*) GetProcAddress(handle, createConnectionFunctionName.c_str());
    if (!createConnection)
    {
        FreeLibrary(handle);
        throw DatabaseException("Cannot load driver " + driverNameLC + ": no function " + createConnectionFunctionName);
    }

    DestroyDriverInstance* destroyConnection = (DestroyDriverInstance*) GetProcAddress(handle, destroyConnectionFunctionName.c_str());
    if (!destroyConnection)
    {
        createConnection = nullptr;
        destroyConnection = nullptr;
        FreeLibrary(handle);
        throw DatabaseException("Cannot load driver " + driverNameLC + ": no function " + destroyConnectionFunctionName);
    }
#else
    // reset errors
    dlerror();

    // load the symbols
    void* ptr = dlsym(handle, createConnectionFunctionName.c_str());
    auto* createConnection = bit_cast<CreateDriverInstance*>(ptr);

    DestroyDriverInstance* destroyConnection = nullptr;
    const char*            dlsymError = dlerror();
    if (dlsymError == nullptr)
    {
        ptr = dlsym(handle, destroyConnectionFunctionName.c_str());
        destroyConnection = bit_cast<DestroyDriverInstance*>(ptr);
        dlsymError = dlerror();
    }

    if (dlsymError != nullptr)
    {
        m_core->createConnection = nullptr;
        m_core->destroyConnection = nullptr;
        dlclose(handle);
        throw DatabaseException(String("Cannot load driver ") + driverNameLC + String(": ") + string(dlsymError));
    }

#endif
    m_core->driver = make_shared<DatabaseDriver>();
    m_core->driver->m_handle = handle;
    m_core->driver->m_createConnection = createConnection;
    m_core->driver->m_destroyConnection = destroyConnection;

    m_core->createConnection = createConnection;
    m_core->destroyConnection = destroyConnection;

    // Registering the loaded driver in the map
    DriverLoaders::loadedDrivers.add(driverNameLC, m_core->driver);
}

[[nodiscard]] DatabaseConnection DatabaseConnectionPool::getConnection()
{
    return make_shared<AutoDatabaseConnection>(*this);
}

SPoolDatabaseConnection DatabaseConnectionPool::createConnection()
{
    if (m_core->createConnection == nullptr)
    {
        load();
    }
    return m_core->create(toString(), m_maxConnections, m_connectionTimeout);
}

void DatabaseConnectionPool::releaseConnection(const SPoolDatabaseConnection& connection)
{
    m_core->release(connection);
}

size_t DatabaseConnectionPool::totalConnections() const
{
    return m_core->connections.size();
}

size_t DatabaseConnectionPool::availableConnections() const
{
    return m_core->pool.size();
}
