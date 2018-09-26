/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CachedSSLContext.cpp - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday September 26 2018                            ║
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
#ifndef __CACHED_SSLCONTEXT_H__
#define __CACHED_SSLCONTEXT_H__

#include <set>
#include "SSLContext.h"

namespace sptk {

class CachedSSLContext
{
    typedef std::map<String,SSLContext> CachedSSLContextMap;

    static SharedMutex          m_mutex;
    static CachedSSLContextMap  m_contexts;

    /**
     * Makes SSL context ident, used as cache index, from keys definition
     * @param keyFileName           Private key file name
     * @param certificateFileName   Certificate file name
     * @param password              Key file password
     * @param caFileName            Optional CA (root certificate) file name
     * @param verifyMode            Ether SSL_VERIFY_NONE, or SSL_VERIFY_PEER, for server can be ored with SSL_VERIFY_FAIL_IF_NO_PEER_CERT and/or SSL_VERIFY_CLIENT_ONCE
     * @param verifyDepth           Connection verify depth
     */
    static String makeIdent(const String& keyFileName = "", const String& certificateFileName = "",
                            const String& password = "",
                            const String& caFileName = "", int verifyMode = SSL_VERIFY_NONE, int verifyDepth = 0);
public:
    /**
     * @brief Loads private key and certificate(s)
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
    static SSLContext* get(const String& keyFileName="", const String& certificateFileName="", const String& password="",
                           const String& caFileName = "", int verifyMode = SSL_VERIFY_NONE, int verifyDepth = 0);
};

}

#endif
