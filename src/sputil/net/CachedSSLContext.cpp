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

#include <sptk5/Buffer.h>
#include "sptk5/net/CachedSSLContext.h"

using namespace std;
using namespace sptk;

SharedMutex                             CachedSSLContext::m_mutex;
CachedSSLContext::CachedSSLContextMap   CachedSSLContext::m_contexts;

sptk::SSLContext* CachedSSLContext::get(const sptk::String& keyFileName, const sptk::String& certificateFileName,
                                        const sptk::String& password, const sptk::String& caFileName,
                                        int verifyMode, int verifyDepth)
{
    String ident = makeIdent(keyFileName, certificateFileName, password, caFileName, verifyMode, verifyDepth);

    UniqueLock(m_mutex);

    auto itor = m_contexts.find(ident);
    if (itor != m_contexts.end())
        return &itor->second;

    SSLContext& newContext = m_contexts[ident];
    if (!keyFileName.empty() || !certificateFileName.empty())
        newContext.loadKeys(keyFileName, certificateFileName, password, caFileName, verifyMode, verifyDepth);

    return &newContext;
}

String CachedSSLContext::makeIdent(const String& keyFileName, const String& certificateFileName, const String& password,
                             const String& caFileName, int verifyMode, int verifyDepth)
{
    Buffer buffer;
    buffer.append(keyFileName); buffer.append('~');
    buffer.append(certificateFileName); buffer.append('~');
    buffer.append(caFileName); buffer.append('~');
    buffer.append(int2string(verifyMode)); buffer.append('~');
    buffer.append(int2string(verifyDepth));
    return String(buffer.c_str(), buffer.length());
}
