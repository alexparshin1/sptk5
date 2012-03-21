/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDBDriverLoader.h  -  description
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

#ifndef __CDATABASE_H__
#define __CDATABASE_H__

#include <sptk5/db/CDBDriver.h>
#include <sptk5/CCaseInsensitiveCompare.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

typedef CDBDriver* CCreateDriverInstance(std::string);
typedef void CDestroyDriverInstance(CDBDriver*);

#ifdef WIN32
    typedef HMODULE CDriverHandle;                   ///< Windows: Driver DLL handle type
#else
    typedef void*   CDriverHandle;                   ///< Unix: Driver SO/DLL handle type
#endif

/// @brief Database driver description
class SP_EXPORT CDBDriverLoader : public CSynchronized
{
protected:
    CDriverHandle           m_handle;                   ///< Driver SO/DLL handle after load
    CCreateDriverInstance*  m_createDriverInstance;     ///< Function that creates driver instances
    CDestroyDriverInstance* m_destroyDriverInstance;    ///< Function that destroys driver instances
public:
    /// @brief Constructor
    /// @param handle CDriverHandle, Handle of loaded driver library
    /// @param createDriverInstance CCreateDriverInstance*, Function that creates driver instances
    /// @param destroyDriverInstance CDestroyDriverInstance*, Function that destroys driver instances
    CDBDriverLoader(CDriverHandle handle = 0, CCreateDriverInstance* createDriverInstance = 0, CDestroyDriverInstance* destroyDriverInstance = 0)
    {
        m_handle = handle;
        m_createDriverInstance = createDriverInstance;
        m_destroyDriverInstance = destroyDriverInstance;
    }

    /// @brief Loads driver by name
    ///
    /// First successful driver load places driver into driver cache.
    void load(std::string driverName) throw (std::exception);

    /// @brief Creates driver instance
    /// @param connectionString std::string, Connect string
    CDBDriver* createDriverInstance(std::string connectString)
    {
        return m_createDriverInstance(connectString);
    }

    /// @brief Destroys driver instance
    /// @param driverInstance CDBDriver*, destroys the driver instance
    void destroyDriverInstance(CDBDriver* driverInstance)
    {
        m_destroyDriverInstance(driverInstance);
    }
};
/// @}
}
#endif
