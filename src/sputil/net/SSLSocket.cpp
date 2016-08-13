/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SSLSocket.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/sptk.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sptk5/net/SSLSocket.h>
#include <sptk5/threads/Thread.h>

using namespace std;
using namespace sptk;

// OpenSSL library initialization
class CSSLLibraryLoader
{
    static Synchronized*  m_locks;

    void load_library()
    {
        SSL_library_init();
        SSL_load_error_strings();
        ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
    }

    static void lock_callback(int mode, int type, char *file, int line)
    {
        if (mode & CRYPTO_LOCK)
            m_locks[type].lock();
        else
            m_locks[type].unlock();
    }

    static unsigned long thread_id(void)
    {
        unsigned long ret;
#ifdef _WIN32
        ret = GetCurrentThreadId();
#else
        ret=(unsigned long)pthread_self();
#endif
        return(ret);
    }

    static void init_locks(void)
    {
        m_locks = new Synchronized[CRYPTO_num_locks()];
        CRYPTO_set_id_callback(thread_id);
        CRYPTO_set_locking_callback((void (*)(int, int, const char*, int))lock_callback);
    }

    static void kill_locks(void)
    {
        CRYPTO_set_locking_callback(NULL);
        delete [] m_locks;
    }

public:
    CSSLLibraryLoader()
    {
        load_library();
        init_locks();
    }

    ~CSSLLibraryLoader()
    {
        CRYPTO_set_locking_callback(NULL);
        CRYPTO_set_id_callback(NULL);
#if OPENSSL_VERSION_NUMBER > 0x1000114fL
        SSL_COMP_free_compression_methods();
#endif
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
        kill_locks();
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
    }
};

Synchronized* CSSLLibraryLoader::m_locks;

static CSSLLibraryLoader loader;

void SSLSocket::throwSSLError(int rc)
{
    int errorCode = SSL_get_error(m_ssl, rc);
    string error = getSSLError("SSL_connect", errorCode);
    throw Exception(error, __FILE__, __LINE__);
}

SSLSocket::SSLSocket(SSLContext& sslContext)
: m_ssl(SSL_new(sslContext.handle()))
{
}

SSLSocket::~SSLSocket()
{
    SSL_free(m_ssl);
}

void SSLSocket::open(string hostName, uint32_t port, CSocketOpenMode openMode, bool blockingMode, uint32_t timeoutMS) THROWS_EXCEPTIONS
{
    TCPSocket::open(hostName, port, openMode, blockingMode, timeoutMS);

    SYNCHRONIZED_CODE;

    SSL_set_fd(m_ssl, (int) m_sockfd);

    if (blockingMode) {
        int rc = SSL_connect(m_ssl);
        if (rc <= 0) {
            close();
            throwSSLError(rc);
        }
        return;
    } else {
        time_t started = time(NULL);
        for (;;) {
            int rc = SSL_connect(m_ssl);
            if (rc >= 0)
                break;
            int error = SSL_get_error(m_ssl, rc);
            switch(error) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                break;
            default:
                close();
                throwSSLError(rc);
                break;
            }
            time_t now = time(NULL);
            if (now - started > 30)
                throw Exception("SSL handshake timeout");
            Thread::msleep(1);
        }
    }
}

void SSLSocket::close()
{
    SSL_set_fd(m_ssl, -1);
    TCPSocket::close();
}

void SSLSocket::attach(SOCKET socketHandle) throw (std::exception)
{
    SYNCHRONIZED_CODE;

    int rc = 1;
    if (m_sockfd != socketHandle) {
        TCPSocket::attach(socketHandle);
        rc = SSL_set_fd(m_ssl, (int) socketHandle);
    }

    if (rc > 0)
        rc = SSL_accept(m_ssl);

    if (rc <= 0) {
        int32_t errorCode = SSL_get_error(m_ssl, rc);
        string error = getSSLError("SSL_accept", errorCode);

        // In non-blocking mode we may have incomplete read or write, so the function call should be repeated
        if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE)
            throw TimeoutException(error, __FILE__, __LINE__);

        // The serious problem - can't accept, and it's final
        throw Exception(error, __FILE__, __LINE__);
    }
}

string SSLSocket::getSSLError(std::string function, int32_t openSSLError) const
{
    string error("ERROR " + function + ": ");
    unsigned long unknownError;

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
        unknownError = ERR_get_error();
        if (!unknownError)
            return error + "System call or protocol error";
    }

    return error + ERR_func_error_string(unknownError) + string(": ") + ERR_reason_error_string(unknownError);
}

uint32_t SSLSocket::socketBytes()
{
    if (m_ssl) {
        char dummy[8];
        SSL_read(m_ssl, dummy, 0);
        return SSL_pending(m_ssl);
    }
    return 0;
}

size_t SSLSocket::recv(void* buffer, size_t size) throw (exception)
{
    int rc;
    for (;;) {
        rc = SSL_read(m_ssl, buffer, (int) size);
        if (rc >= 0)
            break;
        int error = SSL_get_error(m_ssl, rc);
        switch(error) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            break;
        default:
            close();
            throwSSLError(rc);
            break;
        }
    }
    return rc;
}

#define WRITE_BLOCK 16384
size_t SSLSocket::send(const void* buffer, size_t len) throw (exception)
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
            throw Exception(getSSLError("writing to SSL connection", errorCode));
        Thread::msleep(10);
    }
}
