/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CODBC.cpp - description                                ║
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

#include <cstdio>
#include <cstring>

#include <sptk5/db/ODBCEnvironment.h>

using namespace std;
using namespace sptk;

static const char cantSetConnectOption[] = "Can't set connect option";
static const char cantEndTranscation[] = "Can't end transaction";
static const char cantGetInformation[] = "Can't get connect information";

// Returns true if result code indicates success
static inline bool Successful(RETCODE ret)
{
    return ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO;
}

void ODBCBase::exception(string text, int line) const
{
    throw DatabaseException(text, __FILE__, line);
}

//---------------------------------------------------------------------------
// ODBC Environment class
//---------------------------------------------------------------------------
ODBCEnvironment::ODBCEnvironment()
: m_hEnvironment(SQL_NULL_HENV)
{
}

ODBCEnvironment::~ODBCEnvironment()
{
    if (valid())
        freeEnv();
}

void ODBCEnvironment::allocEnv()
{
    if (valid())
        return; // Already allocated
    lock_guard<mutex> lock(m_mutex);
    if (!Successful(SQLAllocEnv(&m_hEnvironment))) {
        m_hEnvironment = SQL_NULL_HENV;
        exception("Can't allocate ODBC environment", __LINE__);
    }
}

void ODBCEnvironment::freeEnv()
{
    if (!valid())
        return; // Never allocated
    lock_guard<mutex> lock(m_mutex);
    SQLFreeEnv(m_hEnvironment);
    m_hEnvironment = SQL_NULL_HENV;
}

//--------------------------------------------------------------------------------------------
// ODBC Connection class
//--------------------------------------------------------------------------------------------

ODBCConnectionBase::ODBCConnectionBase()
: m_cEnvironment(getEnvironment()),
  m_hConnection(SQL_NULL_HDBC),
  m_connected(false)
{
}

ODBCConnectionBase::~ODBCConnectionBase()
{
    if (isConnected())
        disconnect();
    freeConnect();
}

// Static environment object inside this function
ODBCEnvironment& ODBCConnectionBase::getEnvironment()
{
    static ODBCEnvironment Env;
    return Env;
}

void ODBCConnectionBase::allocConnect()
{
    // If already connected, return false
    if (valid())
        return;

    // Allocate environment if not already done
    m_cEnvironment.allocEnv();

    lock_guard<mutex> lock(m_mutex);

    // Create connection handle
    if (!Successful(SQLAllocConnect(m_cEnvironment.handle(), &m_hConnection))) {
        m_hConnection = SQL_NULL_HDBC;
        exception(errorInformation("Can't alloc connection"), __LINE__);
    }
}

void ODBCConnectionBase::freeConnect()
{
    if (!valid())
        return; // Not connected
    if (isConnected())
        disconnect();

    lock_guard<mutex> lock(m_mutex);

    SQLFreeConnect(m_hConnection);
    m_hConnection = SQL_NULL_HDBC;
    m_connected = false;
    m_connectString = "";
}

void ODBCConnectionBase::connect(const string& ConnectionString, string& pFinalString, bool /*EnableDriverPrompt*/)
{
    // Check parameters
    if (ConnectionString.empty())
        exception("Can'connect: connection string is empty", __LINE__);

    // If handle not allocated, allocate it
    allocConnect();

    // If we are  already connected, disconnect
    if (isConnected())
        disconnect();

    lock_guard<mutex> lock(m_mutex);

    m_connectString = ConnectionString;

    auto* buff = new char[2048];
    SWORD bufflen = 0;

#ifdef WIN32
    HWND ParentWnd = 0;
#else
    void* ParentWnd = nullptr;
#endif
    SQLRETURN rc = ::SQLDriverConnect(m_hConnection, ParentWnd, (UCHAR FAR*) ConnectionString.c_str(), SQL_NTS,
                                      (UCHAR FAR*) buff, (short) 2048, &bufflen, SQL_DRIVER_NOPROMPT);

    if (!Successful(rc)) {
        string errorInfo = errorInformation(("SQLDriverConnect(" + ConnectionString + ")").c_str());
        exception(errorInfo, __LINE__);
    }
    pFinalString = buff;
    delete[] buff;

    m_connected = true;
    m_connectString = pFinalString;

    // Trying to get more information about the driver
    auto* driverDescription = new SQLCHAR[2048];
    SQLSMALLINT descriptionLength = 0;
    rc = SQLGetInfo(m_hConnection, SQL_DBMS_NAME, driverDescription, 2048, &descriptionLength);
    if (Successful(rc))
        m_driverDescription = (char*) driverDescription;

    rc = SQLGetInfo(m_hConnection, SQL_DBMS_VER, driverDescription, 2048, &descriptionLength);
    if (Successful(rc))
        m_driverDescription += " " + string((char*) driverDescription);

    delete[] driverDescription;
}

void ODBCConnectionBase::disconnect()
{
    if (!isConnected())
        return; // Not connected

    lock_guard<mutex> lock(m_mutex);

    SQLDisconnect(m_hConnection);
    m_connected = false;
    m_connectString = "";
}

void ODBCConnectionBase::setConnectOption(UWORD fOption, UDWORD vParam)
{
    if (!isConnected())
        exception(errorInformation(cantSetConnectOption), __LINE__);

    lock_guard<mutex> lock(m_mutex);

    if (!Successful(SQLSetConnectOption(m_hConnection, fOption, vParam)))
        exception(errorInformation(cantSetConnectOption), __LINE__);
}

void ODBCConnectionBase::execQuery(const char* query)
{
    SQLHSTMT hstmt = SQL_NULL_HSTMT;

    // Allocate Statement Handle
    if (!Successful(SQLAllocHandle(SQL_HANDLE_STMT, m_hConnection, &hstmt)))
        throw Exception("Can't allocate handle");
    if (!Successful(SQLExecDirect(hstmt, (SQLCHAR*) query, strlen(query)))) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        throw Exception("Can't execute query: " + String(query));
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
}

void ODBCConnectionBase::beginTransaction()
{
    execQuery("BEGIN TRANSACTION");
}

void ODBCConnectionBase::transact(UWORD fType)
{
    if (!isConnected())
        exception(string(cantEndTranscation) + "Not connected to the database", __LINE__);

    lock_guard<mutex> lock(m_mutex);

    if (fType == SQL_COMMIT)
        execQuery("COMMIT");
    else
        execQuery("ROLLBACK");
}

//==============================================================================
String sptk::removeDriverIdentification(const char* error)
{
    if (error == nullptr)
        return "";

    auto* p = (char*) error;
    const char* p1 = error;
    while (p1 != nullptr) {
        p1 = strstr(p, "][");
        if (p1 != nullptr)
            p = (char*) p1 + 1;
    }
    p1 = strstr(p, "]");
    if (p1 != nullptr)
        p = (char*) p1 + 1;
    p1 = strstr(p, "sqlerrm(");
    if (p1 != nullptr) {
        p = (char*) p1 + 8;
        auto len = (int) strlen(p);
        if (p[len - 1] == ')')
            p[len - 1] = 0;
    }
    return p;
}

string extract_error(
        SQLHANDLE handle,
        SQLSMALLINT type)
{
    SQLSMALLINT i = 0;
    SQLINTEGER native;
    SQLCHAR state[7];
    SQLCHAR text[256];
    SQLSMALLINT len;
    SQLRETURN ret;

    string error;
    for (;;) {
        i++;
        ret = SQLGetDiagRec(type, handle, i, state, &native, text, sizeof(text), &len);
        if (ret != SQL_SUCCESS)
            break;
        error += removeDriverIdentification((char*) text) + string(". ");
    }

    return error;
}

string ODBCConnectionBase::errorInformation(const char* function)
{
    char errorDescription[SQL_MAX_MESSAGE_LENGTH];
    char errorState[SQL_MAX_MESSAGE_LENGTH];
    SWORD pcnmsg = 0;
    SQLINTEGER nativeError = 0;
    *errorState = 0;
    *errorDescription = 0;

    int rc = SQLError(SQL_NULL_HENV, m_hConnection, SQL_NULL_HSTMT, (UCHAR FAR*) errorState, &nativeError,
                      (UCHAR FAR*) errorDescription, sizeof(errorDescription), &pcnmsg);
    if (rc == SQL_SUCCESS)
        return extract_error(m_hConnection, SQL_HANDLE_DBC);

    rc = SQLError(m_cEnvironment.handle(), SQL_NULL_HDBC, SQL_NULL_HSTMT, (UCHAR FAR*) errorState,
                  &nativeError, (UCHAR FAR*) errorDescription, sizeof(errorDescription), &pcnmsg);
    if (rc != SQL_SUCCESS)
        exception(cantGetInformation, __LINE__);

    return String(function) + ": " + removeDriverIdentification(errorDescription);
}
