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

#include "sptk5/net/CachedSSLContext.h"

#include "sptk5/md5.h"

#include <filesystem>

using namespace std;
using namespace sptk;

shared_mutex                          CachedSSLContext::m_mutex;
CachedSSLContext::CachedSSLContextMap CachedSSLContext::m_contexts;

namespace {
/**
 * @brief Size and last-write time of a file, as a piece of a stamp.
 * @param file              File to look at, which need not exist.
 * @return the two values, or "-" for a file that is not there or cannot be looked at.
 */
String fileStamp(const filesystem::path& file)
{
    if (file.empty())
    {
        return "-";
    }

    error_code errorCode;
    const auto size = filesystem::file_size(file, errorCode);
    if (errorCode)
    {
        return "-";
    }

    const auto written = filesystem::last_write_time(file, errorCode);
    if (errorCode)
    {
        return "-";
    }

    return to_string(size) + ":" + to_string(written.time_since_epoch().count());
}
} // namespace

String CachedSSLContext::stampOf(const SSLKeys& keys)
{
    return fileStamp(keys.privateKeyFileName()) + "~" +
           fileStamp(keys.certificateFileName()) + "~" +
           fileStamp(keys.caFileName());
}

SharedSSLContext CachedSSLContext::get(const SSLKeys& keys, const String& cipherList, bool tlsOnly)
{
    const auto ident = std::hash<string> {}(keys.ident() + (tlsOnly ? "-tls-" : "-tls,ssl-") + cipherList);
    const auto now = chrono::steady_clock::now();

    {
        const shared_lock lock(m_mutex);
        if (const auto it = m_contexts.find(ident);
            it != m_contexts.end() && now - it->second.inspected < recheckInterval)
        {
            // The ordinary path, and the only one a busy server takes: recently checked, so the
            // files are not touched at all.
            return it->second.context;
        }
    }

    // Outside the lock: stamping reaches the filesystem, and holding writers off while it does
    // would put every connection behind one stat() at a time.
    const auto stamp = stampOf(keys);

    const unique_lock lock(m_mutex);

    if (const auto it = m_contexts.find(ident);
        it != m_contexts.end())
    {
        if (it->second.stamp == stamp)
        {
            // Same files as the context was built from. Marked as just looked at, so the next
            // second of connections goes back to the path above.
            it->second.inspected = now;
            return it->second.context;
        }

        // The files have changed underneath it - a renewal, or a certificate installed by hand.
        // The old context stays alive for as long as the connections already using it do.
    }

    auto context = make_shared<SSLContext>(cipherList, tlsOnly);

    if (!keys.privateKeyFileName().empty() || !keys.certificateFileName().empty())
    {
        context->loadKeys(keys);
    }

    m_contexts[ident] = {context, stamp, now};

    return context;
}
