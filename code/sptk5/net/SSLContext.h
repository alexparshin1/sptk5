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
#include <sptk5/String.h>
#include <sptk5/threads/Locks.h>
#include <sptk5/net/SSLKeys.h>
#include <openssl/ssl.h>
#include <mutex>
#include <memory>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * SSL connection context
 */
class SSLContext : public SharedMutex
{
    std::shared_ptr<SSL_CTX>    m_ctx;                          ///< SSL connection context
    String                      m_password;                     ///< Password for auto-answer in callback function
    static int                  s_server_session_id_context;

    /**
     * Password auto-reply callback function
     */
    static int passwordReplyCallback(char *replyBuffer, int replySize, int rwflag, void *userdata);

    /**
     * Throw SSL error
     * @param humanDescription  Human-readable error description
     */
    [[noreturn]] static void throwError(const String& humanDescription);

public:

    /**
     * Constructor
	 * @param cipherList		Cipher list. Use "ALL" if not known.
     */
    explicit SSLContext(const String& cipherList);

    /**
     * Loads private key and certificate(s)
     *
     * Private key and certificates must be encoded with PEM format.
     * A single file containing private key and certificate can be used by supplying it for both,
     * private key and certificate parameters.
     * If private key is protected with password, then password can be supplied to auto-answer.
     * @param keys                  Keys and certificates
     */
    void loadKeys(const SSLKeys& keys);

    /**
     * Returns SSL context handle
     */
    SSL_CTX* handle();
};

using SharedSSLContext = std::shared_ptr<SSLContext>;

/**
 * @}
 */
}
