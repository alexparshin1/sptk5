/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __MAILMESSAGEBODY_H__
#define __MAILMESSAGEBODY_H__

#include <sptk5/sptk.h>
#include <string>

namespace sptk
{

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Defines the type of the mail message
 */
enum MailMessageType
{
    /**
     * Message has plain text only
     */
    MMT_PLAIN_TEXT_MESSAGE,

    /**
     * Message has plain text and HTML parts
     */
    MMT_HTML_MESSAGE

};

/**
 * Mail message body text
 *
 * Contains the message text as plain text, or as an HTML text and stripped HTML text (where HTML tags removed)
 */
class SP_EXPORT MailMessageBody
{
public:

    /**
     * Default constructor
     */
    MailMessageBody()
    {
        m_type = MMT_PLAIN_TEXT_MESSAGE;
    }
    /**
     * Sets the message text.
     *
     * Tries to detect the HTML messages by searching HTML tag in the first 100 bytes of the message
     * @param messageText const std::string& messageText, the text of the message
     * @param smtp bool, special processing for smtp
     */
    void text(const std::string& messageText, bool smtp);

    /**
     * Returns the message body type
     */
    MailMessageType type() const
    {
        return m_type;
    }

    /**
     * Returns the plain text version of the message
     */
    const std::string& text() const
    {
        return m_plainText;
    }

    /**
     * Returns the html version of the message (if presented)
     */
    const std::string& html() const
    {
        return m_htmlText;
    }

private:

    MailMessageType     m_type;         ///< Message type
    String              m_plainText;    ///< Plain text part of the message
    String              m_htmlText;     ///< Optional HTML part of the message

    /**
     * Builds a plain text string from HTML text
     */
    static String stripHtml(const String& origHtml);
};
/**
 * @}
 */
}
#endif
