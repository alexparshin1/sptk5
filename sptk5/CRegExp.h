/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRegExp.h  -  description
                             -------------------
    begin                : Sun Jul 07 2013
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#ifndef __CREGULAREXPRESSION_H__
#define __CREGULAREXPRESSION_H__

#include <sptk5/sptk.h>

#if HAVE_PCRE

#include <sptk5/cutils>
#include <vector>
#include <pcre.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief PCRE-type regular expressions
class SP_EXPORT CRegExp
{
    /// @brief Match position information
    typedef struct {
        int         m_start;        ///< Match start
        int         m_end;          ///< Match end
    } Match;
    
    typedef std::vector<Match> Matches; ///< Vector of match positions

    std::string     m_pattern;      ///< Match pattern
    bool            m_global;       ///< Global match (g) or first match only
    std::string     m_error;        ///< Last pattern error (if any)

    pcre*           m_pcre;         ///< Compiled PCRE expression handle
    pcre_extra*     m_pcreExtra;    ///< Compiled PCRE expression optimization (for faster execution)
    int32_t         m_pcreOptions;  ///< PCRE pattern options

    /// @brief Computes match positions and lengths
    /// @param text const std::string&, Input text
    /// @param offset size_t&, starting match offset, advanced with every successful match
    /// @param matchOffsets Match*, Output match positions array
    /// @param matchOffsetsSize size_t, Output match positions array size (in elements)
    /// @return number of matches
    size_t nextMatch(const std::string& text, size_t& offset, Match matchOffsets[], size_t matchOffsetsSize) const THROWS_EXCEPTIONS;

public:
    /// @brief Constructor
    ///
    /// Pattern options is combination of flags matching Perl regular expression switches:
    /// 'g'  global match, not just first one
    /// 'i'  letters in the pattern match both upper and lower case  letters
    /// 'm'  multiple lines match
    /// 's'  dot character matches even newlines
    /// 'x'  ignore whitespaces
    /// @param pattern std::string, PCRE pattern
    /// @param options std::string, Pattern options
    CRegExp(std::string pattern, std::string options="");

    /// @brief Destructor
    virtual ~CRegExp();

    /// @brief Returns true if text matches with regular expression
    /// @param text std::string, Input text
    /// @return true if match found
    bool operator == (std::string text) const THROWS_EXCEPTIONS;

    /// @brief Returns true if text doesn't match with regular expression
    /// @param text std::string, Input text
    /// @return true if match found
    bool operator != (std::string text) const THROWS_EXCEPTIONS;

    /// @brief Returns list of strings matched with regular expression
    /// @param text std::string, Text to process
    /// @param matchedStrings sptk::CStrings&, list of matched strings
    /// @return true if match found
    bool m(std::string text, sptk::CStrings& matchedStrings) const THROWS_EXCEPTIONS;

    /// @brief Replaces matches with replacement string
    /// @param text std::string, text to process
    /// @param outputPattern std::string, output pattern using "\\N" as placeholders, with "\\1" as first match
    /// @return processed text
    std::string s(std::string text, std::string outputPattern) const THROWS_EXCEPTIONS;
};

/// @}
}

bool SP_EXPORT operator == (std::string text, const sptk::CRegExp& regexp) THROWS_EXCEPTIONS;
bool SP_EXPORT operator != (std::string text, const sptk::CRegExp& regexp) THROWS_EXCEPTIONS;

#endif

#endif
