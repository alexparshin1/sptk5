/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

// These two #includes must be after SSLContext.h, or it breaks Windows compilation
#include <openssl/err.h>
#include <sptk5/net/CachedSSLContext.h>

#include "sptk5/SystemException.h"
#include <utility>

using namespace std;
using namespace sptk;
using namespace chrono;

#ifndef _WIN32
#define SOCKET_CAST
#else
#define SOCKET_CAST (int)
#endif

// OpenSSL library initialization
class CSSLLibraryLoader
{
#if OPENSSL_API_COMPAT >= 0x10100000L
    static std::mutex* m_locks;
#endif

    [[maybe_unused]] static CSSLLibraryLoader m_loader;

    static void load_library()
    {
        OpenSSL_add_all_algorithms();
#if OPENSSL_API_LEVEL < 20000
        ERR_load_BIO_strings();
#endif
        ERR_load_crypto_strings();
        SSL_load_error_strings();

        SSL_library_init();
    }

#if OPENSSL_API_COMPAT >= 0x10100000L
    static void lock_callback(int mode, int type, const char* /*file*/, int /*line*/)
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
        ret = pthread_self();
#endif
        return (ret);
    }

    static void init_locks()
    {
        m_locks = new mutex[CRYPTO_num_locks()];
        CRYPTO_set_id_callback(thread_id);
        CRYPTO_set_locking_callback((void (*)(int, int, const char*, int)) lock_callback);
    }

    static void kill_locks()
    {
        CRYPTO_set_locking_callback(NULL);
        delete[] m_locks;
    }
#endif

public:
    CSSLLibraryLoader() noexcept
    {
        load_library();
#if OPENSSL_API_COMPAT >= 0x10100000L
        init_locks();
#endif
    }

    CSSLLibraryLoader(const CSSLLibraryLoader&) = delete;
    CSSLLibraryLoader& operator=(const CSSLLibraryLoader&) = delete;

    ~CSSLLibraryLoader() noexcept
    {
#if OPENSSL_API_COMPAT >= 0x10100000L
        CRYPTO_set_locking_callback(NULL);
        CRYPTO_set_id_callback(NULL);
#endif
#if OPENSSL_VERSION_NUMBER > 0x1000114fL
#if OPENSSL_VERSION_NUMBER > 0x20000000L
        SSL_COMP_free_compression_methods();
#endif
#endif
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
#if OPENSSL_API_COMPAT >= 0x10100000L
        kill_locks();
#endif
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
    }
};

#if OPENSSL_API_COMPAT >= 0x10100000L
mutex* CSSLLibraryLoader::m_locks;
#endif

[[maybe_unused]] CSSLLibraryLoader CSSLLibraryLoader::m_loader;

void SSLSocket::throwSSLError(const String& function, int resultCode) const
{
    const int errorCode = sslGetErrorCode(resultCode);
    auto error = sslGetErrorString(function.c_str(), errorCode);
    throw Exception(error);
}

SSLSocket::SSLSocket(String cipherList)
    : m_cipherList(std::move(cipherList))
{
}

SSLSocket::~SSLSocket()
{
    sslFree();
}

void SSLSocket::loadKeys(const SSLKeys& keys)
{
    if (fd() != INVALID_SOCKET)
    {
        throw Exception("Can't set keys on opened socket");
    }
    m_keys = keys;
}

void SSLSocket::setSNIHostName(const String& sniHostName)
{
    m_sniHostName = sniHostName;
}

void SSLSocket::initContextAndSocket()
{
    m_sslContext = CachedSSLContext::get(m_keys, m_cipherList);

    sslNew();

    if (!m_sniHostName.empty())
    {
        sslSetExtHostName();
    }
}

void SSLSocket::openUnlocked(const Host& _host, OpenMode openMode, bool _blockingMode, chrono::milliseconds timeout)
{
    initContextAndSocket();

    TCPSocket::openUnlocked(_host, openMode, _blockingMode, timeout);
}

void SSLSocket::openUnlocked(const struct sockaddr_in& address, OpenMode openMode, bool _blockingMode,
                             chrono::milliseconds timeout)
{
    TCPSocket::openUnlocked(address, openMode, _blockingMode, timeout);

    sslConnectUnlocked(_blockingMode, timeout);
}

bool SSLSocket::tryConnectUnlocked(const DateTime& timeoutAt)
{
    int result = sslConnect();
    if (result == 1)
    {
        return true;
    } // connected

    if (result <= 0)
    {
        const chrono::milliseconds nextTimeout = chrono::duration_cast<chrono::milliseconds>(timeoutAt - DateTime("now"));
        int errorCode = sslGetErrorCode(result);
        if (errorCode == SSL_ERROR_WANT_READ)
        {
            if (!readyToReadUnlocked(nextTimeout))
            {
                throw Exception("SSL handshake read timeout");
            }
            return false; // continue attempts
        }

        if (errorCode == SSL_ERROR_WANT_WRITE)
        {
            if (!readyToWriteUnlocked(nextTimeout))
            {
                throw Exception("SSL handshake write timeout");
            }
            return false; // continue attempts
        }
    }
    throwSSLError("SSL_connect", result);
}

void SSLSocket::sslConnectUnlocked(bool _blockingMode, const milliseconds& timeout)
{
    const DateTime started = DateTime::Now();
    const DateTime timeoutAt(started + timeout);

    sslSetFd(getSocketFdUnlocked());

    if (timeout == chrono::milliseconds(0))
    {
        if (const int result = sslConnect();
            result <= 0)
        {
            closeUnlocked();
            throwSSLError("SSL_connect", result);
        }
        return;
    }

    setBlockingModeUnlocked(false);
    while (!tryConnectUnlocked(timeoutAt))
    {
        // Repeat operation until connected,
        // or throws an exception
    }
    setBlockingModeUnlocked(_blockingMode);
}

void SSLSocket::closeUnlocked()
{
    sslSetFd(-1);
    TCPSocket::closeUnlocked();
}

void SSLSocket::attachUnlocked(SocketType socketHandle, bool accept)
{
    initContextAndSocket();

    if (getSocketFdUnlocked() != socketHandle)
    {
        TCPSocket::attachUnlocked(socketHandle, false);

        if (const auto result = sslSetFd(socketHandle);
            result <= 0)
        {
            const auto errorCode = sslGetErrorCode(result);
            const auto error = sslGetErrorString("SSL_accept", errorCode);
            throw ConnectionException(error);
        }
    }

    if (!accept)
    {
        constexpr seconds connectionTimeout {10};
        sslConnectUnlocked(false, connectionTimeout);
        return;
    }

    if (const int result = sslAccept();
        result <= 0)
    {
        const auto errorCode = sslGetErrorCode(result);
        const auto error = sslGetErrorString("SSL_accept", errorCode);

        // In non-blocking mode we may have incomplete read or write, so the function call should be repeated
        if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE)
        {
            throw TimeoutException(error);
        }

        // The serious problem - can't accept, and it's final
        throw Exception(error);
    }
}

String SSLSocket::sslGetErrorString(const String& function, int32_t openSSLError) const
{
    const String error("ERROR " + function + ": ");

    switch (openSSLError)
    {
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
            break;
    }

    const auto unknownError = ERR_get_error();
    if (unknownError == 0)
    {
        return error + "System call or protocol error";
    }

    return error + ERR_error_string(unknownError, nullptr) + string(": ") + ERR_reason_error_string(unknownError);
}

size_t SSLSocket::getSocketBytesUnlocked() const
{
    if (Socket::getSocketBytesUnlocked() > 0)
    {
        uint8_t dummy = 0;
        sslRead(&dummy, 0);
    }
    return (uint32_t) sslPending();
}

size_t SSLSocket::recvUnlocked(uint8_t* buffer, size_t len)
{
    static const chrono::seconds readTimeout(30);

    for (;;)
    {
        const auto result = sslRead(buffer, len);
        if (result >= 0)
        {
            return result;
        }

        const int error = sslGetErrorCode(result);
        switch (error)
        {
            case SSL_ERROR_WANT_READ:
                // No data available yet
                if (!readyToReadUnlocked(readTimeout))
                {
                    throw Exception("SSL read timeout");
                }
                break;
            case SSL_ERROR_WANT_WRITE:
                // The socket is busy
                if (!readyToWriteUnlocked(readTimeout))
                {
                    throw Exception("SSL write timeout");
                }
                break;
            case SSL_ERROR_NONE:
                // No error, just retry
                break;
            case SSL_ERROR_ZERO_RETURN:
                // peer disconnected
                return 0;
            default:
                close();
                throwSSLError("SSL_read", result);
        }
    }
}

static constexpr int WRITE_BLOCK = 16384;

size_t SSLSocket::sendUnlocked(const uint8_t* buffer, size_t len)
{
    if (len == 0)
    {
        return 0;
    }

    const auto* ptr = buffer;
    auto totalLen = (uint32_t) len;
    for (;;)
    {
        size_t writeLen = totalLen;
        if (totalLen > WRITE_BLOCK)
        {
            writeLen = WRITE_BLOCK;
        }

        const int result = sslWrite(ptr, writeLen);
        if (result > 0)
        {
            ptr += result;
            totalLen -= result;
            if (totalLen == 0)
            {
                return len;
            }
            continue;
        }

        constexpr auto timeout = chrono::seconds(1);

        switch (auto errorCode = sslGetErrorCode(result))
        {
            case SSL_ERROR_WANT_READ:
                if (!readyToReadUnlocked(chrono::milliseconds(timeout)))
                {
                    throw Exception("SSL write timeout (wait for read)");
                }
                break;
            case SSL_ERROR_WANT_WRITE:
                if (!readyToWriteUnlocked(chrono::milliseconds(timeout)))
                {
                    throw Exception("SSL write timeout (wait for write)");
                }
                break;
            case SSL_ERROR_NONE:
                // No error, just retry
                break;
            case SSL_ERROR_ZERO_RETURN:
                // peer disconnected
                return 0;
            case SSL_ERROR_SYSCALL:
                throw SystemException("Error writing to SSL connection");
            default:
                if (!active())
                {
                    throw Exception("Error writing to SSL connection: Socket is closed");
                }
                throw Exception(sslGetErrorString("writing to SSL connection fd=" + to_string(getSocketFdUnlocked()), errorCode));
        }
    }
}

void SSLSocket::sslNew()
{
    scoped_lock lock(m_mutex);
    if (m_ssl != nullptr)
    {
        SSL_free(m_ssl);
    }
    m_ssl = SSL_new(m_sslContext->handle());
}

void SSLSocket::sslFree() const
{
    const scoped_lock lock(m_mutex);
    if (m_ssl != nullptr)
    {
        SSL_free(m_ssl);
    }
}

int SSLSocket::sslSetFd(SocketType fd) const
{
    scoped_lock lock(m_mutex);
    return SSL_set_fd(m_ssl, fd);
}

void SSLSocket::sslSetExtHostName() const
{
    scoped_lock lock(m_mutex);
    if (auto result = (int) SSL_set_tlsext_host_name(m_ssl, m_sniHostName.c_str());
        !result)
    {
        throwSSLError("SSL_set_tlsext_host_name", result);
    }
}

int SSLSocket::sslConnect() const
{
    scoped_lock lock(m_mutex);
    return SSL_connect(m_ssl);
}

int SSLSocket::sslAccept() const
{
    scoped_lock lock(m_mutex);
    return SSL_accept(m_ssl);
}

int SSLSocket::sslRead(uint8_t* buffer, size_t len) const
{
    scoped_lock lock(m_mutex);
    return SSL_read(m_ssl, buffer, (int) len);
}

int SSLSocket::sslWrite(const uint8_t* buffer, size_t len) const
{
    scoped_lock lock(m_mutex);
    return SSL_write(m_ssl, buffer, (int) len);
}

int SSLSocket::sslPending() const
{
    scoped_lock lock(m_mutex);
    return SSL_pending(m_ssl);
}

int SSLSocket::sslGetErrorCode(int result) const
{
    scoped_lock lock(m_mutex);
    return SSL_get_error(m_ssl, result);
}
