/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseConnectionString.cpp  -  description
                             -------------------
    begin                : Sun Mar 25 2012
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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
#include <string.h>
#include <stdlib.h>

using namespace std;
using namespace sptk;

static const CStrings driverNames("sqlite3|postgres|postgresql|oracle|mysql|firebird|odbc", "|");

void CDatabaseConnectionString::parse() THROWS_EXCEPTIONS
{
    size_t pos;
    string connStr(m_connectionString);

    // Find extra parameters
    pos = connStr.find_first_of("?");
    if (pos != string::npos) {
        CStrings parameters(connStr.substr(pos + 1),"&");
        for (CStrings::iterator item = parameters.begin(); item != parameters.end(); item++) {
            CStrings pair(*item, "='");
            if (pair.size() == 2)
                m_parameters[ pair[0] ] = pair[1];
        }
        connStr.erase(pos);
    }

    pos = connStr.find("://");
    if (pos != string::npos) {
        m_driverName = connStr.substr(0, pos);
        connStr.erase(0, pos + 3);
        if (driverNames.indexOf(m_driverName) < 0)
            throwDatabaseException("Driver name is unknown: " + m_connectionString);
    } else
        throwDatabaseException("Driver name is missing: " + m_connectionString);

    pos = connStr.find("@");
    if (pos != string::npos) {
        CStrings usernameAndPassword(connStr.substr(0, pos),":");
        m_userName = usernameAndPassword[0];
        if (usernameAndPassword.size() > 1)
            m_password = usernameAndPassword[1];
        connStr.erase(0, pos + 1);
    }

    pos = connStr.find("/");
    if (pos != string::npos) {
        m_databaseName = connStr.substr(pos + 1);
        connStr.erase(pos);
        if (m_databaseName.find("/") != string::npos)
            m_databaseName = "/" + m_databaseName;
    }

    CStrings hostAndPort(connStr, ":");
    m_hostName = hostAndPort[0];
    if (hostAndPort.size() > 1)
        m_portNumber = (uint16_t) atoi(hostAndPort[1].c_str());
}
