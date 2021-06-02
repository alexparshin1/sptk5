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

#pragma once

#include <sptk5/sptk.h>
#include <sptk5/net/SSLContext.h>
#include <sptk5/net/TCPSocket.h>
#include <memory>
#include <sptk5/net/SSLKeys.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Encrypted TCP Socket
 */
class SP_EXPORT SSLSocket: public TCPSocket, public std::mutex
{
public:
    /**
     * Returns number of bytes available for read
     */
    size_t socketBytes() override;

    /**
     * Throws SSL error based on SSL function return code
     * @param function          SSL function name
     * @param rc                SSL function return code
     */
    [[noreturn]] void throwSSLError(const String& function, int rc) const;

    /**
     * Constructor
	 * @param cipherList		Optional cipher list
     */
    explicit SSLSocket(const String& cipherList="ALL");

    /**
     * Destructor
     */
    ~SSLSocket() override;

    /**
     * Loads private key and certificate(s)
     *
     * Key should be loaded once before the connection. There is no need to load keys for any consequent connection
     * with the same keys.
     * Private key and certificates must be encoded with PEM format.
     * A single file containing private key and certificate can be used by supplying it for both,
     * private key and certificate parameters.
     * If private key is protected with password, then password can be supplied to auto-answer.
     * @param keys                  SSL keys
     */
    void loadKeys(const SSLKeys& keys);

    /**
     * Set SNI host name.
     * This method only affects next connection.
     * @param sniHostName           SNI host name
     */
    void setSNIHostName(const String& sniHostName);

    /**
     * Attaches socket handle
     *
     * This method is designed to only attach socket handles obtained with accept().
     * @param socketHandle          External socket handle.
     */
    void attach(SOCKET socketHandle, bool accept) override;

    /**
     * Closes the socket connection
     *
     * This method is not thread-safe.
     */
    void close() noexcept override;

    /**
     * Returns SSL handle
     */
    SSL* handle()
    {
        return m_ssl;
    }

protected:

    /**
     * Initialize SSL context and socket structures
     */
    void initContextAndSocket();

    /**
     * opens the socket connection by host and port
     *
     * Initializes SSL first, if host name is empty or port is 0 then the current host and port values are used.
     * They could be defined by previous calls of  open(), port(), or host() methods.
     * @param host const Host&, the host name
     * @param openMode              Socket open mode
     * @param blockingMode          Socket blocking (true) on non-blocking (false) mode
     * @param timeout               Connection timeout. The default is 0 (wait forever)
     */
    void _open(const Host& host, CSocketOpenMode openMode, bool blockingMode, std::chrono::milliseconds timeout) override;

    /**
     * Opens the client socket connection by host and port
     * @param address               Address and port
     * @param openMode              Socket open mode
     * @param blockingMode          Socket blocking (true) on non-blocking (false) mode
     * @param timeout               Connection timeout. The default is 0 (wait forever)
     */
    void _open(const struct sockaddr_in& address, CSocketOpenMode openMode, bool blockingMode, std::chrono::milliseconds timeout) override;

    /**
     * Reads data from SSL socket
     * @param buffer            Destination buffer
     * @param size              Destination buffer size
     * @return the number of bytes read from the socket
     */
    size_t recv(void* buffer, size_t size) override;

    /**
     * Sends data through SSL socket
     * @param buffer            Send buffer
     * @param len               Send data length
     * @return the number of bytes sent the socket
     */

    size_t send(const void* buffer, size_t len) override;

    /**
     * Get error description for SSL error code
     * @param function          SSL function
     * @param SSLError          Error code returned by SSL_get_error() result
     * @return Error description
     */
    virtual std::string getSSLError(const std::string& function, int32_t SSLError) const;

private:

    SharedSSLContext	m_sslContext {nullptr};     ///< SSL context
    SSL*				m_ssl {nullptr};            ///< SSL socket
    SSLKeys				m_keys;                     ///< SSL keys info

    String				m_sniHostName;              ///< SNI host name (optional)
    String				m_cipherList;				///< Cipher List, the default is "ALL"

    void openSocketFD(bool blockingMode, const std::chrono::milliseconds& timeout);

    bool tryConnect(const DateTime& timeoutAt);
};

/**
 * @}
 */
}

