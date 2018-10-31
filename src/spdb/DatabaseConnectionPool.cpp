/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseConnectionPool.cpp - description               ║
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

#include <sptk5/db/DatabaseConnectionPool.h>
#include <sptk5/SystemException.h>

#ifndef WIN32
    /// *nix only
    #include <dlfcn.h>
#else
#endif

using namespace std;
using namespace sptk;

class DriverLoaders
{
    map<string, DatabaseDriver*, CaseInsensitiveCompare> drivers;
public:
    DriverLoaders() = default;
    ~DriverLoaders()
    {
        for (auto itor: drivers)
            delete itor.second;
    }

    DatabaseDriver* get(const String& driverName)
    {
        auto itor = drivers.find(driverName);
        if (itor == drivers.end())
            return nullptr;
        return itor->second;
    }

    void add(const String& driverName, DatabaseDriver* driver)
    {
        auto itor = drivers.find(driverName);
        if (itor != drivers.end())
            delete itor->second;
        drivers[driverName] = driver;
    }
};

static DriverLoaders m_loadedDrivers;

DatabaseConnectionPool::DatabaseConnectionPool(const String& connectionString, unsigned maxConnections) :
    DatabaseConnectionString(connectionString),
    m_driver(nullptr),
    m_createConnection(nullptr),
    m_destroyConnection(nullptr),
    m_maxConnections(maxConnections)
{
}

bool DatabaseConnectionPool::closeConnectionCB(PoolDatabaseConnection*& item, void* data)
{
    PoolDatabaseConnection* connection = item;
    auto connectionPool = (DatabaseConnectionPool*)data;
    connectionPool->destroyConnection(connection, false);
    return true;
}

DatabaseConnectionPool::~DatabaseConnectionPool()
{
    m_connections.each(closeConnectionCB,this);
}

void DatabaseConnectionPool::load()
{
    lock_guard<mutex> lock(*this);

    String driverName = lowerCase(m_driverName);
    if (driverName == "mssql")
        driverName = "odbc";

    DatabaseDriver* loadedDriver = m_loadedDrivers.get(driverName);
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
        throw SystemException("Cannot load library " + driverFileName);
#else
    String driverFileName = String("libspdb5_") + driverName + String(".so");

    DriverHandle handle = dlopen(driverFileName.c_str(), RTLD_NOW);
    if (handle == nullptr)
        throw DatabaseException("Cannot load library: " + string(dlerror()));
#endif

    // Creating the driver instance
    String create_connectionFunctionName(driverName + String("_create_connection"));
    String destroy_connectionFunctionName(driverName + String("_destroy_connection"));
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
        throw DatabaseException(String("Cannot load driver ") + driverName + String(": ") + string(dlsym_error));
    }

#endif
    auto driver = new DatabaseDriver;
    driver->m_handle = handle;
    driver->m_createConnection = createConnection;
    driver->m_destroyConnection = destroyConnection;

    m_createConnection = createConnection;
    m_destroyConnection = destroyConnection;

    // Registering loaded driver in the map
    m_loadedDrivers.add(driverName, driver);
}

DatabaseConnection DatabaseConnectionPool::getConnection()
{
    return make_shared<AutoDatabaseConnection>(*this);
}

PoolDatabaseConnection* DatabaseConnectionPool::createConnection()
{
    if (m_driver == nullptr)
        load();
    PoolDatabaseConnection* connection = nullptr;
    if (m_connections.size() < m_maxConnections && m_pool.empty()) {
        connection = m_createConnection(toString().c_str());
        m_connections.push_back(connection);
        return connection;
    }
    m_pool.pop(connection, std::chrono::seconds(10));
    return connection;
}

void DatabaseConnectionPool::releaseConnection(PoolDatabaseConnection* connection)
{
    m_pool.push(connection);
}

void DatabaseConnectionPool::destroyConnection(PoolDatabaseConnection* connection, bool unlink)
{
    if (unlink)
        m_connections.remove(connection);
    try {
        connection->close();
    }
    catch (...) {}
    m_destroyConnection(connection);
}

#if USE_GTEST
#include <gtest/gtest.h>
#include <sptk5/db/DatabaseTests.h>

TEST(SPTK_PostgreSQLConnection, connect)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        databaseTests.testConnect(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, DDL)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        databaseTests.testDDL(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, bulkInsert)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        databaseTests.testBulkInsert(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}


TEST(SPTK_PostgreSQLConnection, queryParameters)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        databaseTests.testQueryParameters(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, transaction)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        databaseTests.testTransaction(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, connect)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        databaseTests.testConnect(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, DDL)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        databaseTests.testDDL(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, bulkInsert)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        databaseTests.testBulkInsert(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, queryParameters)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        databaseTests.testQueryParameters(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, transaction)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        databaseTests.testTransaction(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, connect)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        databaseTests.testConnect(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, DDL)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        databaseTests.testDDL(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, bulkInsert)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        databaseTests.testBulkInsert(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, queryParameters)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        databaseTests.testQueryParameters(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, transaction)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        databaseTests.testTransaction(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, connect)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        databaseTests.testConnect(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, DDL)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        databaseTests.testDDL(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, bulkInsert)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        databaseTests.testBulkInsert(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, queryParameters)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        databaseTests.testQueryParameters(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, transaction)
{
    DatabaseConnectionString connectionString = databaseTests.connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        databaseTests.testTransaction(connectionString);
    }
    catch (const exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

#endif
