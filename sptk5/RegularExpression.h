/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       RegularExpression.h - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_REGULAREXPRESSION_H__
#define __SPTK_REGULAREXPRESSION_H__

#include <sptk5/sptk.h>

#if HAVE_PCRE

#include <sptk5/cutils>
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
    std::string     m_pattern;

    /**
     * Global match (g) or first match only
     */
    bool            m_global;

    /**
     * Last pattern error (if any)
     */
    std::string     m_error;


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
     * @brief Computes match positions and lengths
     * @param text const std::string&, Input text
     * @param offset size_t&, starting match offset, advanced with every successful match
     * @param matchOffsets Match*, Output match positions array
     * @param matchOffsetsSize size_t, Output match positions array size (in elements)
     * @return number of matches
     */
    size_t nextMatch(const std::string& text, size_t& offset, Match matchOffsets[], size_t matchOffsetsSize) const THROWS_EXCEPTIONS;

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
     * @param pattern std::string, PCRE pattern
     * @param options std::string, Pattern options
     */
    RegularExpression(const std::string& pattern, const std::string& options = "");

    /**
     * @brief Destructor
     */
    virtual ~RegularExpression();

    /**
     * @brief Returns true if text matches with regular expression
     * @param text std::string, Input text
     * @return true if match found
     */
    bool operator ==(const std::string& text) const THROWS_EXCEPTIONS;

    /**
     * @brief Returns true if text doesn't match with regular expression
     * @param text std::string, Input text
     * @return true if match found
     */
    bool operator !=(const std::string& text) const THROWS_EXCEPTIONS;

    /**
     * @brief Returns true if text matches with regular expression
     * @param text std::string, Text to process
     * @return true if match found
     */
    bool matches(const std::string& text) const THROWS_EXCEPTIONS;

    /**
     * @brief Returns list of strings matched with regular expression
     * @param text std::string, Text to process
     * @param matchedStrings sptk::Strings&, list of matched strings
     * @return true if match found
     */
    bool m(const std::string& text, sptk::Strings& matchedStrings) const THROWS_EXCEPTIONS;

    /**
     * @brief Replaces matches with replacement string
     * @param text std::string, text to process
     * @param outputPattern std::string, output pattern using "\\N" as placeholders, with "\\1" as first match
     * @return processed text
     */
    std::string s(const std::string& text, std::string outputPattern) const THROWS_EXCEPTIONS;

    /**
     * @brief Returns list of strings split by regular expression
     * @param text std::string, Text to process
     * @param outputStrings sptk::Strings&, list of matched strings
     * @return true if match found
     */
    bool split(const std::string& text, sptk::Strings& outputStrings) const THROWS_EXCEPTIONS;

    /**
     * @brief Replaces matches with replacement string
     * @param text std::string, text to process
     * @param outputPattern std::string, output pattern using "\\N" as placeholders, with "\\1" as first match
     * @param replaced bool&, optional flag if replacement was made
     * @return processed text
     */
    std::string replaceAll(const std::string& text, std::string outputPattern, bool& replaced) const THROWS_EXCEPTIONS;
};

typedef RegularExpression RegularExpression;

/**
 * @}
 */
}

bool SP_EXPORT operator == (const std::string& text, const sptk::RegularExpression& regexp) THROWS_EXCEPTIONS;
bool SP_EXPORT operator != (const std::string& text, const sptk::RegularExpression& regexp) THROWS_EXCEPTIONS;

#endif

#endif
