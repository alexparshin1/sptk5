/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       BaseMailConnect.h - description                        ║
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

#ifndef __BASEMAILCONNECT_H__
#define __BASEMAILCONNECT_H__

#include <sptk5/sptk.h>
#include <sptk5/Buffer.h>
#include <sptk5/net/MailMessageBody.h>

#include <string>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Base mail socket
 *
 * BaseMailConnect class is the base class for mail message components
 */
class SP_EXPORT BaseMailConnect
{
protected:
    /**
     * Mail FROM: a single e-mail address in format: "Jonh Doe <jonhd\@noname.com>"
     */
    String              m_from;

    /**
     * Mail TO: semicolon-separated string of addresses in format: "Jonh Doe <jonhd\@noname.com>; Jane Doe <janed\@noname.com>"
     */
    String              m_to;

    /**
     * Mail CC: semicolon-separated string of addresses in format: "Jonh Doe <jonhd\@noname.com>; Jane Doe <janed\@noname.com>"
     */
    String              m_cc;

    /**
     * Mail BCC: semicolon-separated string of addresses in format: "Jonh Doe <jonhd\@noname.com>; Jane Doe <janed\@noname.com>"
     */
    String              m_bcc;

    /**
     * Mail SUBJECT:
     */
    String              m_subject;

    /**
     * Mail text (plain-text and html parts of the message)
     */
    MailMessageBody     m_body;

    /**
     * The list of attachment files separated with ';'
     */
    String              m_attachments;

    /**
     * Internal message buffer
     */
    Buffer             m_messageBuffer;

    /**
     * Encoding the message into internal message buffer
     */
    void mimeFile(const String& fileName, const String& fileAlias, std::stringstream& message);

public:
    /**
     * Default constructor
     */
    BaseMailConnect() noexcept
    {
    }

    /**
     * Method from() returns the current value of 'FROM:' field of e-mail message.
     * @returns a single e-mail address.
     */
    String from() const noexcept
    {
        return m_from;
    }

    /**
     * Method from() sets the current value of 'FROM:' field
     * of e-mail message.
     * @param addr should be an e-mail address in format:
     * Real sender name <sender\@host.net>. The example: John Doe <johnd\@unknown.org>
     */
    void from(const String& addr) 
    {
        m_from = addr;
    }

    /**
     * Method to() returns the current value of 'TO:' field
     * of e-mail message
     */
    String to() const noexcept
    {
        return m_to;
    }

    /**
     * Method from() sets the current value of 'TO:' field
     * of e-mail message.
     * @param addr should be a semicolon-separated list of one or more e-mail addresses in format:
     * Real sender name <sender\@host.net>. The example: John Doe <johnd\@unknown.org>
     */
    void to(const String& addr)
    {
        m_to = addr;
    }

    /**
     * Method cc() returns the current value of 'CC:' field
     * of e-mail message
     * @returns a list of e-mail addresses. See method to() description for format
     */
    String cc() const noexcept
    {
        return m_cc;
    }

    /**
     * Method cc() sets the current value of 'CC:' field
     * of e-mail message.
     * @param addr should be a semicolon-separated list of one or more e-mail addresses in format:
     * Real sender name <sender\@host.net>. The example: John Doe <johnd\@unknown.org>
     */
    void cc(const String& addr)
    {
        m_cc = addr;
    }

    /**
     * Method bcc() returns the current value of 'BCC:' field
     * of e-mail message.
     * @returns a list of e-mail addresses. See method to() description for format
     */
    String bcc() const noexcept
    {
        return m_bcc;
    }

    /**
     * Method bcc() sets the current value of 'BCC:' field of e-mail message.
     * @param addr should be a semicolon-separated list of one or more e-mail addresses in format:
     * Real sender name <sender\@host.net>. The example: John Doe <johnd\@unknown.org>
     */
    void bcc(const String& addr)
    {
        m_bcc = addr;
    }

    /**
     * Method subject() returns the current value of 'SUBJECT:' field of e-mail message.
     * @returns current message subject
     */
    String subject() const noexcept
    {
        return m_subject;
    }

    /**
     * Method subject() sets the current value of 'BCC:' field of e-mail message.
     * @param subj A message subject
     */
    void subject(const String& subj)
    {
        m_subject = subj;
    }

    /**
     * Method subject() returns the current plain text part of e-mail message.
     * @returns current message plain-text part
     */
    String body() const noexcept
    {
        return m_body.text();
    }

    /**
     * @brief Sets the current plain text part of e-mail message.
     * @param body              Message body
     * @param smtp              Do we need special pre-processing for SMTP?
     */
    void body(const String& body, bool smtp)
    {
        m_body.text(body, smtp);
    }

    /**
     * Method attachments() returns the current semicolon-separated
     * list of attachments of e-mail message. Example: "readme.txt;readme.doc".
     * @returns current message list of attachments
     */
    String attachments() const noexcept
    {
        return m_attachments;
    }

    /**
     * Method attachments() sets the current semicolon-separated
     * list of attachments of e-mail message. Example: "readme.txt;readme.doc".
     * @param attachments current message list of attachments
     */
    void attachments(const String& attachments)
    {
        m_attachments = attachments;
    }

    /**
     * Method messageBuffer() returns the reference to the internal current message text completely
     * prepared for sending, as described in RFC-822 message format. It only makes sense to use it after call to sendMessage().
     * @returns reference to current message text
     */
    const Buffer& messageBuffer() const noexcept
    {
        return m_messageBuffer;
    }

    /**
     * Method mimeMessage() encodes the message components into RFC-822 message format.
     * @param buffer A buffer to put the encoded RFC-822 format message
     */
    void mimeMessage(Buffer& buffer);

    /**
     * Method sendMessage() builds an RFC-822 format message out of message parameters,
     * and sends it. Should be implemented in derived classes.
     */
    virtual void sendMessage() = 0;
};
/**
 * @}
 */
}
#endif
