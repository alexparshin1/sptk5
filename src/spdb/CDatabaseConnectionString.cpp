/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseConnectionString.cpp  -  description
                             -------------------
    begin                : Sun Mar 25 2012
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

#include <sptk5/db/CDatabaseConnectionString.h>
#include <string.h>
#include <stdlib.h>

using namespace std;
using namespace sptk;

void CDatabaseConnectionString::parseParameters(char* ptr) throw (CDatabaseException)
{
    m_parameters.clear();
    char* start = ptr;
    while (start) {
        char* end = strchr(ptr, '&');
        if (end) {
            *end = 0;
            end++;
        }
        char* name = start;
        char* value = strchr(name, '=');
        if (!value)
            throw CDatabaseException("Parameter " + string(name) + " is invalid");
        *value = 0;
        value++;
        m_parameters[name] = value;
        start = end;
    }
}

void CDatabaseConnectionString::parse() throw (CDatabaseException)
{
    char* temp = strdup(m_connectionString.c_str());
    try {
        char* ptr = strchr(temp, '?');
        if (ptr) {
            *ptr = 0;
            parseParameters(ptr + 1);
        }

        // Locate the protocol
        char* start = temp;
        ptr = strstr(start, "://");
        if (!ptr)
            throw CDatabaseException("Driver name not found: " + m_connectionString);
        *ptr = 0;
        m_driverName = start;
        start = ptr + 3;

        // Locate optional user name and password
        ptr = strchr(start, '@');
        if (ptr) {
            char* name = start;
            char* password = strchr(name, ':');
            if (password) {
                *password = 0;
                password++;
                m_password = password;
            }
            m_userName = name;
            ptr = 0;
            start = ptr + 1;
        }

        // Locate database name
        ptr = strchr(start, '/');
        if (ptr) {
            *ptr = 0;
            m_databaseName = ptr + 1;
        }

        // Locate server port
        char* port = strchr(start, '/');
        if (port) {
            *port = 0;
            m_portNumber = (uint16_t) atoi(port + 1);
            if (!m_portNumber)
                throw CDatabaseException("Invalid port number: " + m_connectionString);
        }

        m_hostName = start;

        free(temp);
    }
    catch (...) {
        free(temp);
        throw;
    }
}
