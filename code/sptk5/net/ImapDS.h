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

#pragma once

#include <functional>
#include <sptk5/MemoryDS.h>
#include <sptk5/net/ImapConnect.h>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Progression callback function prototype.
 */
using ProgressCallback = std::function<void(int total, int progress)>;

/**
 * @brief IMAP datasource.
 *
 * Allows browsing the list of messages and folders on the IMAP server.
 * It returns a dataset with message headers.
 */
class SP_EXPORT ImapDS
    : public MemoryDS
{
public:
    /**
     * @brief Default constructor.
     */
    using MemoryDS::MemoryDS;

    /**
     * @brief Set IMAP host.
     */
    void host(const Host& host) const
    {
        m_imap.host(host);
    }

    /**
     * @brief Get IMAP host.
     */
    Host host() const
    {
        return m_imap.host();
    }

    /**
     * @brief IMAP username.
     */
    void user(const String& usr)
    {
        m_user = usr;
    }

    /**
     * @brief IMAP username.
     */
    const String& user() const
    {
        return m_user;
    }

    /**
     * @brief IMAP user password.
     */
    void password(const String& pwd)
    {
        m_password = pwd;
    }

    /**
     * @brief IMAP user password.
     */
    const String& password() const
    {
        return m_password;
    }

    /**
     * @brief IMAP folder name.
     */
    void folder(const String& d)
    {
        m_folder = d;
    }

    /**
     * @brief IMAP folder name.
     */
    const String& folder() const
    {
        return m_folder;
    }

    /**
     * @brief IMAP message ID (message number in the folder). If defined,
     * the open() will retrieve only the message with the selected ID (if any).
     */
    void messageID(int msgid)
    {
        m_msgid = msgid;
    }

    /**
     * @brief Returns the ID of the message when defined to retrieve one message only.
     */
    int messageID() const
    {
        return m_msgid;
    }

    /**
     * @brief Sets the fetch body flag. Should be called prior to open().
     * If the fetch body flag is not set, only the message headers will be retrieved, and that is much faster.
     */
    void fetchBody(bool fb)
    {
        m_fetchbody = fb;
    }

    /**
     * @brief Returns the current value of the fetch body flag.
     * @returns the fetch body flag.
     */
    bool fetchBody() const
    {
        return m_fetchbody;
    }

    /**
     * @brief Opens the IMAP server connection with username and password defined with user() and password().
     * Scans the IMAP folder defined with folder(), then closes the IMAP server connection.
     */
    bool open() override;

    /**
     * @brief Optional callback for the open() method progression.
     * @param cb CProgressCallback, a callback function.
     */
    void callback(const ProgressCallback& cb)
    {
        m_callback = cb;
    }

private:
    ImapConnect      m_imap;               ///< IMAP socket connector.
    String           m_folder;             ///< IMAP folder name.
    String           m_user;               ///< IMAP user name.
    String           m_password;           ///< IMAP user password.
    bool             m_fetchbody {false};  ///< Do we want to fetch the message headers AND message body?
    ProgressCallback m_callback {nullptr}; ///< Internal prograssion callback for open().
    int              m_msgid {0};          ///< Internal message ID.
};
/**
 * @}
 */
} // namespace sptk
