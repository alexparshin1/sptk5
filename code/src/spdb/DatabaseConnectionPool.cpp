/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DatabaseConnectionPool.cpp - description               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#if USE_GTEST
#include <sptk5/db/DatabaseTests.h>
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
        for (auto& itor: drivers)
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
    auto* connectionPool = (DatabaseConnectionPool*)data;
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

    String driverNameLC = lowerCase(driverName());
    if (driverNameLC == "mssql")
        driverNameLC = "odbc";

    DatabaseDriver* loadedDriver = m_loadedDrivers.get(driverNameLC);
    if (loadedDriver != nullptr) {
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

    DriverHandle handle = dlopen(driverFileName.c_str(), RTLD_NOW);
    if (handle == nullptr)
        throw DatabaseException("Cannot load library: " + string(dlerror()));
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
    if (dlsym_error == nullptr) {
        ptr = dlsym(handle, destroy_connectionFunctionName.c_str());
        destroyConnection = (DestroyDriverInstance*) ptr;
        dlsym_error = dlerror();
    }

    if (dlsym_error != nullptr) {
        m_createConnection = nullptr;
        dlclose(handle);
        throw DatabaseException(String("Cannot load driver ") + driverNameLC + String(": ") + string(dlsym_error));
    }

#endif
    auto* driver = new DatabaseDriver;
    driver->m_handle = handle;
    driver->m_createConnection = createConnection;
    driver->m_destroyConnection = destroyConnection;

    m_createConnection = createConnection;
    m_destroyConnection = destroyConnection;

    // Registering loaded driver in the map
    m_loadedDrivers.add(driverNameLC, driver);
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
    catch (const Exception& e) {
        CERR(e.what() << endl)
    }
    m_destroyConnection(connection);
}

#if USE_GTEST

//───────────────────────────────── PostgreSQL ───────────────────────────────────────────
#if HAVE_POSTGRESQL

TEST(SPTK_PostgreSQLConnection, connect)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        DatabaseTests::testConnect(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, DDL)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        DatabaseTests::testDDL(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, bulkInsert)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        DatabaseTests::testBulkInsert(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, bulkInsertPerformance)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        DatabaseTests::testBulkInsertPerformance(connectionString, 1024);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, queryParameters)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        DatabaseTests::testQueryParameters(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, transaction)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        DatabaseTests::testTransaction(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_PostgreSQLConnection, select)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("postgresql");
    if (connectionString.empty())
        FAIL() << "PostgreSQL connection is not defined";
    try {
        DatabaseTests::testSelect(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

#endif

//───────────────────────────────── MySQL ────────────────────────────────────────────────
#if HAVE_MYSQL

TEST(SPTK_MySQLConnection, connect)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        DatabaseTests::testConnect(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, DDL)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        DatabaseTests::testDDL(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, bulkInsert)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        DatabaseTests::testBulkInsert(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, bulkInsertPerformance)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        DatabaseTests::testBulkInsertPerformance(connectionString, 1024);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, queryParameters)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        DatabaseTests::testQueryParameters(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, transaction)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        DatabaseTests::testTransaction(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MySQLConnection, select)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mysql");
    if (connectionString.empty())
        FAIL() << "MySQL connection is not defined";
    try {
        DatabaseTests::testSelect(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

#endif

//───────────────────────────────── Oracle ─────────────────────────────────────────────
#if HAVE_ORACLE

TEST(SPTK_OracleConnection, connect)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        DatabaseTests::testConnect(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, DDL)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        DatabaseTests::testDDL(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, bulkInsert)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        DatabaseTests::testBulkInsert(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, bulkInsertPerformance)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        DatabaseTests::testBulkInsertPerformance(connectionString, 1024);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, queryParameters)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        DatabaseTests::testQueryParameters(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, transaction)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oracle connection is not defined";
    try {
        DatabaseTests::testTransaction(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_OracleConnection, select)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("oracle");
    if (connectionString.empty())
        FAIL() << "Oralce connection is not defined";
    try {
        DatabaseTests::testSelect(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

#endif

//───────────────────────────────── MS SQL ─────────────────────────────────────────────
#if HAVE_ODBC

TEST(SPTK_MSSQLConnection, connect)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        DatabaseTests::testConnect(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, DDL)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        DatabaseTests::testDDL(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, bulkInsert)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        DatabaseTests::testBulkInsert(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, bulkInsertPerformance)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        DatabaseTests::testBulkInsertPerformance(connectionString, 1024);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, queryParameters)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        DatabaseTests::testQueryParameters(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, transaction)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        DatabaseTests::testTransaction(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

TEST(SPTK_MSSQLConnection, select)
{
    DatabaseConnectionString connectionString = DatabaseTests::tests().connectionString("mssql");
    if (connectionString.empty())
        FAIL() << "MSSQL connection is not defined";
    try {
        DatabaseTests::testSelect(connectionString);
    }
    catch (const Exception& e) {
        FAIL() << connectionString.toString() << ": " << e.what();
    }
}

#endif

#endif
