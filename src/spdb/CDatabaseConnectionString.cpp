/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseConnectionString.cpp  -  description
                             -------------------
    begin                : Sun Mar 25 2012
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

#include <sptk5/db/CDatabaseConnectionString.h>
#include <sptk5/CStrings.h>
#include <sptk5/CRegExp.h>
#include <string.h>
#include <stdlib.h>

using namespace std;
using namespace sptk;

void CDatabaseConnectionString::parse() throw (CDatabaseException)
{
    CStrings    parts;
    int matchCount = CRegExp("^(\\w+)://(([\\w-_]+)(:(\\S+))?@)?([\\w-_]+)(:(\\d+))?(/([\\w_]+))?(\\?.*)?$").m(m_connectionString,parts);
    if (!parts.size())
        throwDatabaseException("Invalid connection string: m_connectionString");
    int i = 0;
    m_driverName = parts[i++];
    i++;
    m_userName = parts[i++];
    i++;
    m_password = parts[i++];
    m_hostName = parts[i++];
    i++;
    string port = parts[i++];
    if (!port.empty()) {
        m_portNumber = string2int(port);
        if (!m_portNumber)
            throw CDatabaseException("Invalid port number: " + m_connectionString);
    } else
        m_portNumber = 0;
    i++;
    if (i < parts.size()) {
        m_databaseName = parts[i++];

        if (i < parts.size()) {
            string parameters = parts[i++];

            CRegExp("([\\w_]+)=([\\w\\s/\\\\:-_\"']+)","g").m(parameters, parts);
            m_parameters.clear();
            for (unsigned i = 0; i < parts.size(); i+= 2)
                m_parameters[ parts[i] ] = parts[i+1];
        }
    }

    if (m_driverName != CRegExp("^(sqlite|postgres|oracle|mysql|mssql)"))
        throw CDatabaseException("Driver name not found: " + m_connectionString);

    if (m_userName.empty() && !m_password.empty())
        throw CDatabaseException("User name not found: " + m_connectionString);
}
