/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <future>
#include <regex>
#include <sptk5/cutils>

#if defined(HAVE_PCRE) | defined(HAVE_PCRE2)

using namespace std;
using namespace sptk;

namespace sptk {

struct Match {
    pcre_offset_t m_start {0}; ///< Match start
    pcre_offset_t m_end {0};   ///< Match end
};

class MatchData
{
    friend class RegularExpression;

public:
#ifdef HAVE_PCRE2
    shared_ptr<pcre2_match_data> match_data;

    MatchData(pcre2_code* pcre, size_t maxMatches)
        : match_data(shared_ptr<pcre2_match_data>(pcre2_match_data_create_from_pattern(pcre, nullptr),
                                                  [](auto* ptr) {
                                                      pcre2_match_data_free(ptr);
                                                  }))
        , matches(maxMatches + 2)
        , maxMatches(maxMatches + 2)
    {
    }

#else
    static constexpr int reservedMatches = 4;

    MatchData(const pcre*, size_t maxMatches)
        : matches(maxMatches + (size_t) reservedMatches * 2)
        , maxMatches(maxMatches + reservedMatches)
    {
    }
#endif

    MatchData(const MatchData&) = delete;

    MatchData& operator=(const MatchData&) = delete;

    vector<Match> matches;
    size_t maxMatches {0};
};

} // namespace sptk

size_t RegularExpression::getCaptureCount() const
{
    int captureCount = 0;

    if (
#ifdef HAVE_PCRE2
        pcre2_pattern_info(m_pcre.get(), PCRE2_INFO_CAPTURECOUNT, &captureCount)
#else
        pcre_fullinfo(m_pcre.get(), m_pcreExtra.get(), PCRE_INFO_CAPTURECOUNT, &captureCount)
#endif
        != 0)
    {
        captureCount = 0;
    }

    return (size_t) captureCount;
}

const RegularExpression::Group RegularExpression::Groups::emptyGroup;

const RegularExpression::Group& RegularExpression::Groups::operator[](int index) const
{
    if (size_t(index) >= m_groups.size())
    {
        return emptyGroup;
    }
    return m_groups[index];
}

const RegularExpression::Group& RegularExpression::Groups::operator[](const char* name) const
{
    auto itor = m_namedGroups.find(name);
    if (itor == m_namedGroups.end())
    {
        return emptyGroup;
    }
    return itor->second;
}

void RegularExpression::Groups::grow(size_t groupCount)
{
    m_groups.reserve(m_groups.size() + groupCount);
}

void RegularExpression::compile()
{
#ifdef HAVE_PCRE2
    int errornumber {0};
    PCRE2_SIZE erroroffset {0};

    auto* pcre = pcre2_compile(
        (PCRE2_SPTR) m_pattern.c_str(), // the pattern
        PCRE2_ZERO_TERMINATED,          // indicates pattern is zero-terminated
        m_options,                      // options
        &errornumber,                   // for error number
        &erroroffset,                   // for error offset
        nullptr);                       // use default compile context

    if (pcre == nullptr)
    {
        array<PCRE2_UCHAR, 256> buffer {};
        pcre2_get_error_message(errornumber, buffer.data(), sizeof(buffer));
        throw Exception((const char*) buffer.data());
    }

    m_pcre = shared_ptr<PCREHandle>(pcre,
                                    [](auto* ptr) {
                                        pcre2_code_free(ptr);
                                    });

#else
    const char* error = nullptr;
    int errorOffset = 0;

    auto* pcre = pcre_compile(m_pattern.c_str(), (int) m_options, &error, &errorOffset, nullptr);
    m_pcre = shared_ptr<PCREHandle>(pcre,
                                    [](auto* pcreHandle) {
                                        pcre_free(pcreHandle);
                                    });

    if (!m_pcre)
        m_error = "PCRE pattern error at pattern offset " + int2string(errorOffset) + ": " + string(error);
#if PCRE_MAJOR > 7
    else
    {
        auto* pcreExtra = pcre_study(m_pcre.get(), 0, &error);
        if (!pcreExtra && error)
        {
            m_error = "PCRE pattern study error : " + string(error);
        }
        else
        {
            m_pcreExtra = shared_ptr<PCREExtraHandle>(pcreExtra,
                                                      [](pcre_extra* study) {
                                                          pcre_free_study(study);
                                                      });
        }
    }
#endif
#endif
    m_captureCount = getCaptureCount();
}

RegularExpression::RegularExpression(const String& pattern, const String& options)
    : m_pattern(pattern)
{
    for (auto ch: options)
    {
        switch (ch)
        {
            case 'i':
                m_options |= SPRE_CASELESS;
                break;
            case 'm':
                m_options |= SPRE_MULTILINE;
                break;
            case 's':
                m_options |= SPRE_DOTALL;
                break;
            case 'x':
                m_options |= SPRE_EXTENDED;
                break;
            case 'g': // Special case
                m_global = true;
                break;
            default:
                break;
        }
    }
    compile();
}

size_t RegularExpression::nextMatch(const String& text, size_t& offset, MatchData& matchData) const
{
    if (!m_pcre)
        throwException(m_error)

#ifdef HAVE_PCRE2

            const auto* ovector = pcre2_get_ovector_pointer(matchData.match_data.get());

    auto rc = pcre2_match(
        m_pcre.get(),               // the compiled pattern
        (PCRE2_SPTR) text.c_str(),  // the subject string
        text.length(),              // the length of the subject
        offset,                     // start at offset in the subject
        0,                          // default options
        matchData.match_data.get(), // block for storing the result
        nullptr);                   // use default match context

    if (rc >= 0)
    {
        memcpy((uint8_t*) matchData.matches.data(), ovector, sizeof(pcre_offset_t) * 2 * rc);
        offset = ovector[1];
        return size_t(rc); // match count
    }

    if (rc == PCRE2_ERROR_NOMATCH)
    {
        if (m_options == 0)
        {
            return false;
        }         /* All matches found */
        ++offset; /* Advance one code unit */
    }

    return rc >= 0;
#else
            int rc = pcre_exec(
                m_pcre.get(), m_pcreExtra.get(), text.c_str(), (int) text.length(), (int) offset, 0,
                (pcre_offset_t*) matchData.matches.data(),
                (pcre_offset_t) matchData.maxMatches * 2);

    if (rc == PCRE_ERROR_NOMATCH)
        return 0;

    if (rc < 0)
    {
        switch (rc)
        {
            case PCRE_ERROR_NULL:
                throwException("Null argument") case PCRE_ERROR_BADOPTION : throwException("Invalid regular expression option") case PCRE_ERROR_BADMAGIC : case PCRE_ERROR_UNKNOWN_NODE : throwException("Invalid compiled regular expression\n") case PCRE_ERROR_NOMEMORY : throwException("Out of memory") default : throwException("Unknown error")
        }
    }

    int matchCount = rc; // If match count is zero - there are too many matches

    offset = (size_t) matchData.matches[0].m_end;
    return (size_t) matchCount;
#endif
}

bool RegularExpression::operator==(const String& text) const
{
    size_t offset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);
    return nextMatch(text, offset, matchData) > 0;
}

bool RegularExpression::operator!=(const String& text) const
{
    size_t offset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);
    return nextMatch(text, offset, matchData) == 0;
}

bool RegularExpression::matches(const String& text) const
{
    size_t offset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);
    size_t matchCount = nextMatch(text, offset, matchData);
    return matchCount > 0;
}

RegularExpression::Groups RegularExpression::m(const String& text, size_t& offset) const
{
    Groups matchedStrings;

    MatchData matchData(m_pcre.get(), m_captureCount);

    bool first {true};
    do
    {
        size_t matchCount = nextMatch(text, offset, matchData);
        if (matchCount == 0)
        { // No matches
            break;
        }

        matchedStrings.grow(matchCount);

        size_t matchIndex = 0;
        if (matchCount > 1)
        {
            ++matchIndex;
        }

        for (; matchIndex < matchCount; ++matchIndex)
        {
            const Match& match = matchData.matches[matchIndex];
            if (match.m_start >= 0)
            {
                matchedStrings.add(
                    Group(
                        string(text.c_str() + match.m_start,
                               size_t(match.m_end - match.m_start)),
                        match.m_start, match.m_end));
            }
            else
            {
                matchedStrings.add(Group());
            }
        }

        if (first)
        {
            extractNamedMatches(text, matchedStrings, matchData, matchCount);
        }

        first = false;

    } while (m_global && offset < text.length());

    return matchedStrings;
}

void RegularExpression::extractNamedMatches(const String& text, RegularExpression::Groups& matchedStrings,
                                            const MatchData& matchData, size_t matchCount) const
{
    auto nameCount = (int) getNamedGroupCount();
    if (nameCount > 0)
    {
        const char* nameTable = nullptr;
        int nameEntrySize = 0;
        getNameTable(nameTable, nameEntrySize);
        const auto* tabptr = nameTable;
        for (int i = 0; i < nameCount; ++i)
        {
            auto n = size_t(((int) tabptr[0] << 8) | (int) tabptr[1]);
            String name(tabptr + 2, size_t(nameEntrySize - 3));
            if (const auto& match = matchData.matches[n]; match.m_start >= 0 && n < matchCount)
            {
                String value(text.c_str() + match.m_start, size_t(match.m_end - match.m_start));
                matchedStrings.add(name.c_str(), Group(value, match.m_start, match.m_end));
            }
            else
            {
                matchedStrings.add(name.c_str(), Group());
            }
            tabptr += nameEntrySize;
        }
    }
}

void RegularExpression::getNameTable(const char*& nameTable, int& nameEntrySize) const
{
    nameEntrySize = 0;
#ifdef HAVE_PCRE2
    pcre2_pattern_info(m_pcre.get(), PCRE2_INFO_NAMETABLE, &nameTable);
    pcre2_pattern_info(m_pcre.get(), PCRE2_INFO_NAMEENTRYSIZE, &nameEntrySize);
#else
    pcre_fullinfo(m_pcre.get(), m_pcreExtra.get(), PCRE_INFO_NAMETABLE, &nameTable);
    pcre_fullinfo(m_pcre.get(), m_pcreExtra.get(), PCRE_INFO_NAMEENTRYSIZE, &nameEntrySize);
#endif
}

size_t RegularExpression::getNamedGroupCount() const
{
    int nameCount = 0;

    if (
#ifdef HAVE_PCRE2
        pcre2_pattern_info(m_pcre.get(), PCRE2_INFO_NAMECOUNT, &nameCount)
#else
        pcre_fullinfo(m_pcre.get(), m_pcreExtra.get(), PCRE_INFO_NAMECOUNT, &nameCount)
#endif
        != 0)
    {
        nameCount = 0;
    }

    return (size_t) nameCount;
}

Strings RegularExpression::split(const String& text) const
{
    Strings matchedStrings;

    size_t offset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);

    pcre_offset_t lastMatchEnd = 0;
    do
    {
        size_t matchCount = nextMatch(text, offset, matchData);
        if (matchCount == 0)
        { // No matches
            break;
        }

        for (size_t matchIndex = 0; matchIndex < matchCount; ++matchIndex)
        {
            const Match& match = matchData.matches[matchIndex];
            matchedStrings.push_back(string(text.c_str() + lastMatchEnd, size_t(match.m_start - lastMatchEnd)));
            lastMatchEnd = match.m_end;
        }

    } while (offset);

    matchedStrings.push_back(string(text.c_str() + lastMatchEnd));

    return matchedStrings;
}

String RegularExpression::replaceAll(const String& text, const String& outputPattern, bool& replaced) const
{
    size_t offset = 0;
    size_t lastOffset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);
    string result;

    replaced = false;

    do
    {
        size_t fragmentOffset = offset;
        size_t matchCount = nextMatch(text, offset, matchData);
        if (matchCount == 0)
        { // No matches
            break;
        }
        if (offset)
        {
            lastOffset = offset;
        }

        // Create next replacement
        size_t pos = 0;
        string nextReplacement;
        replaced = true;
        while (pos != string::npos)
        {
            size_t placeHolderStart = findNextPlaceholder(pos, outputPattern);

            if (placeHolderStart == string::npos)
            {
                nextReplacement += outputPattern.substr(pos);
                break;
            }

            nextReplacement += outputPattern.substr(pos, placeHolderStart - pos);
            ++placeHolderStart;
            auto placeHolderIndex = (size_t) string2int(outputPattern.c_str() + placeHolderStart);
            size_t placeHolderEnd = outputPattern.find_first_not_of("0123456789", placeHolderStart);
            if (placeHolderIndex < matchCount)
            {
                const Match& match = matchData.matches[placeHolderIndex];
                const char* matchPtr = text.c_str() + match.m_start;
                nextReplacement += string(matchPtr, size_t(match.m_end) - size_t(match.m_start));
            }
            pos = placeHolderEnd;
        }

        // Append text from fragment start to match start
        if (size_t fragmentStartLength = size_t(matchData.matches[0].m_start) - size_t(fragmentOffset);
            fragmentStartLength != 0)
        {
            result += text.substr(fragmentOffset, fragmentStartLength);
        }

        // Append next replacement
        result += nextReplacement;

    } while (offset);

    if (lastOffset < text.length())
    {
        return result + text.substr(lastOffset);
    }

    return result;
}

String RegularExpression::s(const String& text, const std::function<String(const String&)>& replace,
                            bool& replaced) const
{
    size_t offset = 0;
    size_t lastOffset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);
    string result;

    replaced = false;

    do
    {
        size_t fragmentOffset = offset;
        if (size_t matchCount = nextMatch(text, offset, matchData); matchCount == 0)
        {
            break;
        } // No matches
        if (offset)
        {
            lastOffset = offset;
        }

        replaced = true;

        // Append text from fragment start to match start
        if (size_t fragmentStartLength = size_t(matchData.matches[0].m_start) - size_t(fragmentOffset);
            fragmentStartLength != 0)
        {
            result += text.substr(fragmentOffset, fragmentStartLength);
        }

        // Append replacement
        String currentMatch(text.c_str() + matchData.matches[0].m_start,
                            (unsigned) matchData.matches[0].m_end - (unsigned) matchData.matches[0].m_start);

        String nextReplacement = replace(currentMatch);

        result += nextReplacement;

    } while (offset);

    return result + text.substr(lastOffset);
}

size_t RegularExpression::findNextPlaceholder(size_t pos, const String& outputPattern)
{
    size_t placeHolderStart = pos;
    for (;; ++placeHolderStart)
    {
        placeHolderStart = outputPattern.find('\\', placeHolderStart);
        if (placeHolderStart == string::npos || isdigit(outputPattern[placeHolderStart + 1]))
        {
            break;
        }
    }
    return placeHolderStart;
}

String RegularExpression::replaceAll(const String& text, const map<String, String>& substitutions, bool& replaced) const
{
    // For "i" option, make lowercase match map
    map<String, String> substitutionsMap;
    bool ignoreCase = (m_options & SPRE_CASELESS) == SPRE_CASELESS;
    if (ignoreCase)
    {
        for (const auto& [name, value]: substitutions)
        {
            substitutionsMap[lowerCase(name)] = value;
        }
    }
    else
    {
        substitutionsMap = substitutions;
    }

    return s(
        text, [&substitutionsMap, ignoreCase](const String& needle) {
            auto itor = substitutionsMap.find(ignoreCase ? needle.toLowerCase() : needle);
            if (itor == substitutionsMap.end())
            {
                return needle;
            }
            return itor->second;
        },
        replaced);
}

String RegularExpression::s(const String& text, const String& outputPattern) const
{
    bool replaced = false;
    return replaceAll(text, outputPattern, replaced);
}

const String& RegularExpression::pattern() const
{
    return m_pattern;
}

#ifdef USE_GTEST

static const String testPhrase("This is a test text to verify rexec text data group");

TEST(SPTK_RegularExpression, match_first)
{
    RegularExpression matchFirst("test text", "g");
    auto matches = matchFirst.m(testPhrase);
    String words;
    for (const auto& match: matches.groups())
    {
        words = match.value;
    }
    EXPECT_STREQ(words.c_str(), "test text");
}

TEST(SPTK_RegularExpression, match_first_group)
{
    RegularExpression matchFirst("(test text)", "g");
    auto matches = matchFirst.m(testPhrase);
    String words;
    for (const auto& match: matches.groups())
    {
        words = match.value;
    }
    EXPECT_STREQ(words.c_str(), "test text");
}

TEST(SPTK_RegularExpression, match_many)
{
    RegularExpression matchWord("(\\w+)+", "g");
    auto matches = matchWord.m(testPhrase);
    Strings words;
    for (const auto& match: matches.groups())
    {
        words.push_back(match.value);
    }
    EXPECT_STREQ(words.join("_").c_str(), "This_is_a_test_text_to_verify_rexec_text_data_group");
}

TEST(SPTK_RegularExpression, match)
{
    RegularExpression match1("test.*verify");
    RegularExpression match2("test  .*verify");
    EXPECT_TRUE(match1.matches(testPhrase));
    EXPECT_FALSE(match2.matches(testPhrase));
    EXPECT_TRUE(match1 == String(testPhrase));
    EXPECT_FALSE(match1 != String(testPhrase));
}

TEST(SPTK_RegularExpression, match_global)
{
    RegularExpression match("(te[xs]t) (to verify|data)", "g");

    EXPECT_TRUE(match.matches(testPhrase));

    auto matchedStrings = match.m(testPhrase);

    EXPECT_STREQ(matchedStrings[0].value.c_str(), "text");
    EXPECT_EQ(matchedStrings.groups().size(), size_t(4));
}

TEST(SPTK_RegularExpression, named_groups)
{
    RegularExpression match("(?<aname>[xyz]+) (?<avalue>\\d+) (?<description>\\w+)");

    RegularExpression::Groups matchedStrings;
    auto matchedNamedGroups = match.m("  xyz 1234 test1, xxx 333 test2,\r yyy 333 test3\r\nzzz 555 test4");

    EXPECT_STREQ(matchedNamedGroups["aname"].value.c_str(), "xyz");
    EXPECT_STREQ(matchedNamedGroups["avalue"].value.c_str(), "1234");
    EXPECT_STREQ(matchedNamedGroups["description"].value.c_str(), "test1");
}

TEST(SPTK_RegularExpression, replace)
{
    RegularExpression match1("^(.*)(white).*(rabbit)(.*)");
    EXPECT_STREQ("white crow eats flies over rabbit",
                 match1.s("This is a white rabbit", "\\2 crow eats flies over \\3").c_str());
}

TEST(SPTK_RegularExpression, replaceAll)
{
    map<String, String> substitutions = {
        {"$NAME", "John Doe"},
        {"$CITY", "London"},
        {"$YEAR", "2000"}};

    RegularExpression matchPlaceholders("\\$[A-Z]+", "g");
    String text = "$NAME was in $CITY in $YEAR ";
    bool replaced(false);
    String result = matchPlaceholders.replaceAll(text, substitutions, replaced);
    EXPECT_STREQ("John Doe was in London in 2000 ", result.c_str());
}

TEST(SPTK_RegularExpression, lambdaReplace)
{
    map<String, String> substitutions = {
        {"$NAME", "John Doe"},
        {"$CITY", "London"},
        {"$YEAR", "2000"}};

    RegularExpression matchPlaceholders("\\$[A-Z]+", "g");
    String text = "$NAME was in $CITY in $YEAR ";
    bool replaced(false);
    String result = matchPlaceholders.s(
        text, [&substitutions](const String& match) {
            return substitutions[match];
        },
        replaced);
    EXPECT_STREQ("John Doe was in London in 2000 ", result.c_str());
}

TEST(SPTK_RegularExpression, extract)
{
    RegularExpression match1("^(.*)(text).*(verify)(.*)");
    auto matchedStrings = match1.m(testPhrase);
    EXPECT_TRUE(matchedStrings);
    EXPECT_EQ(size_t(4), matchedStrings.groups().size());
    EXPECT_STREQ("This is a test ", matchedStrings[0].value.c_str());
    EXPECT_STREQ(" rexec text data group", matchedStrings[3].value.c_str());
}

TEST(SPTK_RegularExpression, split)
{
    RegularExpression match("[\\s]+");
    auto matchedStrings = match.split(testPhrase);
    EXPECT_EQ(size_t(11), matchedStrings.size());
    EXPECT_STREQ("This", matchedStrings[0].c_str());
    EXPECT_STREQ("text", matchedStrings[8].c_str());
}

TEST(SPTK_RegularExpression, match_performance)
{
    String data("red=#FF0000, green=#00FF00, blue=#0000FF");
    RegularExpression match("((\\w+)=(#\\w+))");
    constexpr size_t maxIterations = 100000;
    StopWatch stopWatch;
    stopWatch.start();
    for (size_t i = 0; i < maxIterations; ++i)
    {
        String s(data);
        while (auto matches = match.m(s))
        {
            s = s.substr(matches[0].value.length());
            auto groupCount = matches.groups().size();
            EXPECT_EQ(groupCount, 3U);
        }
    }
    stopWatch.stop();
    constexpr double oneThousand = 1000;
    COUT("SPTK: " << maxIterations << " regular expressions executed for " << stopWatch.seconds() << " seconds, "
                  << fixed << setprecision(1) << maxIterations / stopWatch.seconds() / oneThousand << "K/sec" << endl)
}

TEST(SPTK_RegularExpression, std_match_performance)
{
    String data("red=#FF0000, green=#00FF00, blue=#0000FF");
    std::regex match("(\\w+)=(#\\w+)");
    constexpr size_t maxIterations = 100000;
    StopWatch stopWatch;
    stopWatch.start();
    for (size_t i = 0; i < maxIterations; ++i)
    {
        String s(data);
        std::smatch color_matches;
        while (std::regex_search(s, color_matches, match))
        {
            s = color_matches.suffix().str();
            auto groupCount = color_matches.size();
            EXPECT_EQ(groupCount, 3U);
        }
    }
    stopWatch.stop();
    constexpr double oneThousand = 1000;
    COUT("STD: " << maxIterations << " regular expressions executed for " << stopWatch.seconds() << " seconds, "
                 << fixed << setprecision(1) << maxIterations / stopWatch.seconds() / oneThousand << "K/sec" << endl)
}

TEST(SPTK_RegularExpression, asyncExec)
{
    RegularExpression match("(?<aname>[xyz]+) (?<avalue>\\d+) (?<description>\\w+)");

    mutex amutex;
    queue<future<size_t>> states;

    constexpr size_t maxThreads = 10;
    for (size_t n = 0; n < maxThreads; ++n)
    {
        future<size_t> f = async(launch::async, [&match]() {
            RegularExpression::Groups matchedStrings;
            auto matchedNamedGroups = match.m("  xyz 1234 test1, xxx 333 test2,\r yyy 333 test3\r\nzzz 555 test4");
            return matchedNamedGroups.namedGroups().size();
        });
        scoped_lock lock(amutex);
        states.push(move(f));
    }

    future<size_t> f;
    auto statesCount = states.size();
    for (size_t n = 0; n < maxThreads;)
    {
        bool gotOne {false};
        if (statesCount > 0)
        {
            scoped_lock lock(amutex);
            if (!states.empty())
            {
                f = move(states.front());
                states.pop();
                ++n;
                gotOne = true;
            }
        }

        constexpr chrono::milliseconds tenMilliseconds(10);
        if (gotOne)
        {
            f.wait();
        }
        else
        {
            this_thread::sleep_for(tenMilliseconds);
        }
    }
}

#endif

#endif
