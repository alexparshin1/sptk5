/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CFtpDS.h - description
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

#ifndef __CFTPDS_H__
#define __CFTPDS_H__

#include <sptk5/CMemoryDS.h>
#include <sptk5/net/CFTPConnect.h>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// Progression callback function to receive the information about operation progress
typedef void (*CProgressCallback)(int total,int progress);

/// Show policy defines what will be shown
enum CFtpShowPolicy {
    SHOW_ALL = 0,        /// Show everything
    HIDE_MESSAGES = 1,      /// Hide messages
    HIDE_FOLDERS = 2,    /// Hide folders
    NO_SORT = 4          /// Do not sort files and folders
};

/// @brief FTP datasource
///
/// Class CFtpDS allows to browse the list of files on FTP server.
/// It returns a dataset with file names, sizes, modification times etc.
class CFtpDS : public CMemoryDS  {
private:
    CFTPConnect           m_ftp;           /// FTP socket
    std::string           m_host;       /// FTP host name
    int                   m_port;       /// FTP port
    std::string           m_folder;        /// FTP folder name
    int                   m_showpolicy;    /// FTP show policy, see CFtpShowPolicy
    std::string           m_user;       /// FTP user name
    std::string           m_password;      /// FTP user password
    CProgressCallback     m_callback;      /// Internal callback to indicate the progress on open()
public:
    /// Default Constructor
    CFtpDS() : CMemoryDS(), m_port(21), m_showpolicy(0), m_callback(NULL) { }

    /// Destructor
    virtual ~CFtpDS() {
        close();
    }

    /// Returns current show policy, see CFtpShowPolicy
    int  showPolicy() const           {
        return m_showpolicy;
    }

    /// Defines current show policy, see CFtpShowPolicy
    void showPolicy(char type)        {
        m_showpolicy = type;
    }

    /// FTP host name
    void host(std::string host_name)      {
        m_host = host_name;
    }

    /// FTP host name
    std::string host() const              {
        return m_host;
    }

    /// FTP port number
    void port(int p)                  {
        m_port = p;
    }

    /// FTP port number
    int port() const                  {
        return m_port;
    }

    /// FTP user name
    void user(std::string usr)            {
        m_user = usr;
    }

    /// FTP user name
    std::string user() const              {
        return m_user;
    }

    /// FTP user password
    void password(std::string pwd)        {
        m_password = pwd;
    }

    /// FTP user password
    std::string password() const          {
        return m_password;
    }

    /// FTP folder name
    void folder(std::string d)            {
        m_folder = d;
    }

    /// FTP folder name
    const std::string &folder() const     {
        return m_folder;
    }

    /// Establishes the connection with FTP host,
    /// selects the FTP folder(),
    /// reads the list of files and directories,
    /// closes the FTP connection
    virtual bool              open();

    /// Sets the callback for the open() progression
    void callback(CProgressCallback cb) {
        m_callback = cb;
    }
};
/// @}
}
#endif
