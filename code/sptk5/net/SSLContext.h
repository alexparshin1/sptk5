/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SSLContext.h - description                             ║
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

#ifndef __SSLCONTEXT_H__
#define __SSLCONTEXT_H__

#include <sptk5/sptk.h>
#include <sptk5/String.h>
#include <openssl/ssl.h>
#include <mutex>
#include <sptk5/threads/Locks.h>

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
    /**
     * SSL connection context
     */
    SSL_CTX*        m_ctx;

    /**
     * Password for auto-answer in callback function
     */
    String          m_password;

    /**
     * Password auto-reply callback function
     */
    static int passwordReplyCallback(char *replyBuffer, int replySize, int rwflag, void *userdata);

    /**
     * Throw SSL error
     * @param humanDescription  Human-readable error description
     */
    void throwError(const String& humanDescription);

public:

    /**
     * Default constructor
     */
    SSLContext();

    /**
     * Destructor
     */
    virtual ~SSLContext();

    /**
     * Loads private key and certificate(s)
     *
     * Private key and certificates must be encoded with PEM format.
     * A single file containing private key and certificate can be used by supplying it for both,
     * private key and certificate parameters.
     * If private key is protected with password, then password can be supplied to auto-answer.
     * @param keyFileName           Private key file name
     * @param certificateFileName   Certificate file name
     * @param password              Key file password
     * @param caFileName            Optional CA (root certificate) file name
     * @param verifyMode            Ether SSL_VERIFY_NONE, or SSL_VERIFY_PEER, for server can be ored with SSL_VERIFY_FAIL_IF_NO_PEER_CERT and/or SSL_VERIFY_CLIENT_ONCE
     * @param verifyDepth           Connection verify depth
     */
    void loadKeys(const String& keyFileName, const String& certificateFileName, const String& password,
                  const String& caFileName = "", int verifyMode = SSL_VERIFY_NONE, int verifyDepth = 0);

    /**
     * Returns SSL context handle
     */
    SSL_CTX* handle();
};

/**
 * @}
 */
}

#endif
