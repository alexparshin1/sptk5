/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CImapDS.h - description
                             -------------------
    begin                : Mar 19 2003
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CIMAPDS_H__
#define __CIMAPDS_H__

#include <sptk5/CMemoryDS.h>
#include <sptk5/net/CImapConnect.h>
#include <string>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// Progression callback function prototype
typedef void (*CProgressCallback)(int total,int progress);

/// @brief IMAP datasource
///
/// Allows to browse the list of messages and folders on IMAP server.
/// It returns a dataset with message headers.
class CImapDS : public CMemoryDS  {
protected:
    CImapConnect         m_imap;        ///< IMAP socket connector
    std::string          m_folder;      ///< IMAP folder name
    std::string          m_user;        ///< IMAP user name
    std::string          m_password;    ///< IMAP user password
    bool                 m_fetchbody;   ///< True, if we want to fetch the message headers AND message body
    CProgressCallback    m_callback;    ///< Internal prograssion callback for open()
    int                  m_msgid;       ///< Internal message ID
public:

    /// Default constructor
    CImapDS() : CMemoryDS(), m_fetchbody(false), m_callback(NULL) {
        m_msgid = 0;
    }

    /// Destructor
    virtual ~CImapDS() {
        close();
    }

    /// IMAP host name
    void host(std::string host_name)  {
        m_imap.host(host_name);
    }

    /// IMAP host name
    std::string host() const          {
        return m_imap.host();
    }

    /// IMAP user name
    void user(std::string usr)        {
        m_user = usr;
    }

    /// IMAP user name
    std::string user() const          {
        return m_user;
    }

    /// IMAP user password
    void password(std::string pwd)    {
        m_password = pwd;
    }

    /// IMAP user password
    std::string password() const      {
        return m_password;
    }

    /// IMAP folder name
    void folder(std::string d)        {
        m_folder = d;
    }

    /// IMAP folder name
    const std::string &folder() const {
        return m_folder;
    }

    /// IMAP message ID (message number in the folder). If defined,
    /// the open() will retrieve only the message with the selected ID (if any)
    void messageID(int msgid)       {
        m_msgid = msgid;
    }

    /// Returns the ID of the message when defined to retrieve one message only
    int messageID()   const         {
        return m_msgid;
    }

    /// Sets the fetch body flag. Should be called prior to open().
    /// If the fetch body flag is not set, only the message headers will be retrieved and that is much faster.
    void fetchBody(bool fb)       {
        m_fetchbody = fb;
    }

    /// Returns the current value of the fetch body flag
    /// @returns the fetch body flag
    bool fetchBody() const        {
        return m_fetchbody;
    }

    /// Opens the IMAP server connection with user name and password defined with user() and password().
    /// Scans the IMAP folder defined with folder(), than closes the IMAP server connection.
    virtual bool open();

    /// Optional callback for the open() method progression.
    /// @param cb CProgressCallback, a callback function
    void callback(CProgressCallback cb) {
        m_callback = cb;
    }
};
/// @}
}
#endif
