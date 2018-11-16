/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SSLSocket.cpp - description                            ║
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

#include <sptk5/net/SSLSocket.h>
#include <sptk5/threads/Thread.h>

// These two includes must be after SSLContext.h, or it breaks Windows compilation
#include <openssl/err.h>
#include <sptk5/net/CachedSSLContext.h>

using namespace std;
using namespace sptk;

// OpenSSL library initialization
class CSSLLibraryLoader
{
    static std::mutex*  m_locks;

    void load_library()
    {
        SSL_library_init();
        SSL_load_error_strings();
        ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
    }

    static void lock_callback(int mode, int type, char *file, int line)
    {
        if ((mode & CRYPTO_LOCK) == CRYPTO_LOCK)
            m_locks[type].lock();
        else
            m_locks[type].unlock();
    }

    static unsigned long thread_id()
    {
        unsigned long ret;
#ifdef _WIN32
        ret = GetCurrentThreadId();
#else
        ret=(unsigned long)pthread_self();
#endif
        return(ret);
    }

    static void init_locks()
    {
        m_locks = new mutex[CRYPTO_num_locks()];
        CRYPTO_set_id_callback(thread_id);
        CRYPTO_set_locking_callback((void (*)(int, int, const char*, int))lock_callback);
    }

    static void kill_locks()
    {
        CRYPTO_set_locking_callback(NULL);
        delete [] m_locks;
    }

    CSSLLibraryLoader(const CSSLLibraryLoader& other) = default;
    CSSLLibraryLoader(CSSLLibraryLoader&& other) noexcept {}
    CSSLLibraryLoader& operator = (const CSSLLibraryLoader& other) = default;
    CSSLLibraryLoader& operator = (CSSLLibraryLoader&& other) noexcept
    {
        return *this;
    }

public:
    CSSLLibraryLoader() noexcept
    {
        load_library();
        init_locks();
    }

    ~CSSLLibraryLoader() noexcept
    {
        CRYPTO_set_locking_callback(NULL);
        CRYPTO_set_id_callback(NULL);
#if OPENSSL_VERSION_NUMBER > 0x1000114fL
#if OPENSSL_VERSION_NUMBER > 0x20000000L
        SSL_COMP_free_compression_methods();
#endif
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

mutex* CSSLLibraryLoader::m_locks;

static CSSLLibraryLoader loader;

void SSLSocket::throwSSLError(const String& function, int rc)
{
    int errorCode = SSL_get_error(m_ssl, rc);
    string error = getSSLError(function.c_str(), errorCode);
    throw Exception(error, __FILE__, __LINE__);
}

SSLSocket::SSLSocket()
: m_ssl(nullptr)
{
}

SSLSocket::~SSLSocket()
{
    if (m_ssl != nullptr)
        SSL_free(m_ssl);
}

void SSLSocket::loadKeys(const string& keyFileName, const string& certificateFileName, const string& password,
                         const string& caFileName, int verifyMode, int verifyDepth)
{
    if (m_sockfd > 0)
        throw Exception("Socket already opened");

    m_keyFileName = keyFileName;
    m_certificateFileName = certificateFileName;
    m_password = password;
    m_caFileName = caFileName;
    m_verifyMode = verifyMode;
    m_verifyDepth = verifyDepth;
}

void SSLSocket::setSNIHostName(const String& sniHostName)
{
    m_sniHostName = sniHostName;
}

void SSLSocket::initContextAndSocket()
{
    m_sslContext = CachedSSLContext::get(m_keyFileName, m_certificateFileName, m_password, m_caFileName, m_verifyMode, m_verifyDepth);

    if (m_ssl != nullptr)
        SSL_free(m_ssl);

    m_ssl = SSL_new(m_sslContext->handle());

    if (!m_sniHostName.empty()) {
        int rc = (int) SSL_set_tlsext_host_name(m_ssl, m_sniHostName.c_str());
        if (!rc)
            throwSSLError("SSL_set_tlsext_host_name", rc);
    }
}

void SSLSocket::_open(const Host& host, CSocketOpenMode openMode, bool _blockingMode, chrono::milliseconds timeout)
{
    initContextAndSocket();

    if (!host.hostname().empty())
        m_host = host;
    if (m_host.hostname().empty())
        throw Exception("Please, define the host name", __FILE__, __LINE__);

    sockaddr_in addr = {};
    host.getAddress(addr);

    _open(addr, openMode, _blockingMode, timeout);
}

void SSLSocket::_open(const struct sockaddr_in& address, CSocketOpenMode openMode, bool _blockingMode, chrono::milliseconds timeout)
{
    DateTime started = DateTime::Now();
    DateTime timeoutAt(started + timeout);
    TCPSocket::_open(address, openMode, _blockingMode, timeout);

    lock_guard<mutex> lock(*this);

    SSL_set_fd(m_ssl, (int) m_sockfd);
/*
    int rc = SSL_set1_host(m_ssl, m_host.hostname().c_str());
    if (rc != 1)
        throwSSLError("SSL_set1_host", rc);
*/
    if (timeout == chrono::milliseconds(0)) {
        int rc = SSL_connect(m_ssl);
        if (rc <= 0) {
            close();
            throwSSLError("SSL_connect", rc);
        }
        return;
    }

    blockingMode(false);
    while (true) {
        int rc = SSL_connect(m_ssl);
        if (rc == 1)
            break;
        if (rc <= 0) {
            int errorCode = SSL_get_error(m_ssl, rc);
            if (errorCode == SSL_ERROR_WANT_READ) {
                if (!readyToRead(timeoutAt))
                    throw Exception("SSL handshake read timeout");
                continue;
            }
            else if (errorCode == SSL_ERROR_WANT_WRITE) {
                if (!readyToWrite(timeoutAt))
                    throw Exception("SSL handshake write timeout");
                continue;
            }
        }
        throwSSLError("SSL_connect", rc);
    }
    blockingMode(_blockingMode);
}

void SSLSocket::close() noexcept
{
    SSL_set_fd(m_ssl, -1);
    TCPSocket::close();
}

void SSLSocket::attach(SOCKET socketHandle)
{
    lock_guard<mutex> lock(*this);

    initContextAndSocket();

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

string SSLSocket::getSSLError(const string& function, int32_t openSSLError) const
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
        if (unknownError == 0)
            return error + "System call or protocol error";
    }

    return error + ERR_func_error_string(unknownError) + string(": ") + ERR_reason_error_string(unknownError);
}

size_t SSLSocket::socketBytes()
{
    if (m_reader.availableBytes() > 0)
        return m_reader.availableBytes();
    if (m_ssl != nullptr) {
        char dummy[8];
        SSL_read(m_ssl, dummy, 0);
        return (uint32_t) SSL_pending(m_ssl);
    }
    return 0;
}

size_t SSLSocket::recv(void* buffer, size_t size)
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
            throwSSLError("SSL_read", rc);
            break;
        }
    }
    return (size_t) rc;
}
#define WRITE_BLOCK 16384

size_t SSLSocket::send(const void* buffer, size_t len)
{
    if (len == 0)
        return 0;
    auto* ptr = (const char*) buffer;
    auto totalLen = (uint32_t) len;
    for (;;) {
        size_t writeLen = totalLen;
        if (totalLen > WRITE_BLOCK)
            writeLen = WRITE_BLOCK;
        int rc = SSL_write(m_ssl, ptr, (int) writeLen);
        if (rc > 0) {
            ptr += rc;
            totalLen -= rc;
            if (totalLen == 0)
                return len;
            continue;
        }
        int32_t errorCode = SSL_get_error(m_ssl, rc);
        if (errorCode != SSL_ERROR_WANT_READ && errorCode != SSL_ERROR_WANT_WRITE)
            throw Exception(getSSLError("writing to SSL connection", errorCode));
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}
