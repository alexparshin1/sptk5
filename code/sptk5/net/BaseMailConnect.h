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

#include <sptk5/Buffer.h>
#include <sptk5/net/MailMessageBody.h>

#include <string>

namespace sptk {

/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Base mail socket.
 *
 * BaseMailConnect class is the base class for mail message components.
 */
class SP_EXPORT BaseMailConnect
{
public:
    /**
     * @brief Default constructor.
     */
    BaseMailConnect() = default;

    /**
     * @brief Destructor.
     */
    virtual ~BaseMailConnect() = default;

    /**
     * @brief Method from() returns the current value of the 'FROM:' field of the e-mail message.
     * @returns a single e-mail address.
     */
    String from() const noexcept
    {
        return m_from;
    }

    /**
     * @brief Method from() sets the current value of the 'FROM:' field of the e-mail message.
     * @param addr E-mail address in format:.
     * Real sender name <sender\@host.net>. The example: John Doe <johnd\@unknown.org>.
     */
    void from(const String& addr)
    {
        m_from = addr;
    }

    /**
     * @brief Method to() returns the current value of the 'TO:' field of the e-mail message.
     */
    String to() const noexcept
    {
        return m_to;
    }

    /**
     * @brief Method to() sets the current value of the 'TO:' field of the e-mail message.
     * @param addr Semicolon-separated list of one or more e-mail addresses in format:.
     * Real sender name <sender\@host.net>. The example: John Doe <johnd\@unknown.org>.
     */
    void to(const String& addr)
    {
        m_to = addr;
    }

    /**
     * @brief Method cc() returns the current value of the 'CC:' field of the e-mail message.
     * @returns List of e-mail addresses. See method to() description for format.
     */
    String cc() const noexcept
    {
        return m_cc;
    }

    /**
     * @brief Method cc() sets the current value of the 'CC:' field of the e-mail message.
     * @param addr Semicolon-separated list of one or more e-mail addresses in format:.
     * Real sender name <sender\@host.net>. The example: John Doe <johnd\@unknown.org>.
     */
    void cc(const String& addr)
    {
        m_cc = addr;
    }

    /**
     * @brief Method bcc() returns the current value of the 'BCC:' field of the e-mail message.
     * @returns List of e-mail addresses. See the to() method description for the format.
     */
    String bcc() const noexcept
    {
        return m_bcc;
    }

    /**
     * @brief Method bcc() sets the current value of the 'BCC:' field of the e-mail message.
     * @param addr Semicolon-separated list of one or more e-mail addresses in format:.
     * Real sender name <sender\@host.net>. The example: John Doe <johnd\@unknown.org>.
     */
    void bcc(const String& addr)
    {
        m_bcc = addr;
    }

    /**
     * @brief Method subject() returns the current value of the 'SUBJECT:' field of the e-mail message.
     * @returns current message subject.
     */
    String subject() const noexcept
    {
        return m_subject;
    }

    /**
     * @brief Method subject() sets the current value of the 'BCC:' field of the e-mail message.
     * @param subj A message subject.
     */
    void subject(const String& subj)
    {
        m_subject = subj;
    }

    /**
     * @brief Method body() returns the current plain text part of the e-mail message.
     * @returns current message plain-text part.
     */
    String body() const noexcept
    {
        return m_body.text();
    }

    /**
     * @brief Sets the current plain text part of the e-mail message.
     * @param body              Message body.
     * @param smtp              Do we need special pre-processing for SMTP?.
     */
    void body(const String& body, bool smtp)
    {
        m_body.text(body, smtp);
    }

    /**
     * @brief Method attachments() returns the current semicolon-separated list of attachments of the e-mail message.
     * Example: "readme.txt;readme.doc".
     * @returns current message list of attachments.
     */
    String attachments() const noexcept
    {
        return m_attachments;
    }

    /**
     * @brief Method attachments() sets the current semicolon-separated list of attachments of the e-mail message.
     * Example: "readme.txt;readme.doc".
     * @param attachments current message list of attachments.
     */
    void attachments(const String& attachments)
    {
        m_attachments = attachments;
    }

    /**
     * @brief Method messageBuffer() returns the reference to the internal current message text completely.
     * prepared for sending, as described in RFC-822 message format. It only makes sense to use it after call to sendMessage().
     * @returns reference to the current message text.
     */
    const Buffer& messageBuffer() const noexcept
    {
        return m_messageBuffer;
    }

    /**
     * @brief Method mimeMessage() encodes the message components into RFC-822 message format.
     * @param buffer A buffer to put the encoded RFC-822 format message.
     */
    void mimeMessage(Buffer& buffer);

protected:
    /**
     * @brief Method sendMessage() builds an RFC-822 format message out of message parameters and sends it.
     * Should be implemented in derived classes.
     */
    virtual void sendMessage() = 0;

    /**
     * @brief Encoding the message into the internal message buffer.
     */
    static void mimeFile(const String& fileName, const String& fileAlias, std::stringstream& message);

private:
    String          m_from;          ///< Mail FROM: a single e-mail address in format: "Jonh Doe <jonhd\@noname.com>".
    String          m_to;            ///< Mail TO: semicolon-separated string of addresses in format: "Jonh Doe <jonhd\@noname.com>; Jane Doe <janed\@noname.com>".
    String          m_cc;            ///< Mail CC: semicolon-separated string of addresses in format: "Jonh Doe <jonhd\@noname.com>; Jane Doe <janed\@noname.com>".
    String          m_bcc;           ///< Mail CC: semicolon-separated string of addresses in format: "Jonh Doe <jonhd\@noname.com>; Jane Doe <janed\@noname.com>".
    String          m_subject;       ///< Mail SUBJECT.
    MailMessageBody m_body;          ///< Mail text (plain-text and HTML parts of the message).
    String          m_attachments;   ///< The list of attachment files separated with ';'.
    Buffer          m_messageBuffer; ///< Internal message buffer.
};
/**
 * @}
 */
} // namespace sptk
