/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDBDriverLoader.cpp  -  description
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

#include <sptk5/db/CDBDriverLoader.h>
#ifndef WIN32
    /// *nix only
    #include <dlfcn.h>
#else
#endif

using namespace std;
using namespace sptk;

CDBDriverLoader::DriverLoaders CDBDriverLoader::m_loadedDrivers;

void CDBDriverLoader::load(std::string driverName) throw (exception)
{
    SYNCHRONIZED_CODE;

    CDBDriverLoader* loadedDriver = m_loadedDrivers[driverName];
    if (loadedDriver) {
        m_handle = loadedDriver->m_handle;
        m_createDriverInstance = loadedDriver->m_createDriverInstance;
        m_destroyDriverInstance = loadedDriver->m_destroyDriverInstance;
        return;
    }

    string driverFileName = "libspdb5_"+driverName+".so";

    m_handle = dlopen(driverFileName.c_str(), RTLD_NOW);
    if (!m_handle)
        throw CDatabaseException("Cannot load library: " + string(dlerror()));

    // reset errors
    dlerror();

    // load the symbols
    m_createDriverInstance = (CCreateDriverInstance*) dlsym(m_handle, (driverName + "_createDriverInstance").c_str());
    const char* dlsym_error = dlerror();
    if (!dlsym_error) {
        m_destroyDriverInstance = (CDestroyDriverInstance*) dlsym(m_handle, (driverName + "_destroyDriverInstance").c_str());
        dlsym_error = dlerror();
    }

    if (dlsym_error) {
        m_createDriverInstance = 0;
        dlclose(m_handle);
        m_handle = 0;
        throw CDatabaseException("Cannot load driver " + driverName + ": " + string(dlsym_error));
    }

    m_loadedDrivers[driverName] = this;
}
