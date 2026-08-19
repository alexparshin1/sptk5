/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/
#pragma once

#include "SSLContext.h"
#include <sptk5/net/SSLKeys.h>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

class CachedSSLContext
{
public:
    /**
     * @brief Loads private key and certificate(s).
     *
     * The private key and certificates must be encoded with PEM format.
     * A single file containing the private key and certificate can be used by supplying it for both,
     * private key and certificate parameters.
     * If the private key is protected with password, then password can be supplied to auto-answer.
     * @param keys				Keys.
	 * @param cipherList		Cipher list.
     * @param tlsOnly 			Use only TLS.
     * @return					Shared SSL context.
     */
    [[nodiscard]] SP_EXPORT static SharedSSLContext get(const SSLKeys& keys, const String& cipherList, bool tlsOnly);

private:
    /**
     * @brief A cached context, and what the key files looked like when it was built.
     *
     * Keys are recognised by their file names, and a file name is not what a certificate is:
     * renewal writes new content to the path that was always there. Remembering the size and
     * write time of each file is what lets a replaced certificate be noticed at all.
     */
    struct CachedContext
    {
        SharedSSLContext                      context;   ///< The context itself.
        String                                stamp;     ///< Key file sizes and write times it was built from.
        std::chrono::steady_clock::time_point inspected; ///< When those files were last looked at.
    };

    using CachedSSLContextMap = std::map<size_t, CachedContext>;

    /**
     * @brief How long a cached context is trusted before its files are looked at again.
     *
     * A context is asked for on every connection, and a stat() per key file on every connection
     * is a cost on the accept path that a certificate replaced once in sixty days does not earn.
     * Within this interval the cached context is handed out without touching the filesystem;
     * after it, the files are stamped once and the context is rebuilt only if they differ.
     */
    static constexpr std::chrono::seconds recheckInterval {1};

    /**
     * @brief How the key files look right now.
     *
     * Two different certificates written to the same path within one clock tick, at exactly the
     * same size, would still look alike. That is unlikely enough to leave alone, and nothing
     * short of reading both files would settle it.
     *
     * @param keys              Keys to stamp.
     * @return the stamp, with a placeholder for each file that is not there.
     */
    [[nodiscard]] static String stampOf(const SSLKeys& keys);

    static std::shared_mutex   m_mutex;    ///< Mutex for thread safety.
    static CachedSSLContextMap m_contexts; ///< Cached SSL contexts.
};

/**
 * @}
 */

} // namespace sptk
