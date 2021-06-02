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
#include <openssl/ssl.h>
#include <sptk5/threads/Locks.h>

namespace sptk {

class SP_EXPORT SSLKeys
{
    mutable SharedMutex m_mutex;
    String              m_privateKeyFileName;
    String              m_certificateFileName;
    String              m_password;
    String              m_caFileName;
    int                 m_verifyMode {SSL_VERIFY_NONE};
    int                 m_verifyDepth {0};

    /**
     * Assign keys from another object
     * @param other
     */
    void assign(const SSLKeys& other);

public:

    /**
     * Default constructor
     */
    SSLKeys() = default;

    /**
     * Constructor
     *
     * Private key and certificates must be encoded with PEM format.
     * A single file containing private key and certificate can be used by supplying it for both,
     * private key and certificate parameters.
     * If private key is protected with password, then password can be supplied to auto-answer.
     * @param privateKeyFileName    Private key file name
     * @param certificateFileName   Certificate file name
     * @param password              Key file password
     * @param caFileName            Optional CA (root certificate) file name
     * @param verifyMode            Ether SSL_VERIFY_NONE, or SSL_VERIFY_PEER, for server can be ored with SSL_VERIFY_FAIL_IF_NO_PEER_CERT and/or SSL_VERIFY_CLIENT_ONCE
     * @param verifyDepth           Connection verify depth
     */
    SSLKeys(String privateKeyFileName, String certificateFileName, String password="",
            String caFileName="", int verifyMode=SSL_VERIFY_NONE, int verifyDepth=0);

    /**
     * Copy constructor
     * @param other             The other object
     */
    SSLKeys(const SSLKeys& other);

    /**
     * Copy assignment
     * @param other             The other object
     */
    SSLKeys& operator = (const SSLKeys& other);

    /**
     * Destructor
     */
    ~SSLKeys() noexcept = default;

    /**
     * Unique SSL keys identifier, for SSL keys index
     * @return SSL keys identifier
     */
    String ident() const;

    /**
     * @return private key file name
     */
    String privateKeyFileName() const;

    /**
     * @return certificate file name
     */
    String certificateFileName() const;

    /**
     * @return private key password
     */
    String password() const;

    /**
     * @return certificate authority file name
     */
    String caFileName() const;

    /**
     * @return verify mode
     */
    int verifyMode() const;

    /**
     * @return number of certificates to verify
     */
    int verifyDepth() const;
};

}

