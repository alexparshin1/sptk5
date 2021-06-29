/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
using namespace chrono;

// OpenSSL library initialization
class CSSLLibraryLoader
{
#if OPENSSL_API_COMPAT >= 0x10100000L
    static std::mutex*          m_locks;
#endif

    static CSSLLibraryLoader m_loader;

    static void load_library()
    {
        OpenSSL_add_all_algorithms();
        ERR_load_BIO_strings();
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

    CSSLLibraryLoader(CSSLLibraryLoader&&) noexcept = delete;

    CSSLLibraryLoader& operator=(const CSSLLibraryLoader&) = delete;

    CSSLLibraryLoader& operator=(CSSLLibraryLoader&&) noexcept = delete;

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
mutex*              CSSLLibraryLoader::m_locks;
#endif

CSSLLibraryLoader   CSSLLibraryLoader::m_loader;

void SSLSocket::throwSSLError(const String& function, int rc) const
{
    int errorCode = SSL_get_error(m_ssl, rc);
    string error = getSSLError(function.c_str(), errorCode);
    throw Exception(error, __FILE__, __LINE__);
}

SSLSocket::SSLSocket(const String& cipherList)
    : m_cipherList(cipherList)
{
}

SSLSocket::~SSLSocket()
{
    if (m_ssl != nullptr)
    {
        SSL_free(m_ssl);
    }
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

    if (m_ssl != nullptr)
    {
        SSL_free(m_ssl);
    }

    m_ssl = SSL_new(m_sslContext->handle());

    if (!m_sniHostName.empty())
    {
        if (auto rc = (int) SSL_set_tlsext_host_name(m_ssl, m_sniHostName.c_str());
            !rc)
        {
            throwSSLError("SSL_set_tlsext_host_name", rc);
        }
    }
}

void SSLSocket::_open(const Host& _host, OpenMode openMode, bool _blockingMode, chrono::milliseconds timeout)
{
    initContextAndSocket();

    TCPSocket::_open(_host, openMode, _blockingMode, timeout);
}

void SSLSocket::_open(const struct sockaddr_in& address, OpenMode openMode, bool _blockingMode,
                      chrono::milliseconds timeout)
{
    TCPSocket::_open(address, openMode, _blockingMode, timeout);

    scoped_lock lock(*this);

    openSocketFD(_blockingMode, timeout);
}

bool SSLSocket::tryConnect(const DateTime& timeoutAt)
{
    int rc = SSL_connect(m_ssl);
    if (rc == 1)
    {
        return true;
    } // connected
    if (rc <= 0)
    {
        chrono::milliseconds nextTimeout = chrono::duration_cast<chrono::milliseconds>(timeoutAt - DateTime("now"));
        int errorCode = SSL_get_error(m_ssl, rc);
        if (errorCode == SSL_ERROR_WANT_READ)
        {
            if (!readyToRead(nextTimeout))
            {
                throw Exception("SSL handshake read timeout");
            }
            return false; // continue attempts
        }
        else if (errorCode == SSL_ERROR_WANT_WRITE)
        {
            if (!readyToWrite(nextTimeout))
            {
                throw Exception("SSL handshake write timeout");
            }
            return false; // continue attempts
        }
    }
    throwSSLError("SSL_connect", rc);
}

void SSLSocket::openSocketFD(bool _blockingMode, const chrono::milliseconds& timeout)
{
    DateTime started = DateTime::Now();
    DateTime timeoutAt(started + timeout);

    SSL_set_fd(m_ssl, fd());

    if (timeout == chrono::milliseconds(0))
    {
        if (int rc = SSL_connect(m_ssl);
            rc <= 0)
        {
            close();
            throwSSLError("SSL_connect", rc);
        }
        return;
    }

    blockingMode(false);
    while (!tryConnect(timeoutAt))
    {
        // Repeat operation until connected,
        // or throws an exception
    }
    blockingMode(_blockingMode);
}

void SSLSocket::close() noexcept
{
    SSL_set_fd(m_ssl, -1);
    TCPSocket::close();
}

void SSLSocket::attach(SOCKET socketHandle, bool accept)
{
    scoped_lock lock(*this);

    initContextAndSocket();

    if (fd() != socketHandle)
    {
        TCPSocket::attach(socketHandle, false);
        int rc = SSL_set_fd(m_ssl, socketHandle);
        if (rc <= 0)
        {
            int32_t errorCode = SSL_get_error(m_ssl, rc);
            string error = getSSLError("SSL_accept", errorCode);
            throw ConnectionException(error);
        }
    }

    if (!accept)
    {
        openSocketFD(false, seconds(10));
        return;
    }

    int rc = SSL_accept(m_ssl);
    if (rc <= 0)
    {
        int32_t errorCode = SSL_get_error(m_ssl, rc);
        string error = getSSLError("SSL_accept", errorCode);

        // In non-blocking mode we may have incomplete read or write, so the function call should be repeated
        if (errorCode == SSL_ERROR_WANT_READ || errorCode == SSL_ERROR_WANT_WRITE)
        {
            throw TimeoutException(error, __FILE__, __LINE__);
        }

        // The serious problem - can't accept, and it's final
        throw Exception(error, __FILE__, __LINE__);
    }
}

string SSLSocket::getSSLError(const string& function, int32_t openSSLError) const
{
    string error("ERROR " + function + ": ");
    unsigned long unknownError;

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
            unknownError = ERR_get_error();
            if (unknownError == 0)
            {
                return error + "System call or protocol error";
            }
    }

    return error + ERR_func_error_string(unknownError) + string(": ") + ERR_reason_error_string(unknownError);
}

size_t SSLSocket::socketBytes()
{
    if (reader().availableBytes() > 0)
    {
        return reader().availableBytes();
    }
    if (m_ssl != nullptr)
    {
        array<char, 8> dummy;
        SSL_read(m_ssl, dummy.data(), 0);
        return (uint32_t) SSL_pending(m_ssl);
    }
    return 0;
}

size_t SSLSocket::recv(uint8_t* buffer, size_t len)
{
    int rc;
    for (;;)
    {
        rc = SSL_read(m_ssl, buffer, (int) len);
        if (rc >= 0)
        {
            break;
        }
        int error = SSL_get_error(m_ssl, rc);
        switch (error)
        {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                break;
            default:
                close();
                throwSSLError("SSL_read", rc);
        }
    }
    return (size_t) rc;
}

#define WRITE_BLOCK 16384

size_t SSLSocket::send(const uint8_t* buffer, size_t len)
{
    if (len == 0)
    {
        return 0;
    }
    auto* ptr = (const char*) buffer;
    auto totalLen = (uint32_t) len;
    for (;;)
    {
        size_t writeLen = totalLen;
        if (totalLen > WRITE_BLOCK)
        {
            writeLen = WRITE_BLOCK;
        }
        int rc = SSL_write(m_ssl, ptr, (int) writeLen);
        if (rc > 0)
        {
            ptr += rc;
            totalLen -= rc;
            if (totalLen == 0)
            {
                return len;
            }
            continue;
        }

        if (int32_t errorCode = SSL_get_error(m_ssl, rc);
            errorCode != SSL_ERROR_WANT_READ && errorCode != SSL_ERROR_WANT_WRITE)
        {
            throw Exception(getSSLError("writing to SSL connection", errorCode));
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

#if USE_GTEST

TEST(SPTK_SSLSocket, connect)
{
    SSLKeys keys(String(TEST_DIRECTORY) + "/keys/test.key", String(TEST_DIRECTORY) + "/keys/test.cert");
    SSLSocket sslSocket;

    try
    {
        sslSocket.loadKeys(keys); // Optional step - not required for Google connect
        sslSocket.open(Host("www.google.com:443"));
        sslSocket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

#endif
