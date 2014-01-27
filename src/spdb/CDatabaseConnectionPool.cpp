/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseConnectionPool.cpp  -  description
                             -------------------
    begin                : Sun Mar 11 2012
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the <ORGANIZATION> nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include <sptk5/db/CDatabaseConnectionPool.h>
#ifndef WIN32
    /// *nix only
    #include <dlfcn.h>
#else
#endif

using namespace std;
using namespace sptk;

typedef map<string, CDatabaseConnectionPool*, CCaseInsensitiveCompare> DriverLoaders;
static DriverLoaders m_loadedDrivers;

CDatabaseConnectionPool::CDatabaseConnectionPool(std::string connectionString, unsigned maxConnections) :
    CDatabaseConnectionString(connectionString),
    m_handle(0),
    m_createConnection(0),
    m_destroyConnection(0),
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

void CDatabaseConnectionPool::load() throw (CDatabaseException)
{
    SYNCHRONIZED_CODE;

    string driverName = lowerCase(m_driverName);

    CDatabaseConnectionPool* loadedDriver = m_loadedDrivers[driverName];
    if (loadedDriver) {
        m_handle = loadedDriver->m_handle;
        m_createConnection = loadedDriver->m_createConnection;
        m_destroyConnection = loadedDriver->m_destroyConnection;
        return;
    }

    // Load the library
#ifdef WIN32
    string driverFileName = "spdb5_"+driverName+".dll";
    m_handle = LoadLibrary (driverFileName.c_str());
    if (!m_handle)
        throw CDatabaseException("Cannot load library: " + driverFileName);
#else
    string driverFileName = "libspdb5_"+driverName+".so";

    m_handle = dlopen(driverFileName.c_str(), RTLD_NOW);
    if (!m_handle)
        throw CDatabaseException("Cannot load library: " + string(dlerror()));
#endif

    // Creating the driver instance
    string create_connectionFunctionName(driverName + "_create_connection");
    string destroy_connectionFunctionName(driverName + "_destroy_connection");
#ifdef WIN32
    m_createConnection = (CCreateDriverInstance*) GetProcAddress(m_handle, create_connectionFunctionName.c_str());
    if (!m_createConnection) {
        m_handle = NULL;
        throw CDatabaseException("Cannot load driver " + driverName + ": no function " + create_connectionFunctionName);
    }
    m_destroyConnection = (CDestroyDriverInstance*) GetProcAddress(m_handle, destroy_connectionFunctionName.c_str());
    if (!m_destroyConnection) {
        m_handle = NULL;
        throw CDatabaseException("Cannot load driver " + driverName + ": no function " + destroy_connectionFunctionName);
    }
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
    conv.void_ptr = dlsym(m_handle, create_connectionFunctionName.c_str());
    m_createConnection = conv.create_func_ptr;

    const char* dlsym_error = dlerror();
    if (!dlsym_error) {
        conv.void_ptr = dlsym(m_handle, destroy_connectionFunctionName.c_str());
        m_destroyConnection = conv.destroy_func_ptr;
        dlsym_error = dlerror();
    }

    if (dlsym_error) {
        m_createConnection = 0;
        dlclose(m_handle);
        m_handle = NULL;
        throw CDatabaseException("Cannot load driver " + driverName + ": " + string(dlsym_error));
    }

#endif

    // Registering loaded driver in the map
    m_loadedDrivers[driverName] = this;
}

CDatabaseConnection* CDatabaseConnectionPool::createConnection() throw (CDatabaseException)
{
    if (!m_handle)
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
