/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/sptk.h>

#include <memory>
#include <sptk5/net/SSLContext.h>
#include <sptk5/net/SSLKeys.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Encrypted TCP Socket.
 */
class SP_EXPORT SSLSocket : public TCPSocket
{
public:
    /**
     * @brief Throws SSL error based on SSL function return code.
     * @param function          SSL function name.
     * @param resultCode        SSL function return code.
     * @param location          Location of the error, defaults to the current source location.
     */
    [[noreturn]] void throwSSLError(const String& function, int resultCode, std::source_location location = std::source_location::current()) const;

    /**
     * @brief Constructor.
	 * @param cipherList		Optional cipher list.
     * @param tlsOnly           The TLS only mode.
     */
    explicit SSLSocket(String cipherList = "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4", bool tlsOnly = false);

    /**
     * @brief Destructor.
     */
    ~SSLSocket() override;

    /**
     * @brief Loads private key and certificate(s).
     *
     * Keys should be loaded once before the connection. There is no need to load keys for any consequent connection
     * with the same keys.
     * The private key and certificates must be encoded with PEM format.
     * A single file containing the private key and certificate can be used by supplying it for both,
     * private key and certificate parameters.
     * If the private key is protected with password, then password can be supplied to auto-answer.
     * @param keys                  SSL keys.
     */
    void loadKeys(const SSLKeys& keys);

    /**
     * @brief Set SNI host name.
     * This method only affects the next connection.
     * @param sniHostName           SNI host name.
     */
    [[maybe_unused]] void setSNIHostName(const String& sniHostName);

protected:
    /**
     * @brief Reads and writes share the SSL session here, so they may not overlap.
     *
     * Both directions run through one SSL object, and OpenSSL requires that only one thread
     * touches it at a time. So this socket keeps the original scheme: every I/O call takes
     * the socket mutex exclusively.
     */
    [[nodiscard]] bool fullDuplexIO() const noexcept override
    {
        return false;
    }

    /**
     * @brief Initialize SSL context and socket structures.
     */
    void initContextAndSocket();

    /**
     * @brief Returns the number of bytes available for read.
     */
    size_t getSocketBytesUnlocked() const override;

    /**
     * @brief Reports true if there is data to read, decrypted data included.
     *
     * The base implementation polls the socket, which is the wrong question for an encrypted
     * connection: TLS arrives in records, and reading one hands over its whole plaintext, so the
     * bytes a caller is waiting for are routinely sitting in the SSL buffer with nothing left on
     * the socket to poll for. A request whose body shared a record with its headers - which is
     * what a client that writes them separately produces, and what TLS 1.3 does with the last
     * handshake flight - then waits for data that has already arrived, until some unrelated byte
     * happens to come along and wake the poll.
     *
     * @param timeout           Read timeout.
     */
    [[nodiscard]] bool readyToReadUnlocked(const std::chrono::milliseconds& timeout) override;

    /**
     * @brief Opens the socket connection by host and port.
     *
     * Initializes SSL first, if the host name is empty or port is 0, then the current host and port values are used.
     * They could be defined by previous calls of  open(), port(), or host() methods.
     * @param host const Host&, the host name.
     * @param openMode              Socket open mode.
     * @param blockingMode          Socket blocking (true) on non-blocking (false) mode.
     * @param timeout               Connection timeout. The default is 0 (wait forever).
     * @param clientBindAddress     Client bind address.
     */
    void openUnlocked(const Host& host, OpenMode openMode, bool blockingMode,
                      const std::chrono::milliseconds& timeout, const char* clientBindAddress) override;

    /**
     * @brief Opens the client socket connection by host and port.
     * @param address               Address and port.
     * @param openMode              Socket open mode.
     * @param blockingMode          Socket blocking (true) on non-blocking (false) mode.
     * @param timeout               Connection timeout. The default is 0 (wait forever).
     * @param clientBindAddress     Client bind address.
     */
    void openUnlocked(const struct sockaddr_in& address, OpenMode openMode, bool blockingMode,
                      const std::chrono::milliseconds& timeout, const char* clientBindAddress) override;

    /**
     * @brief Get error description for SSL error code.
     * @param function          SSL function.
     * @param openSSLError          Error code returned by SSL_get_error() result.
     * @return Error description.
     */
    virtual String sslGetErrorString(const String& function, int32_t openSSLError) const;

    /**
     * @brief Attaches the socket handle.
     *
     * This method is designed to only attach socket handles obtained with accept().
     * @param socketHandle          External socket handle.
     * @param accept                If true, then the socket handle should be accepted otherwise it is connected.
     */
    void attachUnlocked(SocketType socketHandle, bool accept) override;

    /**
     * @brief Closes the socket connection.
     *
     * This method is not thread-safe.
     */
    void closeUnlocked() override;

    /**
     * @brief Reads data from SSL socket.
     * @param buffer            Destination buffer.
     * @param size              Destination buffer size.
     * @return the number of bytes read from the socket.
     */
    size_t recvUnlocked(uint8_t* buffer, size_t size) override;

    /**
     * @brief Sends data through SSL socket.
     * @param buffer            Send buffer.
     * @param len               Send data length.
     * @return the number of bytes sent to the socket.
     */
    size_t sendUnlocked(const uint8_t* buffer, size_t len) override;

private:
    /**
     * @brief An SSL operation's return value together with the error code that explains it.
     *
     * SSL_get_error() reports on the last operation performed on the SSL object by this
     * thread, so it is only meaningful when nothing has touched that object in between.
     * Querying it in a separate, separately-locked call cannot promise that: another thread's
     * read or write may land in the gap and the caller then acts on someone else's error.
     * Returning both values from one locked call is what makes the pairing hold.
     */
    struct SslOutcome
    {
        int result; ///< Value returned by the SSL_* call.
        int error;  ///< SSL_get_error() for that value, taken under the same lock.
    };

    mutable std::mutex m_mutex;                ///< Mutex that protects access to m_ssl.
    SharedSSLContext   m_sslContext {nullptr}; ///< SSL context.
    SSL*               m_ssl {nullptr};        ///< SSL socket.
    SSLKeys            m_keys;                 ///< SSL keys info.
    String             m_sniHostName;          ///< SNI host name (optional).
    String             m_cipherList;           ///< Cipher List.
    bool               m_tlsOnly {false};      ///< TLS only mode.

    bool tryConnectUnlocked(const DateTime& timeoutAt);

    void sslConnectUnlocked(bool blockingMode, const std::chrono::milliseconds& timeout);
    void sslNew();
    void sslFree();

    /**
     * @brief Detaches or attaches the descriptor of the SSL object.
     * @param fd                Descriptor to use, or INVALID_SOCKET to detach.
     * @return 1 on success (including when there is no SSL object to touch), 0 on failure.
     */
    int sslSetFd(SocketType fd) const;

    int sslSetExtHostName() const;
    int sslPending() const;
    int sslGetErrorCode(int result) const;

    SslOutcome sslConnect() const;
    SslOutcome sslAccept() const;
    SslOutcome sslRead(uint8_t* buffer, size_t len) const;
    SslOutcome sslWrite(const uint8_t* buffer, size_t len) const;

    /**
     * @brief Returns the SSL object, or throws if the socket was never initialized.
     * @remarks The caller must hold m_mutex.
     */
    SSL* sslHandleLocked() const;
};

/**
 * @}
 */
} // namespace sptk
