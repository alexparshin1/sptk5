/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CSSLSocket.cpp  -  description
                             -------------------
    begin                : Oct 30 2014
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
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

#include <sptk5/sptk.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sptk5/net/CSSLSocket.h>
#include <sptk5/threads/CThread.h>

using namespace std;
using namespace sptk;

void CSSLSocket::throwSSLError(int rc)
{
    int errorCode = SSL_get_error(m_ssl, rc);
    string error = getSSLError("SSL_connect", errorCode);
    throw CException(error, __FILE__, __LINE__);
}

CSSLSocket::CSSLSocket(CSSLContext& sslContext)
: m_ssl(SSL_new(sslContext.handle()))
{
}

CSSLSocket::~CSSLSocket()
{
    SSL_free(m_ssl);
}

void CSSLSocket::open(string hostName, uint32_t port, CSocketOpenMode openMode) THROWS_EXCEPTIONS
{
    CTCPSocket::open(hostName, port, openMode);

    SYNCHRONIZED_CODE;

    SSL_set_fd(m_ssl, (int) m_sockfd);

    int rc = SSL_connect(m_ssl);
    if (rc <= 0) {
        close();
        throwSSLError(rc);
    }
}

void CSSLSocket::close()
{
    SSL_set_fd(m_ssl, -1);
    CTCPSocket::close();
}

void CSSLSocket::attach(SOCKET socketHandle) throw (std::exception)
{
    SYNCHRONIZED_CODE;

    int rc = 1;
    if (m_sockfd != socketHandle) {
        CTCPSocket::attach(socketHandle);
        rc = SSL_set_fd(m_ssl, (int) socketHandle);
    }

    if (rc > 0)
        rc = SSL_accept(m_ssl);

    if (rc <= 0) {
        int32_t errorCode = SSL_get_error(m_ssl, rc);
        string error = getSSLError("SSL_accept", errorCode);

        // In non-blocking mode we may have incomplete read or write, so the function call should be repeated
        if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE)
            throw CTimeoutException(error, __FILE__, __LINE__);

        // The serious problem - can't accept, and it's final
        throw CException(error, __FILE__, __LINE__);
    }
}

string CSSLSocket::getSSLError(std::string function, int32_t openSSLError) const
{
    string error("ERROR " + function + ": ");

    switch (openSSLError) {
    case SSL_ERROR_NONE:
        return function + ": Ok";
    case SSL_ERROR_ZERO_RETURN:
        return error + "Connection interrupted";
    case SSL_ERROR_WANT_READ:
        return error + "Incomplete read";
    case SSL_ERROR_WANT_WRITE:
        return error + "Incomplete write";
    case SSL_ERROR_WANT_CONNECT:
    case SSL_ERROR_WANT_X509_LOOKUP:
        return error + "Connect failed";
    case SSL_ERROR_WANT_ACCEPT:
        return error + "Accept failed";
    default:
        openSSLError = ERR_get_error();
        if (!openSSLError)
            return error + "System call or protocol error";
    }

    return error + ERR_func_error_string(openSSLError) + string(": ") + ERR_reason_error_string(openSSLError);
}

uint32_t CSSLSocket::socketBytes()
{
    if (m_ssl) {
        char dummy[8];
        SSL_read(m_ssl, dummy, 0);
        return SSL_pending(m_ssl);
    }
    return 0;
}

size_t CSSLSocket::recv(void* buffer, size_t size) throw (exception)
{
    int rc = SSL_read(m_ssl, buffer, (int) size);
    //if (rc == 0)
    //    throw CException("Connection terminated");
    if (rc < 0)
        throwSSLError(rc);
    return rc;
}

#define WRITE_BLOCK 16384
size_t CSSLSocket::send(const void* buffer, size_t len) throw (exception)
{
    const char* ptr = (const char*) buffer;
    uint32_t    totalLen = (uint32_t)len;
    int         rc;
    for (;;) {
        size_t writeLen = totalLen;
        if (totalLen > WRITE_BLOCK)
            writeLen = WRITE_BLOCK;
        rc = SSL_write(m_ssl, ptr, (int) writeLen);
        if (rc > 0) {
            ptr += rc;
            totalLen -= rc;
            if (!totalLen)
                return len;
            continue;
        }
        int32_t errorCode = SSL_get_error(m_ssl, rc);
        if (errorCode != SSL_ERROR_WANT_READ && errorCode != SSL_ERROR_WANT_WRITE)
            throw CException(getSSLError("writing to SSL connection", errorCode));
        CThread::msleep(10);
    }
}
