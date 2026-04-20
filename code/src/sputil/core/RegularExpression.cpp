/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-17                                             ║
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

#include "sptk5/RegularExpression.h"
#include <future>
#include <regex>
#include <sptk5/cutils>


#if defined(HAVE_PCRE) | defined(HAVE_PCRE2)

using namespace std;
using namespace sptk;

namespace sptk {
struct Match
{
    pcre_offset_t m_start {0}; ///< Match start
    pcre_offset_t m_end {0};   ///< Match end
};

class MatchData
{
    friend class RegularExpression;

public:
#ifdef HAVE_PCRE2
    pcre2_match_data* match_data {nullptr};

    MatchData(const pcre2_code* pcre, size_t _maxMatches)
        : match_data(pcre2_match_data_create_from_pattern(pcre, nullptr))
        , matches(_maxMatches + 2)
        , maxMatches(_maxMatches + 2)
    {
    }

    ~MatchData()
    {
        pcre2_match_data_free(match_data);
    }

#else
    static constexpr int reservedMatches = 4;

    MatchData(const pcre*, size_t maxMatches)
        : matches(maxMatches + static_cast<size_t>(reservedMatches) * 2)
        , maxMatches(maxMatches + reservedMatches)
    {
    }
#endif

    MatchData(const MatchData&) = delete;
    MatchData& operator=(const MatchData&) = delete;

    vector<Match> matches;
    size_t        maxMatches {0};
};

} // namespace sptk

size_t RegularExpression::getCaptureCount() const
{
    auto captureCount = 0;

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

    return static_cast<size_t>(captureCount);
}

const RegularExpression::Group RegularExpression::Groups::emptyGroup;

const RegularExpression::Group& RegularExpression::Groups::operator[](int index) const
{
    if (static_cast<size_t>(index) >= m_groups.size())
    {
        return emptyGroup;
    }
    return m_groups[index];
}

const RegularExpression::Group& RegularExpression::Groups::operator[](const char* name) const
{
    const auto itor = m_namedGroups.find(name);
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
    lock_guard lock(m_mutex);

#ifdef HAVE_PCRE2
    auto       errorNumber {0};
    PCRE2_SIZE errorOffset {0};

    auto* pcre = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR8>(m_pattern.c_str()), // the pattern
        PCRE2_ZERO_TERMINATED,                            // indicates pattern is zero-terminated
        m_options,                                        // options
        &errorNumber,                                     // for error number
        &errorOffset,                                     // for error offset
        nullptr);                                         // use default compile context

    if (pcre == nullptr)
    {
        array<PCRE2_UCHAR, 256> buffer {};
        pcre2_get_error_message(errorNumber, buffer.data(), sizeof(buffer));
        throw Exception(bit_cast<const char*>(buffer.data()));
    }

    m_pcre = shared_ptr<PCREHandle>(pcre,
                                    [](auto* ptr)
                                    {
                                        pcre2_code_free(ptr);
                                    });

#else
    const char* error = nullptr;
    int         errorOffset = 0;

    auto* pcre = pcre_compile(m_pattern.c_str(), static_cast<int>(m_options), &error, &errorOffset, nullptr);
    m_pcre = shared_ptr<PCREHandle>(pcre,
                                    [](auto* pcreHandle)
                                    {
                                        pcre_free(pcreHandle);
                                    });

    if (!m_pcre)
        m_error = "PCRE pattern error at pattern offset " + to_string(errorOffset) + ": " + string(error);
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
                                                      [](pcre_extra* study)
                                                      {
                                                          pcre_free_study(study);
                                                      });
        }
    }
#endif
#endif
    m_captureCount = getCaptureCount();
}

RegularExpression::RegularExpression(std::string_view pattern, std::string_view options)
    : m_pattern(pattern.data(), pattern.size())
{
    for (const auto ch: options)
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

RegularExpression::RegularExpression(const RegularExpression& other)
    : m_pattern(other.m_pattern)
    , m_global(other.m_global)
    , m_options(other.m_options)
{
    compile();
}

RegularExpression::RegularExpression(RegularExpression&& other) noexcept
    : m_pattern(std::move(other.m_pattern))
    , m_global(other.m_global)
    , m_pcre(std::move(other.m_pcre))
    , m_pcreExtra(std::move(other.m_pcreExtra))
    , m_options(other.m_options)
    , m_captureCount(other.m_captureCount)
{
}

RegularExpression& RegularExpression::operator=(const RegularExpression& other)
{
    if (this != &other)
    {
        m_pattern = other.m_pattern;
        m_global = other.m_global;
        m_options = other.m_options;
        compile();
    }
    return *this;
}

RegularExpression& RegularExpression::operator=(RegularExpression&& other) noexcept
{
    if (this != &other)
    {
        m_pattern = std::move(other.m_pattern);
        m_global = other.m_global;
        m_pcre = std::move(other.m_pcre);
        m_pcreExtra = std::move(other.m_pcreExtra);
        m_options = other.m_options;
        m_captureCount = other.m_captureCount;
    }
    return *this;
}

size_t RegularExpression::nextMatch(const string& text, size_t& offset, MatchData& matchData) const
{
    lock_guard lock(m_mutex);

    if (!m_pcre)
    {
        throw Exception(m_error);
    }

#ifdef HAVE_PCRE2

    const auto rc = pcre2_match(
        m_pcre.get(),                                // the compiled pattern
        reinterpret_cast<PCRE2_SPTR8>(text.c_str()), // the subject string
        text.length(),                               // the length of the subject
        offset,                                      // start at offset in the subject
        0,                                           // default options
        matchData.match_data,                        // block for storing the result
        nullptr);                                    // use default match context

    if (rc >= 0)
    {
        const auto* offsetVector = pcre2_get_ovector_pointer(matchData.match_data);
        const auto* offsetsEnd = offsetVector + static_cast<size_t>(2 * rc);
        matchData.matches.reserve(rc);
        matchData.matches.clear();
        for (auto* offsetPair = offsetVector; offsetPair != offsetsEnd; offsetPair += 2)
        {
            auto start = static_cast<pcre_offset_t>(*offsetPair);
            auto end = static_cast<pcre_offset_t>(*(offsetPair + 1));
            matchData.matches.emplace_back(start, end);
        }
        auto nextOffset = offsetVector[1];
        if (m_global && offsetVector[0] == offsetVector[1])
        {
            nextOffset = (nextOffset < text.length()) ? nextOffset + 1 : 0;
        }
        offset = nextOffset;
        return static_cast<size_t>(rc); // match count
    }

    if (rc == PCRE2_ERROR_NOMATCH)
    {
        if (m_options == 0)
        {
            return false;
        } /* All matches found */
        ++offset; /* Advance one code unit */
        return false;
    }

    switch (rc)
    {
        case PCRE2_ERROR_NULL:
            throw Exception("Null argument");
        case PCRE2_ERROR_BADOPTION:
            throw Exception("Invalid regular expression option");
        case PCRE2_ERROR_BADMAGIC:
            throw Exception("Invalid compiled regular expression\n");
        case PCRE2_ERROR_NOMEMORY:
            throw Exception("Out of memory");
        default:
            throw Exception(format("Unknown PCRE2 error: {}", rc));
    }
#else
    int rc = pcre_exec(
        m_pcre.get(), m_pcreExtra.get(), text.c_str(), static_cast<int>(text.length()), static_cast<int>(offset), 0,
        reinterpret_cast<pcre_offset_t*>(matchData.matches.data()),
        static_cast<pcre_offset_t>(matchData.maxMatches) * 2);

    if (rc == PCRE_ERROR_NOMATCH)
        return 0;

    if (rc < 0)
    {
        switch (rc)
        {
            case PCRE_ERROR_NULL:
                throw Exception("Null argument");
            case PCRE_ERROR_BADOPTION:
                throw Exception("Invalid regular expression option");
            case PCRE_ERROR_BADMAGIC:
            case PCRE_ERROR_UNKNOWN_NODE:
                throw Exception("Invalid compiled regular expression\n");
            case PCRE_ERROR_NOMEMORY:
                throw Exception("Out of memory");
            default:
                throw Exception("Unknown error");
        }
    }

    const int matchCount = rc; // If the match count is zero - there are too many matches

    auto nextOffset = static_cast<size_t>(matchData.matches[0].m_end);
    if (m_global && matchData.matches[0].m_start == matchData.matches[0].m_end)
    {
        nextOffset = (nextOffset < text.length()) ? nextOffset + 1 : 0;
    }
    offset = nextOffset;
    return static_cast<size_t>(matchCount);
#endif
}

MatchData RegularExpression::createMatchData() const
{
    lock_guard lock(m_mutex);
    return {m_pcre.get(), m_captureCount};
}

bool RegularExpression::operator==(const string& text) const
{
    size_t offset = 0;
    auto   matchData = createMatchData();
    return nextMatch(text, offset, matchData) > 0;
}

bool RegularExpression::matches(const string& text) const
{
    size_t     offset = 0;
    auto       matchData = createMatchData();
    const auto matchCount = nextMatch(text, offset, matchData);
    return matchCount > 0;
}

RegularExpression::Groups RegularExpression::m(const string& text, size_t& offset) const
{
    Groups matchedStrings;
    auto   matchData = createMatchData();

    auto first {true};
    do
    {
        const auto matchCount = nextMatch(text, offset, matchData);
        if (matchCount == 0 || offset == 0)
        { // No matches
            break;
        }
        if (matchData.matches[0].m_start == matchData.matches[0].m_end)
        {
            continue;
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
                Group group(text.c_str(), match.m_start, match.m_end);
                matchedStrings.add(std::move(group));
            }
            else
            {
                Group group;
                matchedStrings.add(std::move(group));
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

void RegularExpression::extractNamedMatches(const string& text, Groups& matchedStrings,
                                            const MatchData& matchData, size_t matchCount) const
{
    if (const auto nameCount = static_cast<int>(getNamedGroupCount());
        nameCount > 0)
    {
        const char* nameTable = nullptr;
        auto        nameEntrySize = 0;
        getNameTable(nameTable, nameEntrySize);
        const auto* tabptr = nameTable;
        for (auto i = 0; i < nameCount; ++i)
        {
            const auto n = static_cast<size_t>((static_cast<int>(tabptr[0]) << 8) | static_cast<int>(tabptr[1]));
            string     name(tabptr + 2, static_cast<size_t>(nameEntrySize - 3));
            auto       nameLength = strlen(name.c_str());
            if (nameLength < name.size())
            {
                name.resize(nameLength);
            }
            if (n < matchCount)
            {
                if (const auto& match = matchData.matches[n]; match.m_start >= 0)
                {
                    Group group(text.c_str(), match.m_start, match.m_end);
                    matchedStrings.add(name, std::move(group));
                    tabptr += nameEntrySize;
                    continue;
                }
            }

            Group group;
            matchedStrings.add(name, std::move(group));
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
    auto nameCount = 0;

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

    return static_cast<size_t>(nameCount);
}

Strings RegularExpression::split(const string& text) const
{
    Strings matchedStrings;

    size_t    offset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);

    pcre_offset_t lastMatchEnd = 0;
    do
    {
        if (const auto matchCount = nextMatch(text, offset, matchData);
            matchCount == 0)
        { // No matches
            break;
        }
        if (matchData.matches[0].m_start == matchData.matches[0].m_end)
        {
            continue;
        }

        const Match& match = matchData.matches[0];
        matchedStrings.push_back(string(text.c_str() + lastMatchEnd, static_cast<size_t>(match.m_start - lastMatchEnd)));
        lastMatchEnd = match.m_end;

    } while (offset);

    matchedStrings.push_back(string(text.c_str() + lastMatchEnd));

    return matchedStrings;
}

string RegularExpression::replaceAll(const string& text, const string& outputPattern, bool& replaced) const
{
    size_t    offset = 0;
    size_t    lastOffset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);
    string    result;

    replaced = false;

    do
    {
        const auto fragmentOffset = offset;
        const auto matchCount = nextMatch(text, offset, matchData);
        if (matchCount == 0)
        { // No matches
            break;
        }
        if (matchData.matches[0].m_start == matchData.matches[0].m_end)
        {
            continue;
        }
        if (offset)
        {
            lastOffset = offset;
        }

        // Create the next replacement
        size_t pos = 0;
        string nextReplacement;
        replaced = true;
        while (pos != string::npos)
        {
            auto placeHolderStart = findNextPlaceholder(pos, outputPattern);

            if (placeHolderStart == string::npos)
            {
                nextReplacement += outputPattern.substr(pos);
                break;
            }

            nextReplacement += outputPattern.substr(pos, placeHolderStart - pos);
            ++placeHolderStart;
            const auto placeHolderIndex = static_cast<size_t>(string2int(outputPattern.c_str() + placeHolderStart));
            const auto placeHolderEnd = outputPattern.find_first_not_of("0123456789", placeHolderStart);
            if (placeHolderIndex < matchCount)
            {
                const Match& match = matchData.matches[placeHolderIndex];
                const char*  matchPtr = text.c_str() + match.m_start;
                nextReplacement += string(matchPtr, static_cast<size_t>(match.m_end) - static_cast<size_t>(match.m_start));
            }
            pos = placeHolderEnd;
        }

        // Append text from fragment start to match start
        if (const auto fragmentStartLength = static_cast<size_t>(matchData.matches[0].m_start) - fragmentOffset;
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

string RegularExpression::s(const string& text, const std::function<string(const string&)>& replace,
                            bool& replaced) const
{
    size_t    offset = 0;
    size_t    lastOffset = 0;
    MatchData matchData(m_pcre.get(), m_captureCount);
    string    result;

    replaced = false;

    do
    {
        const auto fragmentOffset = offset;
        if (const auto matchCount = nextMatch(text, offset, matchData);
            matchCount == 0)
        {
            break;
        } // No matches
        if (matchData.matches[0].m_start == matchData.matches[0].m_end)
        {
            continue;
        }
        if (offset)
        {
            lastOffset = offset;
        }

        replaced = true;

        // Append text from fragment start to match start
        if (const auto fragmentStartLength = static_cast<size_t>(matchData.matches[0].m_start) - fragmentOffset;
            fragmentStartLength != 0)
        {
            result += text.substr(fragmentOffset, fragmentStartLength);
        }

        // Append replacement
        const string currentMatch(text.c_str() + matchData.matches[0].m_start,
                                  static_cast<unsigned>(matchData.matches[0].m_end) - static_cast<unsigned>(matchData.matches[0].m_start));

        const string nextReplacement = replace(currentMatch);

        result += nextReplacement;

    } while (offset);

    return result + text.substr(lastOffset);
}

size_t RegularExpression::findNextPlaceholder(size_t pos, const string& outputPattern)
{
    auto placeHolderStart = pos;
    for (;; ++placeHolderStart)
    {
        placeHolderStart = outputPattern.find('\\', placeHolderStart);
        if (placeHolderStart == outputPattern.size() - 1 ||
            placeHolderStart == string::npos ||
            isdigit(outputPattern[placeHolderStart + 1]))
        {
            break;
        }
    }
    return placeHolderStart;
}

string RegularExpression::replaceAll(const string& text, const SubstitutionMap& substitutions, bool& replaced) const
{
    // For the "i" option, make the lowercase match map
    SubstitutionMap substitutionsMap;
    const auto      ignoreCase = (m_options & SPRE_CASELESS) == SPRE_CASELESS;
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
        text, [&substitutionsMap, ignoreCase](const string& needle)
        {
            const auto itor = substitutionsMap.find(ignoreCase ? lowerCase(needle) : needle);
            if (itor == substitutionsMap.end())
            {
                return needle;
            }
            return itor->second;
        },
        replaced);
}

string RegularExpression::s(const string& text, const string& outputPattern) const
{
    auto replaced = false;
    return replaceAll(text, outputPattern, replaced);
}

const string& RegularExpression::pattern() const
{
    return m_pattern;
}

#endif
