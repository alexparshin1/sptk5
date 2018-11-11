/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       RegularExpression.cpp - description                    ║
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

#include <sptk5/Exception.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/SystemException.h>
#include <sptk5/cutils>

#if HAVE_PCRE

using namespace std;
using namespace sptk;

void RegularExpression::initPCRE()
{
    const char* error;
    int errorOffset;
    m_pcre = pcre_compile(m_pattern.c_str(), m_pcreOptions, &error, &errorOffset, nullptr);
    if (!m_pcre)
        m_error = "PCRE pattern error at pattern offset " + int2string(errorOffset) + ": " + string(error);
#if PCRE_MAJOR > 7
    else {
        m_pcreExtra = pcre_study(m_pcre, 0, &error);
        if (!m_pcreExtra && error) {
            pcre_free(m_pcre);
            m_pcre = nullptr;
            m_error = "PCRE pattern study error : " + string(error);
        }
    }
#endif
}

RegularExpression::RegularExpression(const String& pattern, const String& options)
: m_pattern(pattern), m_global(false), m_pcre(nullptr), m_pcreExtra(nullptr), m_pcreOptions()
{
    for (auto ch: options) {
        switch (ch) {
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
            default:
                break;
        }
    }

    initPCRE();
}

RegularExpression::RegularExpression(const RegularExpression& other)
: m_pattern(other.m_pattern), m_global(other.m_global), m_pcre(nullptr), m_pcreExtra(nullptr), m_pcreOptions(other.m_pcreOptions)
{
    initPCRE();
}

RegularExpression::~RegularExpression()
{
#if PCRE_MAJOR > 7
    if (m_pcreExtra)
        pcre_free_study(m_pcreExtra);
#endif
    if (m_pcre)
        pcre_free(m_pcre);
}

#define MAX_MATCHES 128

size_t RegularExpression::nextMatch(const String& text, size_t& offset, Match matchOffsets[],
                                    size_t matchOffsetsSize) const
{
    if (!m_pcre) throwException(m_error);

    int rc = pcre_exec(m_pcre, m_pcreExtra, text.c_str(), (int) text.length(), (int) offset, 0, (int*) matchOffsets,
                       (int) matchOffsetsSize * 2);
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

    offset = (size_t) matchOffsets[0].m_end;

    return (size_t) matchCount;
}

bool RegularExpression::operator==(const String& text) const
{
    size_t offset = 0;
    Match matchOffsets[MAX_MATCHES];
    return nextMatch(text, offset, matchOffsets, MAX_MATCHES) > 0;
}

bool RegularExpression::operator!=(const String& text) const
{
    size_t offset = 0;
    Match matchOffsets[MAX_MATCHES];
    return nextMatch(text, offset, matchOffsets, MAX_MATCHES) == 0;
}

bool RegularExpression::matches(const String& text) const
{
    size_t offset = 0;
    Match matchOffsets[MAX_MATCHES];
    size_t matchCount = nextMatch(text, offset, matchOffsets, MAX_MATCHES);
    return matchCount > 0;
}

bool RegularExpression::m(const String& text, Strings& matchedStrings) const
{
    matchedStrings.clear();

    size_t offset = 0;
    Match matchOffsets[MAX_MATCHES];
    size_t totalMatches = 0;

    do {
        size_t matchCount = nextMatch(text, offset, matchOffsets, MAX_MATCHES);
        if (matchCount == 0) // No matches
            break;
        totalMatches += matchCount;

        for (size_t matchIndex = 1; matchIndex < matchCount; matchIndex++) {
            Match& match = matchOffsets[matchIndex];
            matchedStrings.push_back(string(text.c_str() + match.m_start, size_t(match.m_end - match.m_start)));
        }

    } while (offset < text.length());

    return totalMatches > 0;
}

bool RegularExpression::split(const String& text, Strings& matchedStrings) const
{
    matchedStrings.clear();

    size_t offset = 0;
    Match matchOffsets[MAX_MATCHES];
    size_t totalMatches = 0;

    int lastMatchEnd = 0;
    do {
        size_t matchCount = nextMatch(text, offset, matchOffsets, MAX_MATCHES);
        if (matchCount == 0) // No matches
            break;

        totalMatches += matchCount;

        for (size_t matchIndex = 0; matchIndex < matchCount; matchIndex++) {
            Match& match = matchOffsets[matchIndex];
            matchedStrings.push_back(string(text.c_str() + lastMatchEnd, size_t(match.m_start - lastMatchEnd)));
            lastMatchEnd = match.m_end;
        }

    } while (offset);

    matchedStrings.push_back(string(text.c_str() + lastMatchEnd));

    return totalMatches > 0;
}

String RegularExpression::replaceAll(const String& text, const String& outputPattern, bool& replaced) const
{
    size_t offset = 0;
    size_t lastOffset = 0;
    Match matchOffsets[MAX_MATCHES];
    size_t totalMatches = 0;
    string result;

    replaced = false;

    do {
        size_t fragmentOffset = offset;
        size_t matchCount = nextMatch(text, offset, matchOffsets, MAX_MATCHES);
        if (matchCount == 0) // No matches
            break;
        //if (matchCount == 1) // String matched but no strings extracted
        //    break;
        if (offset)
            lastOffset = offset;
        totalMatches += matchCount;

        // Create next replacement
        size_t pos = 0;
        string nextReplacement;
        replaced = true;
        while (pos != string::npos) {
            size_t placeHolderStart = pos;
            for (;; placeHolderStart++) {
                placeHolderStart = outputPattern.find('\\', placeHolderStart);
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
            auto placeHolderIndex = (size_t) string2int(outputPattern.c_str() + placeHolderStart);
            size_t placeHolderEnd = outputPattern.find_first_not_of("0123456789", placeHolderStart);
            if (placeHolderIndex < matchCount) {
                Match& match = matchOffsets[placeHolderIndex];
                const char* matchPtr = text.c_str() + match.m_start;
                nextReplacement += string(matchPtr, size_t(match.m_end) - size_t(match.m_start));
            }
            pos = placeHolderEnd;
        }

        // Append text from fragment start to match start
        size_t fragmentStartLength = size_t(matchOffsets[0].m_start) - size_t(fragmentOffset);
        if (fragmentStartLength)
            result += text.substr(fragmentOffset, fragmentStartLength);

        // Append next replacement
        result += nextReplacement;

    } while (offset);

    return result + text.substr(lastOffset);
}

String RegularExpression::replaceAll(const String& text, const map<String, String>& substitutions, bool& replaced) const
{
    size_t offset = 0;
    size_t lastOffset = 0;
    Match matchOffsets[MAX_MATCHES];
    size_t totalMatches = 0;
    string result;

    // For "i" option, make lowercase match map
    map<String, String> substitutionsMap;
    if (m_pcreOptions & PCRE_CASELESS) {
        for (auto& itor: substitutions)
            substitutionsMap[lowerCase(itor.first)] = itor.second;
    } else
        substitutionsMap = substitutions;

    replaced = false;

    do {
        size_t fragmentOffset = offset;
        size_t matchCount = nextMatch(text, offset, matchOffsets, MAX_MATCHES);
        if (matchCount == 0) // No matches
            break;
        //if (matchCount == 1) // String matched but no strings extracted
        //    break;
        if (offset)
            lastOffset = offset;
        totalMatches += matchCount;

        replaced = true;

        // Append text from fragment start to match start
        size_t fragmentStartLength = size_t(matchOffsets[0].m_start) - size_t(fragmentOffset);
        if (fragmentStartLength)
            result += text.substr(fragmentOffset, fragmentStartLength);

        // Append replacement
        string currentMatch(text.c_str() + matchOffsets[0].m_start,
                            (unsigned) matchOffsets[0].m_end - (unsigned) matchOffsets[0].m_start);

        map<String, String>::iterator itor;
        if (m_pcreOptions & PCRE_CASELESS)
            itor = substitutionsMap.find(lowerCase(currentMatch));
        else
            itor = substitutionsMap.find(currentMatch);

        String nextReplacement;
        if (itor == substitutionsMap.end())
            nextReplacement = currentMatch;
        else
            nextReplacement = itor->second;

        result += nextReplacement;

    } while (offset);

    return result + text.substr(lastOffset);
}

String RegularExpression::s(const String& text, const String& outputPattern) const
{
    bool replaced;
    return replaceAll(text, outputPattern, replaced);
}

#if USE_GTEST
#include <gtest/gtest.h>

static const char* testPhrase = "This is a test text to verify MD5 algorithm";

TEST(SPTK_RegularExpression, match)
{
    RegularExpression match1("test.*verify");
    RegularExpression match2("test  .*verify");
    EXPECT_TRUE(match1.matches(testPhrase));
    EXPECT_FALSE(match2.matches(testPhrase));
    EXPECT_TRUE(match1 == String(testPhrase));
    EXPECT_FALSE(match1 != String(testPhrase));
}

TEST(SPTK_RegularExpression, replase)
{
    RegularExpression match1("^(.*)(text).*(verify)(.*)");
    EXPECT_STREQ("This is a test expression to test MD5 algorithm", match1.s(testPhrase, "\\1expression to test\\4").c_str());
}

TEST(SPTK_RegularExpression, extract)
{
    RegularExpression match1("^(.*)(text).*(verify)(.*)");
    Strings matchedStrings;
    EXPECT_TRUE(match1.m(testPhrase, matchedStrings));
    EXPECT_EQ(size_t(4), matchedStrings.size());
    EXPECT_STREQ("This is a test ", matchedStrings[0].c_str());
    EXPECT_STREQ(" MD5 algorithm", matchedStrings[3].c_str());
}

TEST(SPTK_RegularExpression, split)
{
    RegularExpression match("[\\s]+");
    Strings matchedStrings;
    EXPECT_TRUE(match.split(testPhrase, matchedStrings));
    EXPECT_EQ(size_t(9), matchedStrings.size());
    EXPECT_STREQ("This", matchedStrings[0].c_str());
    EXPECT_STREQ("algorithm", matchedStrings[8].c_str());
}

#endif

#endif
