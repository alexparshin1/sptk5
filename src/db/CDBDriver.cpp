/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDBDriver.cpp  -  description
                             -------------------
    begin                : Sun Mar 11 2012
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/db/CDBDriver.h>
#include <sptk5/CException.h>

using namespace std;
using namespace sptk;

extern "C" {
    CDBDriver* create_driver_instance(const char* connectString);
    void destroy_driver_instance(CDBDriver* driverInstance);
}

CDBDriver* create_driver_instance(const char* connectString)
{
    return new CDBDriver(connectString);
}

void destroy_driver_instance(CDBDriver* driverInstance)
{
    delete driverInstance;
}

CDBDriver::CDBDriver(string connectionString) :
        m_connString(connectionString)
{
    m_connected = false;
}

CDBDriver::~CDBDriver()
{
}

void CDBDriver::open(string newConnectionString) throw (CException)
{
    m_connected = true;
}

void CDBDriver::close() throw (CException)
{
    m_connected = false;
}

void CDBDriver::objectList(CStrings& objects) throw (std::exception)
{

}
