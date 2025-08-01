/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SSLKeys.h - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Friday Feb 8 2019                                      ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SSL_KEYS_H__
#define __SSL_KEYS_H__

#include <sptk5/sptk.h>
#include <sptk5/String.h>
#include <openssl/ssl.h>
#include <sptk5/threads/Locks.h>

namespace sptk {

class SSLKeys
{
    mutable SharedMutex m_mutex;
    String              m_privateKeyFileName;
    String              m_certificateFileName;
    String              m_password;
    String              m_caFileName;
    int                 m_verifyMode {SSL_VERIFY_NONE};
    int                 m_verifyDepth {0};

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
     * @param keyFileName           Private key file name
     * @param certificateFileName   Certificate file name
     * @param password              Key file password
     * @param caFileName            Optional CA (root certificate) file name
     * @param verifyMode            Ether SSL_VERIFY_NONE, or SSL_VERIFY_PEER, for server can be ored with SSL_VERIFY_FAIL_IF_NO_PEER_CERT and/or SSL_VERIFY_CLIENT_ONCE
     * @param verifyDepth           Connection verify depth
     */
    SSLKeys(const String& privateKeyFileName, const String& certificateFileName, const String& password="",
            const String& caFileName="", int verifyMode=SSL_VERIFY_NONE, int verifyDepth=0);
    SSLKeys(const SSLKeys& other);

    SSLKeys& operator = (const SSLKeys& other);

    String ident() const;

    String privateKeyFileName() const;

    String certificateFileName() const;

    String password() const;

    String caFileName() const;

    int verifyMode() const;

    int verifyDepth() const;

};

}

#endif
