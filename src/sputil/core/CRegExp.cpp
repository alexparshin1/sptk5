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
    m_pattern(pattern), m_global(false), m_pcreOptions()
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
    if (!m_pcre) {
        m_error = "PCRE pattern error at pattern offset " + int2string(errorOffset) + ": " + string(error);
    } else {
        m_pcreExtra = pcre_study(m_pcre, 0, &error);
        if (!m_pcreExtra) {
            pcre_free(m_pcre);
            m_pcre = NULL;
            m_error = "PCRE pattern study error : " + string(error);
        }
    }
}

CRegExp::~CRegExp()
{
    if (m_pcre) {
        pcre_free_study(m_pcreExtra);
        pcre_free(m_pcre);
    }
}

#define MAX_MATCHES 256

int CRegExp::match(const string& text, std::vector<Match>* matches) const throw (CException)
{
    if (!m_pcre)
        throwException(m_error);

    if (matches)
        matches->clear();

    Match   matchOffsets[MAX_MATCHES];
    int32_t startOffset = 0;

    int     textLength = text.length();

    int     iteration = 0, options = 0;
    bool    done = false;
    int     totalMatches = 0;
    while (!done) {
	const char* textPtr = text.c_str() + startOffset;
	unsigned fragmentOffset = 0;
        int matchCount = pcre_exec(m_pcre, m_pcreExtra, textPtr, textLength - startOffset, 0, options, (int*)matchOffsets, MAX_MATCHES);
        if (matchCount < 0) {
            if (matchCount == PCRE_ERROR_NOMATCH) {
                done = true;
                break;
            }
            throwException("PCRE match error");
        }

        int i = 0;
        if (i == 0 && matchCount > 1)
            i++; /// First match is a complete string
            
        for (; i < matchCount; i++) {
            Match& match = matchOffsets[i];
            if (match.m_start >= fragmentOffset) {
                fragmentOffset = match.m_end;
                totalMatches++;
                if (matches) {
		    match.m_start += startOffset;
                    match.m_end += startOffset;
                    matches->push_back(match);
		}
            }
        }
        
        startOffset += fragmentOffset;

        if (!m_global)
            break;

        iteration++;
    }
    return totalMatches;
}

bool CRegExp::operator == (std::string text) const throw (CException)
{
    return match(text.c_str(), NULL) > 0;
}

bool CRegExp::operator != (std::string text) const throw (CException)
{
    return match(text.c_str(), NULL) <= 0;
}

bool CRegExp::m(std::string text, CStrings& matchedStrings) const throw (CException)
{
    const char* textPtr = text.c_str();

    matchedStrings.clear();

    std::vector<Match> matches;
    int matchCount = match(textPtr, &matches);
    for (std::vector<Match>::iterator itor = matches.begin(); itor != matches.end(); itor++) {
        unsigned sz = itor->m_end - itor->m_start;
        string str(textPtr + itor->m_start, sz);
        matchedStrings.push_back(str);
    }
    return matchCount > 0;
}

string CRegExp::s(string text, string replacement) const throw (CException)
{
    std::vector<Match> matches;

    const char* textStart = text.c_str();
    int rc = match(textStart, &matches);
    if (!rc)
        return text; /// No matches
    string result;
    int fragmentStarts = 0;
    for (std::vector<Match>::iterator itor = matches.begin(); itor != matches.end(); itor++) {
        int substringStarts = itor->m_start;
        int substringEnds = itor->m_end;

        if (fragmentStarts > substringStarts) {
            fragmentStarts = substringEnds + 1;
            continue;
        }
        int fragmentEnds = substringStarts - 1;
        int fragmentLength = fragmentEnds - fragmentStarts + 1;
        string str(textStart + fragmentStarts, fragmentLength);
        result += str + replacement;
        fragmentStarts = substringEnds;
    }
    string str(textStart + fragmentStarts);
    return result + str;
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
