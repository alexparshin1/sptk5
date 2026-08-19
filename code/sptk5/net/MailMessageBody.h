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

#include <sptk5/String.h>
#include <sptk5/sptk.h>
#include <string>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief Defines the type of the mail message.
 */
enum class MailMessageType : uint8_t
{
    PLAIN_TEXT_MESSAGE, ///< Message has plain text only.
    HTML_MESSAGE        ///< Message has plain text and HTML parts.
};

/**
 * @brief Mail message body text.
 *
 * Contains the message text as plain text or as an HTML text and stripped HTML text (where HTML tags removed).
 */
class SP_EXPORT MailMessageBody
{
public:
    /**
     * @brief Default constructor.
     */
    MailMessageBody()
    {
        m_type = MailMessageType::PLAIN_TEXT_MESSAGE;
    }
    /**
     * @brief Sets the message text.
     *
     * Tries to detect the HTML messages by searching HTML tag in the first 100 bytes of the message.
     * @param messageText const std::string& messageText, the text of the message.
     * @param smtp bool, special processing for smtp.
     */
    void text(const std::string& messageText, bool smtp);

    /**
     * @brief Returns the message body type.
     */
    MailMessageType type() const
    {
        return m_type;
    }

    /**
     * @brief Returns the plain text version of the message.
     */
    const std::string& text() const
    {
        return m_plainText;
    }

    /**
     * @brief Returns the HTML version of the message (if presented).
     */
    const std::string& html() const
    {
        return m_htmlText;
    }

private:
    MailMessageType m_type;      ///< Message type.
    String          m_plainText; ///< Plain text part of the message.
    String          m_htmlText;  ///< Optional HTML part of the message.

    /**
     * @brief Builds a plain text string from HTML text.
     */
    static String stripHtml(const String& origHtml);
};
/**
 * @}
 */
} // namespace sptk
