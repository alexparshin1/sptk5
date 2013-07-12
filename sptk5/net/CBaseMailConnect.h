/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CBaseMailConnect.h  -  description
                             -------------------
    begin                : 25 June 2003
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
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
#ifndef __CBASEMAILCONNECT_H__
#define __CBASEMAILCONNECT_H__

#include <sptk5/sptk.h>
#include <sptk5/CBuffer.h>
#include <sptk5/net/CMailMessageBody.h>

#include <string>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Base mail socket
///
/// CBaseMailConnect class is the base class for mail message components
class SP_EXPORT CBaseMailConnect {
protected:
    std::string      m_from;     ///< Mail FROM: a single e-mail address in format: "Jonh Doe <jonhd@noname.com>"
    std::string      m_to;       ///< Mail TO: semicolon-separated string of addresses in format: "Jonh Doe <jonhd@noname.com>; Jane Doe <janed@noname.com>"
    std::string      m_cc;       ///< Mail CC: semicolon-separated string of addresses in format: "Jonh Doe <jonhd@noname.com>; Jane Doe <janed@noname.com>"
    std::string      m_bcc;      ///< Mail BCC: semicolon-separated string of addresses in format: "Jonh Doe <jonhd@noname.com>; Jane Doe <janed@noname.com>"
    std::string      m_subject;  ///< Mail SUBJECT:
    CMailMessageBody m_body;     ///< Mail text (plain-text and html parts of the message)
    std::string      m_attachments; ///< The list of attachment files separated with ';'

    CBuffer m_messageBuffer; ///< Internal message buffer

    /// Encoding the message into internal message buffer
    void mimeFile(std::string fileName,std::string fileAlias,std::stringstream& message);

public:
    /// Default constructor
    CBaseMailConnect() {}

    /// Default destructor
    virtual ~CBaseMailConnect() {}

    /// Method from() returns the current value of 'FROM:' field
    /// of e-mail message.
    /// @returns a single e-mail address.
    std::string from() const        {
        return m_from;
    }

    /// Method from() sets the current value of 'FROM:' field
    /// of e-mail message.
    /// @param addr should be an e-mail address in format:
    /// Real sender name <sender@host.net>. The example: John Doe <johnd@unknown.org>
    void from(std::string addr)     {
        m_from = addr;
    }

    /// Method to() returns the current value of 'TO:' field
    /// of e-mail message
    std::string to() const          {
        return m_to;
    }

    /// Method from() sets the current value of 'TO:' field
    /// of e-mail message.
    /// @param addr should be a semicolon-separated list of one or more e-mail addresses in format:
    /// Real sender name <sender@host.net>. The example: John Doe <johnd@unknown.org>
    void to(std::string addr)       {
        m_to = addr;
    }

    /// Method cc() returns the current value of 'CC:' field
    /// of e-mail message
    /// @returns a list of e-mail addresses. See method to() description for format
    std::string cc() const          {
        return m_cc;
    }

    /// Method cc() sets the current value of 'CC:' field
    /// of e-mail message.
    /// @param addr should be a semicolon-separated list of one or more e-mail addresses in format:
    /// Real sender name <sender@host.net>. The example: John Doe <johnd@unknown.org>
    void cc(std::string addr)       {
        m_cc = addr;
    }

    /// Method bcc() returns the current value of 'BCC:' field
    /// of e-mail message.
    /// @returns a list of e-mail addresses. See method to() description for format
    std::string bcc() const         {
        return m_bcc;
    }

    /// Method bcc() sets the current value of 'BCC:' field
    /// of e-mail message.
    /// @param addr should be a semicolon-separated list of one or more e-mail addresses in format:
    /// Real sender name <sender@host.net>. The example: John Doe <johnd@unknown.org>
    void bcc(std::string addr)      {
        m_bcc = addr;
    }

    /// Method subject() returns the current value of 'SUBJECT:' field
    /// of e-mail message.
    /// @returns current message subject
    std::string subject() const     {
        return m_subject;
    }

    /// Method subject() sets the current value of 'BCC:' field
    /// of e-mail message.
    /// @param subj A message subject
    void subject(std::string subj)  {
        m_subject = subj;
    }

    /// Method subject() returns the current plain text part
    /// of e-mail message.
    /// @returns current message plain-text part
    std::string body() const        {
        return m_body.text();
    }

    /// @brief Sets the current plain text part of e-mail message.
    ///
    /// @param body std::string&, message body
    /// @param smtp bool, do we need special pre-processing for SMTP?
    void body(const std::string& body,bool smtp)     {
        m_body.text(body,smtp);
    }

    /// Method attachments() returns the current semicolon-separated
    /// list of attachments of e-mail message. Example: "readme.txt;readme.doc".
    /// @returns current message list of attachments
    std::string attachments() const {
        return m_attachments;
    }

    /// Method attachments() sets the current semicolon-separated
    /// list of attachments of e-mail message. Example: "readme.txt;readme.doc".
    /// @param attachments current message list of attachments
    void attachments(std::string attachments) {
        m_attachments = attachments;
    }

    /// Method messageBuffer() returns the reference to the internal current message text completely
    /// prepared for sending, as described in RFC-822 message format. It only makes sense to use it after call to sendMessage().
    /// @returns reference to current message text
    const CBuffer& messageBuffer() const {
        return m_messageBuffer;
    }

    /// Method mimeMessage() encodes the message components into RFC-822 message format.
    /// @param buffer A buffer to put the encoded RFC-822 format message
    void mimeMessage(CBuffer& buffer);

    /// Method sendMessage() builds an RFC-822 format message out of message parameters,
    /// and sends it. Should be implemented in derived classes.
    virtual void sendMessage() = 0;
};
/// @}
}
#endif
