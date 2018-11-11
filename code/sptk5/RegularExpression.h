/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       RegularExpression.h - description                      ║
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

#ifndef __SPTK_REGULAR_EXPRESSION_H__
#define __SPTK_REGULAR_EXPRESSION_H__

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>

#if HAVE_PCRE

#include <vector>
#include <pcre.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief PCRE-type regular expressions
 */
class SP_EXPORT RegularExpression
{
    /**
     * @brief Match position information
     */
    typedef struct {
        /**
         * Match start
         */
        int         m_start;

        /**
         * Match end
         */
        int         m_end;

    } Match;

    /**
     * Vector of match positions
     */
    typedef std::vector<Match> Matches;


    /**
     * Match pattern
     */
    sptk::String     m_pattern;

    /**
     * Global match (g) or first match only
     */
    bool            m_global;

    /**
     * Last pattern error (if any)
     */
    sptk::String     m_error;


    /**
     * Compiled PCRE expression handle
     */
    pcre*           m_pcre;

    /**
     * Compiled PCRE expression optimization (for faster execution)
     */
    pcre_extra*     m_pcreExtra;

    /**
     * PCRE pattern options
     */
    int32_t         m_pcreOptions;

    /**
     * Initialize PCRE expression
     */
    void initPCRE();

    /**
     * @brief Computes match positions and lengths
     * @param text              Input text
     * @param offset            Starting match offset, advanced with every successful match
     * @param matchOffsets      Output match positions array
     * @param matchOffsetsSize  Output match positions array size (in elements)
     * @return number of matches
     */
    size_t nextMatch(const sptk::String& text, size_t& offset, Match matchOffsets[], size_t matchOffsetsSize) const;

public:
    /**
     * @brief Constructor
     *
     * Pattern options is combination of flags matching Perl regular expression switches:
     * 'g'  global match, not just first one
     * 'i'  letters in the pattern match both upper and lower case  letters
     * 'm'  multiple lines match
     * 's'  dot character matches even newlines
     * 'x'  ignore whitespaces
     * @param pattern           PCRE pattern
     * @param options           Pattern options
     */
    RegularExpression(const sptk::String& pattern, const sptk::String& options = "");

    /**
     * Copy constructor
     * @param other             Other regular expression
     */
    RegularExpression(const RegularExpression& other);

    /**
     * @brief Destructor
     */
    virtual ~RegularExpression();

    /**
     * @brief Returns true if text matches with regular expression
     * @param text              Input text
     * @return true if match found
     */
    bool operator ==(const sptk::String& text) const;

    /**
     * @brief Returns true if text doesn't match with regular expression
     * @param text              Input text
     * @return true if match found
     */
    bool operator !=(const sptk::String& text) const;

    /**
     * @brief Returns true if text matches with regular expression
     * @param text              Text to process
     * @return true if match found
     */
    bool matches(const sptk::String& text) const;

    /**
     * @brief Returns list of strings matched with regular expression
     * @param text              Text to process
     * @param matchedStrings    List of matched strings
     * @return true if match found
     */
    bool m(const sptk::String& text, sptk::Strings& matchedStrings) const;

    /**
     * @brief Replaces matches with replacement string
     * @param text              Text to process
     * @param outputPattern     Output pattern using "\\N" as placeholders, with "\\1" as first match
     * @return processed text
     */
    sptk::String s(const sptk::String& text, const sptk::String& outputPattern) const;

    /**
     * @brief Returns list of strings split by regular expression
     * @param text              Text to process
     * @param outputStrings     List of matched strings
     * @return true if match found
     */
    bool split(const sptk::String& text, sptk::Strings& outputStrings) const;

    /**
     * @brief Replaces matches with replacement string
     * @param text              Text to process
     * @param outputPattern     Output pattern using "\\N" as placeholders, with "\\1" as first match
     * @param replaced          Optional flag if replacement was made
     * @return processed text
     */
    sptk::String replaceAll(const sptk::String& text, const sptk::String& outputPattern, bool& replaced) const;

    /**
     * @brief Replaces matches with replacement string from map, using matched string as an index
     * @param text              Text to process
     * @param substitutions     Substitutions for matched strings
     * @param replaced          Optional flag if replacement was made
     * @return processed text
     */
    sptk::String replaceAll(const sptk::String& text, const std::map<sptk::String,sptk::String>& substitutions, bool& replaced) const;
};

/**
 * @}
 */
}

#endif

#endif
