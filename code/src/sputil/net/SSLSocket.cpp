/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-03                                             ║
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

// OpenSSL library initialization
class CSSLLibraryLoader
{
#if OPENSSL_API_LEVEL < 20000
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

#if OPENSSL_API_LEVEL < 20000
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
#if OPENSSL_API_LEVEL < 20000
        init_locks();
#endif
    }

    CSSLLibraryLoader(const CSSLLibraryLoader&) = delete;
    CSSLLibraryLoader& operator=(const CSSLLibraryLoader&) = delete;

    ~CSSLLibraryLoader() noexcept
    {
#if OPENSSL_API_LEVEL < 20000
        CRYPTO_set_locking_callback(NULL);
        CRYPTO_set_id_callback(NULL);
        kill_locks();
#endif
#if OPENSSL_API_LEVEL > 20000
        SSL_COMP_free_compression_methods();
#endif
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
    }
};

#if OPENSSL_API_LEVEL < 20000
mutex* CSSLLibraryLoader::m_locks;
#endif

[[maybe_unused]] CSSLLibraryLoader CSSLLibraryLoader::m_loader;

void SSLSocket::throwSSLError(const String& function, const int resultCode, const source_location location) const
{
    const auto errorCode = sslGetErrorCode(resultCode);
    const auto error = sslGetErrorString(function.c_str(), errorCode);
    throw Exception(error, location);
}

SSLSocket::SSLSocket(String cipherList, const bool tlsOnly)
    : m_cipherList(std::move(cipherList))
    , m_tlsOnly(tlsOnly)
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

[[maybe_unused]] void SSLSocket::setSNIHostName(const String& sniHostName)
{
    m_sniHostName = sniHostName;
}

void SSLSocket::initContextAndSocket()
{
    m_sslContext = CachedSSLContext::get(m_keys, m_cipherList, m_tlsOnly);

    sslNew();

    if (!m_sniHostName.empty())
    {
        if (const auto result = sslSetExtHostName();
            !result)
        {
            throwSSLError("SSL_set_tlsext_host_name", result);
        }
    }
}

void SSLSocket::openUnlocked(const Host& _host, const OpenMode openMode, const bool _blockingMode,
                             const milliseconds& timeout, const char* clientBindAddress)
{
    initContextAndSocket();

    TCPSocket::openUnlocked(_host, openMode, _blockingMode, timeout, clientBindAddress);
}

void SSLSocket::openUnlocked(const sockaddr_in& address, const OpenMode openMode, const bool _blockingMode,
                             const milliseconds& timeout, const char* clientBindAddress)
{
    initContextAndSocket();

    TCPSocket::openUnlocked(address, openMode, _blockingMode, timeout, clientBindAddress);

    sslConnectUnlocked(_blockingMode, timeout);
}

bool SSLSocket::tryConnectUnlocked(const DateTime& timeoutAt)
{
    const auto [result, errorCode] = sslConnect();
    if (result == 1)
    {
        return true;
    } // connected

    if (result <= 0)
    {
        auto nextTimeout = chrono::duration_cast<milliseconds>(timeoutAt - DateTime("now"));
        if (nextTimeout.count() <= 0)
        {
            nextTimeout = 0ms;
        }
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

    throw Exception(sslGetErrorString("SSL_connect", errorCode));
}

void SSLSocket::sslConnectUnlocked(const bool _blockingMode, const milliseconds& timeout)
{
    const DateTime started = DateTime::Now();
    const DateTime timeoutAt(started + timeout);

    sslSetFd(getSocketFdUnlocked());

    if (timeout == 0ms)
    {
        if (const auto [result, errorCode] = sslConnect();
            result <= 0)
        {
            // Render before closing - closeUnlocked() detaches the descriptor from the session.
            const auto error = sslGetErrorString("SSL_connect", errorCode);
            closeUnlocked();
            throw Exception(error);
        }
        return;
    }

    setBlockingModeUnlocked(false);

    try
    {
        while (!tryConnectUnlocked(timeoutAt))
        {
            // Repeat the operation until connected or throws an exception
        }
    }
    catch (...)
    {
        setBlockingModeUnlocked(_blockingMode);
        closeUnlocked();
        throw;
    }

    setBlockingModeUnlocked(_blockingMode);
}

void SSLSocket::closeUnlocked()
{
    sslSetFd(INVALID_SOCKET);
    TCPSocket::closeUnlocked();
}

void SSLSocket::attachUnlocked(const SocketType socketHandle, const bool accept)
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

    while (true)
    {
        if (const auto [result, errorCode] = sslAccept();
            result <= 0)
        {
            const auto error = sslGetErrorString("SSL_accept", errorCode);

            // In non-blocking mode we may have incomplete read the function call should be repeated
            if (errorCode == SSL_ERROR_WANT_READ)
            {
                if (!readyToReadUnlocked(30s))
                {
                    throw Exception("SSL accept read timeout");
                }
                continue;
            }

            // In non-blocking mode we may have incomplete write the function call should be repeated
            if (errorCode == SSL_ERROR_WANT_WRITE)
            {
                if (!readyToWriteUnlocked(30s))
                {
                    throw Exception("SSL accept read timeout");
                }
                continue;
            }

            // The serious problem - can't accept, and it's final
            throw Exception(error);
        }
        break;
    }
}

String SSLSocket::sslGetErrorString(const String& function, const int32_t openSSLError) const
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
    return static_cast<uint32_t>(sslPending());
}

bool SSLSocket::readyToReadUnlocked(const chrono::milliseconds& timeout)
{
    // Decrypted and waiting is still waiting. Polling the socket for it would find nothing: the
    // record it arrived in has been read already, and its plaintext is held by OpenSSL.
    if (sslPending() > 0)
    {
        return true;
    }

    // Nothing decrypted yet, but a record may be sitting on the socket unread. Asking for the
    // available byte count decrypts it, and then there is an answer to give without waiting.
    if (Socket::getSocketBytesUnlocked() > 0)
    {
        uint8_t dummy = 0;
        sslRead(&dummy, 0);
        if (sslPending() > 0)
        {
            return true;
        }
    }

    return SocketVirtualMethods::readyToReadUnlocked(timeout);
}

size_t SSLSocket::recvUnlocked(uint8_t* buffer, const size_t len)
{
    if (len == 0)
    {
        return 0;
    }

    for (;;)
    {
        const auto [result, errorCode] = sslRead(buffer, len);
        if (result >= 0)
        {
            return result;
        }

        switch (errorCode)
        {
            case SSL_ERROR_WANT_READ:
                // No data available yet
                if (!readyToReadUnlocked(30s))
                {
                    throw Exception("SSL read timeout");
                }
                break;
            case SSL_ERROR_WANT_WRITE:
                // The socket is busy
                if (!readyToWriteUnlocked(30s))
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
            {
                // Render the error before closing: closeUnlocked() detaches the descriptor from
                // the SSL object, and the description would then be taken from a session that
                // no longer reflects the failure.
                const auto error = sslGetErrorString("SSL_read", errorCode);
                // closeUnlocked(), not close(): the caller (Socket::read()) already holds the
                // socket's exclusive lock, and ReadWriteMutex is not recursive.
                closeUnlocked();
                throw Exception(error);
            }
        }
    }
}

static constexpr auto WRITE_BLOCK = 16384;

size_t SSLSocket::sendUnlocked(const uint8_t* buffer, const size_t len)
{
    if (len == 0)
    {
        return 0;
    }

    const auto* ptr = buffer;
    auto        totalLen = static_cast<uint32_t>(len);
    for (;;)
    {
        size_t writeLen = totalLen;
        if (totalLen > WRITE_BLOCK)
        {
            writeLen = WRITE_BLOCK;
        }

        const auto [result, errorCode] = sslWrite(ptr, writeLen);
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

        constexpr auto timeout = seconds(1);

        switch (errorCode)
        {
            case SSL_ERROR_WANT_READ:
                if (!readyToReadUnlocked(milliseconds(timeout)))
                {
                    throw Exception("SSL write timeout (wait for read)");
                }
                break;
            case SSL_ERROR_WANT_WRITE:
                if (!readyToWriteUnlocked(milliseconds(timeout)))
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
                // activeUnlocked(), not active(): the caller (Socket::write()) already holds the
                // socket's exclusive lock, and ReadWriteMutex is not recursive.
                if (!activeUnlocked())
                {
                    throw Exception("Error writing to SSL connection: Socket is closed");
                }
                throw Exception(sslGetErrorString(format("writing to SSL connection fd={}", getSocketFdUnlocked()), errorCode));
        }
    }
}

SSL* SSLSocket::sslHandleLocked() const
{
    if (m_ssl == nullptr)
    {
        throw Exception("SSL connection is not initialized");
    }
    return m_ssl;
}

void SSLSocket::sslNew()
{
    if (!m_sslContext)
    {
        throw Exception("Can't create SSL connection: no SSL context");
    }

    const scoped_lock lock(m_mutex);
    if (m_ssl != nullptr)
    {
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }
    m_ssl = SSL_new(m_sslContext->handle());
    if (m_ssl == nullptr)
    {
        throw Exception("Can't create SSL connection: " + String(ERR_error_string(ERR_get_error(), nullptr)));
    }
}

void SSLSocket::sslFree()
{
    const scoped_lock lock(m_mutex);
    if (m_ssl != nullptr)
    {
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }
}

int SSLSocket::sslSetFd(const SocketType fd) const
{
    const scoped_lock lock(m_mutex);
    if (m_ssl == nullptr)
    {
        // closeUnlocked() runs this on sockets that were never opened - there is no SSL object
        // to detach the descriptor from, and SSL_set_fd(nullptr, ...) would dereference it.
        return 1;
    }
    return SSL_set_fd(m_ssl, static_cast<int>(fd));
}

int SSLSocket::sslSetExtHostName() const
{
    const scoped_lock lock(m_mutex);
    return SSL_set_tlsext_host_name(sslHandleLocked(), m_sniHostName.c_str());
}

int SSLSocket::sslPending() const
{
    const scoped_lock lock(m_mutex);
    return SSL_pending(sslHandleLocked());
}

int SSLSocket::sslGetErrorCode(const int result) const
{
    const scoped_lock lock(m_mutex);
    if (m_ssl == nullptr)
    {
        return SSL_ERROR_SSL;
    }
    return SSL_get_error(m_ssl, result);
}

SSLSocket::SslOutcome SSLSocket::sslConnect() const
{
    const scoped_lock lock(m_mutex);
    auto*             ssl = sslHandleLocked();
    const auto        result = SSL_connect(ssl);
    return {result, SSL_get_error(ssl, result)};
}

SSLSocket::SslOutcome SSLSocket::sslAccept() const
{
    const scoped_lock lock(m_mutex);
    auto*             ssl = sslHandleLocked();
    const auto        result = SSL_accept(ssl);
    return {result, SSL_get_error(ssl, result)};
}

SSLSocket::SslOutcome SSLSocket::sslRead(uint8_t* buffer, const size_t len) const
{
    const scoped_lock lock(m_mutex);
    auto*             ssl = sslHandleLocked();
    const auto        result = SSL_read(ssl, buffer, static_cast<int>(len));
    return {result, SSL_get_error(ssl, result)};
}

SSLSocket::SslOutcome SSLSocket::sslWrite(const uint8_t* buffer, const size_t len) const
{
    const scoped_lock lock(m_mutex);
    auto*             ssl = sslHandleLocked();
    const auto        result = SSL_write(ssl, buffer, static_cast<int>(len));
    return {result, SSL_get_error(ssl, result)};
}
