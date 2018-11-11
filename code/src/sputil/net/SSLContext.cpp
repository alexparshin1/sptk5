/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SSLContext.cpp - description                           ║
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

#include <cstring>
#include <sptk5/Exception.h>
#include <sptk5/net/SSLContext.h>

// This include must be after SSLContext.h, or it breaks Windows compilation
#include <openssl/err.h>
#include <sptk5/Buffer.h>

using namespace std;
using namespace sptk;

static int s_server_session_id_context = 1;

void SSLContext::throwError(const String& humanDescription)
{
    unsigned long error = ERR_get_error();
    string errorStr = ERR_func_error_string(error) + string("(): ") + ERR_reason_error_string(error);
    throwException(humanDescription + "\n" + errorStr);
}

SSLContext::SSLContext()
{
    m_ctx = SSL_CTX_new(SSLv23_method());
    SSL_CTX_set_cipher_list(m_ctx, "ALL");
    SSL_CTX_set_mode(m_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_CTX_set_session_id_context(m_ctx, (const unsigned char*) &s_server_session_id_context, sizeof s_server_session_id_context);
}

SSLContext::~SSLContext()
{
    UniqueLock(*this);
    SSL_CTX_free(m_ctx);
}

SSL_CTX* SSLContext::handle()
{
    UniqueLock(*this);
    return m_ctx;
}

int SSLContext::passwordReplyCallback(char* replyBuffer, int replySize, int/*rwflag*/, void* userdata)
{
    strncpy(replyBuffer, (const char*) userdata, (size_t) replySize);
    replyBuffer[replySize - 1] = '\0';
    return (int) strlen(replyBuffer);
}

void SSLContext::loadKeys(const String& privateKeyFileName, const String& certificateFileName, const String& password,
                          const String& caFileName, int verifyMode, int verifyDepth)
{
    UniqueLock(*this);

    m_password = password;

    // Load keys and certificates
    if (SSL_CTX_use_certificate_chain_file(m_ctx, certificateFileName.c_str()) <= 0)
        throwError("Can't use certificate file " + certificateFileName);

    // Define password for auto-answer in callback function
    SSL_CTX_set_default_passwd_cb(m_ctx, passwordReplyCallback);
    SSL_CTX_set_default_passwd_cb_userdata(m_ctx, (void*) m_password.c_str());
    if (SSL_CTX_use_PrivateKey_file(m_ctx, privateKeyFileName.c_str(), SSL_FILETYPE_PEM) <= 0)
        throwError("Can't use private key file " + privateKeyFileName);

    if (SSL_CTX_check_private_key(m_ctx) == 0)
        throwError("Can't check private key file " + privateKeyFileName);

    // Load the CAs we trust
    if (!caFileName.empty() && SSL_CTX_load_verify_locations(m_ctx, caFileName.c_str(), nullptr) <= 0)
        throwError("Can't load or verify CA file " + caFileName);

    if (SSL_CTX_set_default_verify_paths(m_ctx) <= 0)
        throwError("Can't set default verify paths");

    SSL_CTX_set_verify(m_ctx, verifyMode, nullptr);
    SSL_CTX_set_verify_depth(m_ctx, verifyDepth);
}
