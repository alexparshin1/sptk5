/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseDriverLoader.cpp  -  description
                             -------------------
    begin                : Sun Mar 11 2012
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
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

#include <sptk5/db/CDatabaseDriverLoader.h>
#ifndef WIN32
    /// *nix only
    #include <dlfcn.h>
#else
#endif

using namespace std;
using namespace sptk;

typedef map<string, CDatabaseDriverLoader*, CCaseInsensitiveCompare> DriverLoaders;
static DriverLoaders m_loadedDrivers;

CDatabaseDriverLoader::CDatabaseDriverLoader(std::string driverName) :
    m_handle(0),
    m_createDriverInstance(0),
    m_destroyDriverInstance(0),
    m_driverName(driverName)
{
}

void CDatabaseDriverLoader::load() throw (CDatabaseException)
{
    SYNCHRONIZED_CODE;

    string driverName = lowerCase(m_driverName);

    CDatabaseDriverLoader* loadedDriver = m_loadedDrivers[driverName];
    if (loadedDriver) {
        m_handle = loadedDriver->m_handle;
        m_createDriverInstance = loadedDriver->m_createDriverInstance;
        m_destroyDriverInstance = loadedDriver->m_destroyDriverInstance;
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
    string createDriverInstanceFunctionName(driverName + "_createDriverInstance");
    string destroyDriverInstanceFunctionName(driverName + "_destroyDriverInstance");
#ifdef WIN32
    m_createDriverInstance = (CCreateDriverInstance*) GetProcAddress(m_handle, createDriverInstanceFunctionName.c_str());
    if (!m_createDriverInstance) {
        m_handle = NULL;
        throw CDatabaseException("Cannot load driver " + driverName + ": no function " + createDriverInstanceFunctionName);
    }
    m_destroyDriverInstance = (CDestroyDriverInstance*) GetProcAddress(m_handle, destroyDriverInstanceFunctionName.c_str());
    if (!m_destroyDriverInstance) {
        m_handle = NULL;
        throw CDatabaseException("Cannot load driver " + driverName + ": no function " + destroyDriverInstanceFunctionName);
    }
#else
    // reset errors
    dlerror();

    // load the symbols
    m_createDriverInstance = (CCreateDriverInstance*) dlsym(m_handle, createDriverInstanceFunctionName.c_str());
    const char* dlsym_error = dlerror();
    if (!dlsym_error) {
        m_destroyDriverInstance = (CDestroyDriverInstance*) dlsym(m_handle, destroyDriverInstanceFunctionName.c_str());
        dlsym_error = dlerror();
    }

    if (dlsym_error) {
        m_createDriverInstance = 0;
        dlclose(m_handle);
        m_handle = NULL;
        throw CDatabaseException("Cannot load driver " + driverName + ": " + string(dlsym_error));
    }

#endif

    // Registering loaded driver in the map
    m_loadedDrivers[driverName] = this;
}

CDatabaseDriver* CDatabaseDriverLoader::createConnection(std::string connectString) throw (CDatabaseException)
{
    if (!m_handle)
        load();
    CDatabaseDriver* driver = m_createDriverInstance(connectString.c_str());
    return driver;
}

void CDatabaseDriverLoader::destroyConnection(CDatabaseDriver* driverInstance)
{
    m_destroyDriverInstance(driverInstance);
}
