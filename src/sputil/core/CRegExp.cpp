/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRegExp.cpp  -  description
                             -------------------
    begin                : Sun Jul 07 2013
    copyright            : (C) 2000-2013 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CRegExp.h>
#include <sptk5/string_ext.h>

#if HAVE_PCRE

using namespace std;
using namespace sptk;

CRegExp::CRegExp(std::string pattern, string options) :
    m_pattern(pattern), m_global(false), m_pcre(NULL), m_pcreExtra(NULL), m_pcreOptions()
{
    for (unsigned i = 0; i < options.length(); i++) {
        switch (options[i]) {
        case 'i':
            m_pcreOptions |= PCRE_CASELESS;
            break;
        case 'm':
            m_pcreOptions |= PCRE_MULTILINE;
            break;
        case 's':
            m_pcreOptions |= PCRE_DOTALL;
            break;
        case 'x':
            m_pcreOptions |= PCRE_EXTENDED;
            break;
        case 'g': // Special case
            m_global = true;
            break;
        }
    }

    const char* error;
    int errorOffset;
    m_pcre = pcre_compile(m_pattern.c_str(), m_pcreOptions, &error, &errorOffset, NULL);
    if (!m_pcre)
        m_error = "PCRE pattern error at pattern offset " + int2string(errorOffset) + ": " + string(error);
#if PCRE_MAJOR > 7
    else {
        m_pcreExtra = pcre_study(m_pcre, 0, &error);
        if (!m_pcreExtra) {
            pcre_free(m_pcre);
            m_pcre = NULL;
            m_error = "PCRE pattern study error : " + string(error);
        }
    }
#endif
}

CRegExp::~CRegExp()
{
#if PCRE_MAJOR > 7
    if (m_pcreExtra)
        pcre_free_study(m_pcreExtra);
#endif
    if (m_pcre)
        pcre_free(m_pcre);
}

#define MAX_MATCHES 128
size_t CRegExp::nextMatch(const string& text, size_t& offset, Match matchOffsets[], size_t matchOffsetsSize) const throw (sptk::CException)
{
    if (!m_pcre)
        throwException(m_error);

    int rc = pcre_exec(m_pcre, m_pcreExtra, text.c_str(), text.length(), offset, 0, (int*)matchOffsets, matchOffsetsSize * 2);
    if (rc == PCRE_ERROR_NOMATCH)
        return 0;

    if (rc < 0) {
        switch (rc) {
            case PCRE_ERROR_NULL         : throwException("Null argument");
            case PCRE_ERROR_BADOPTION    : throwException("Invalid regular expression option");
            case PCRE_ERROR_BADMAGIC     : 
            case PCRE_ERROR_UNKNOWN_NODE : throwException("Invalid compiled regular expression\n");
            case PCRE_ERROR_NOMEMORY     : throwException("Out of memory");
            default                      : throwException("Unknown error");
        }
    }
    
    int matchCount = rc ? rc : MAX_MATCHES; // If match count is zero - there are too many matches

    offset = matchOffsets[0].m_end;
    
    return matchCount;
}

bool CRegExp::operator == (std::string text) const throw (CException)
{
    size_t  offset = 0;
    Match   matchOffsets[MAX_MATCHES];
    return nextMatch(text.c_str(), offset, matchOffsets, MAX_MATCHES) > 0;
}

bool CRegExp::operator != (std::string text) const throw (CException)
{
    size_t  offset = 0;
    Match   matchOffsets[MAX_MATCHES];
    return nextMatch(text.c_str(), offset, matchOffsets, MAX_MATCHES) == 0;
}

bool CRegExp::m(std::string text, CStrings& matchedStrings) const throw (CException)
{
    matchedStrings.clear();

    size_t  offset = 0;
    Match   matchOffsets[MAX_MATCHES];
    int     totalMatches = 0;
    
    do {
        int matchCount = nextMatch(text.c_str(), offset, matchOffsets, MAX_MATCHES);
        if (matchCount == 0) // No matches
            break;
        totalMatches += matchCount;

        for(int matchIndex = 1; matchIndex < matchCount; matchIndex++) {
            Match& match = matchOffsets[matchIndex];
            matchedStrings.push_back(string(text.c_str() + match.m_start, match.m_end - match.m_start));
        }
        
    } while (offset);
    
    return totalMatches > 0;
}

string CRegExp::s(string text, string outputPattern) const throw (CException)
{
    size_t  offset = 0;
    size_t  lastOffset = 0;
    Match   matchOffsets[MAX_MATCHES];
    int     totalMatches = 0;
    string  result;
    
    do {
        size_t fragmentOffset = offset;
        size_t matchCount = nextMatch(text.c_str(), offset, matchOffsets, MAX_MATCHES);
        if (matchCount == 0) // No matches
            break;
        if (matchCount == 1) // String matched but no strings extracted
            break;
        if (offset)
            lastOffset = offset;
        totalMatches += matchCount;
        
        // Create next replacement
        size_t pos = 0;
        string nextReplacement;
        while (pos != string::npos) {
            size_t placeHolderStart = pos;
            for (;;) {
                placeHolderStart = outputPattern.find("\\", placeHolderStart);
                if (placeHolderStart == string::npos)
                    break;
                if (isdigit(outputPattern[placeHolderStart + 1]))
                    break;
            }
            if (placeHolderStart == string::npos) {
                nextReplacement += outputPattern.substr(pos);
                break;
            }

            nextReplacement += outputPattern.substr(pos, placeHolderStart - pos);
            placeHolderStart++;
            size_t placeHolderIndex = atoi(outputPattern.c_str() + placeHolderStart);
            size_t placeHolderEnd = outputPattern.find_first_not_of("0123456789", placeHolderStart);
            if (placeHolderIndex < matchCount) {
                Match& match = matchOffsets[placeHolderIndex];
                const char* matchPtr = text.c_str() + match.m_start;
                nextReplacement += string(matchPtr, match.m_end - match.m_start);
            }
            pos = placeHolderEnd;
        }
        
        // Append text from fragment start to match start
        int fragmentStartLength = matchOffsets[0].m_start - fragmentOffset;
        if (fragmentStartLength)
            result += text.substr(offset, fragmentStartLength);
        
        // Append next replacement
        result += nextReplacement;
        
    } while (offset);
    
    return result + text.substr(lastOffset);
}

bool operator == (std::string text, const sptk::CRegExp& regexp) throw (sptk::CException)
{
    return regexp == text;
}

bool operator != (std::string text, const sptk::CRegExp& regexp) throw (sptk::CException)
{
    return regexp != text;
}

#endif
