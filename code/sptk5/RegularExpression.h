/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       RegularExpression.h - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __SPTK_REGULAR_EXPRESSION_H__
#define __SPTK_REGULAR_EXPRESSION_H__

#include <sptk5/sptk.h>
#include <sptk5/sptk-config.h>
#include <sptk5/Strings.h>

#if HAVE_PCRE2
#define PCRE2_STATIC
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#define SPRE_CASELESS   PCRE2_CASELESS
#define SPRE_MULTILINE  PCRE2_MULTILINE
#define SPRE_DOTALL     PCRE2_DOTALL
#define SPRE_EXTENDED   PCRE2_EXTENDED
#define pcre_offset_t   long
#endif

#include <pcre.h>

#if HAVE_PCRE
#include <pcre.h>
#define SPRE_CASELESS   PCRE_CASELESS
#define SPRE_MULTILINE  PCRE_MULTILINE
#define SPRE_DOTALL     PCRE_DOTALL
#define SPRE_EXTENDED   PCRE_EXTENDED
#define pcre_offset_t   int
#endif

#if (HAVE_PCRE2 | HAVE_PCRE)

#include <vector>
#include <functional>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * PCRE-type regular expressions
 */
class SP_EXPORT RegularExpression
{
public:

    class Group
    {
    public:
        Group(String value, size_t start_position, size_t end_position)
        : value(move(value)), start_position(start_position), end_position(end_position)
        {}

        Group() = default;
        Group(const Group& other) = default;

        String        value;
        pcre_offset_t start_position {0};
        pcre_offset_t end_position {0};
    };

    class Groups
    {
    public:
        Groups() = default;
        Groups(const Groups& other) = default;
        Groups(Groups&& other);

        const Group& operator[] (size_t index) const;
        const Group& operator[] (const String& name) const;

        const std::vector<Group>&      groups() const { return m_groups; }
        const std::map<String, Group>& namedGroups() const { return m_namedGroups; }

        void add(const Group& group) { m_groups.push_back(group); }
        void add(const String& name, const Group& group) { m_namedGroups[name] = group; }

        bool empty() const { return m_groups.empty(); }
        operator bool () const { return !m_groups.empty(); }

    private:
        std::vector<Group>      m_groups;
        std::map<String, Group> m_namedGroups;
        static const Group emptyGroup;
    };

private:
    /**
     * Match position information
     */
    typedef struct {
        pcre_offset_t   m_start;                    ///< Match start
        pcre_offset_t   m_end;                      ///< Match end
    } Match;

    typedef std::vector<Match> Matches;             ///< Vector of match positions

    String                  m_pattern;              ///< Match pattern
    bool                    m_global {false};       ///< Global match (g) or first match only
    String                  m_error;                ///< Last pattern error (if any)

#if HAVE_PCRE2
    pcre2_code*             m_pcre {nullptr};
#else
    pcre*                   m_pcre {nullptr};       ///< Compiled PCRE expression handle
    pcre_extra*             m_pcreExtra {nullptr};  ///< Compiled PCRE expression optimization (for faster execution)
#endif

    int32_t                 m_options {0};          ///< PCRE pattern options

    static constexpr int MAX_MATCHES = 128;

    class MatchData
    {
    public:
#if HAVE_PCRE2
        MatchData(pcre2_code* pcre)
        : matches(128),
          match_data(pcre2_match_data_create_from_pattern(pcre, nullptr))
        {}

        ~MatchData()
        {
            if (match_data)
                pcre2_match_data_free(match_data);
        }
#else
        MatchData() : matches(128) {}
#endif

        Matches             matches;
        pcre2_match_data*   match_data {nullptr};
    };


    /**
     * Initialize PCRE expression
     */
    void compile();

    /**
     * Computes match positions and lengths
     * @param text              Input text
     * @param offset            Starting match offset, advanced with every successful match
     * @param matchDdata        Output match positions array
     * @return number of matches
     */
    size_t nextMatch(const String& text, size_t& offset, MatchData& matchData) const;

public:
    /**
     * Constructor
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
    explicit RegularExpression(String pattern, const String& options = "");

    /**
     * Copy constructor
     * @param other             Other regular expression
     */
    RegularExpression(const RegularExpression& other);

    /**
     * Destructor
     */
    virtual ~RegularExpression();

    /**
     * Returns true if text matches with regular expression
     * @param text              Input text
     * @return true if match found
     */
    bool operator ==(const String& text) const;

    /**
     * Returns true if text doesn't match with regular expression
     * @param text              Input text
     * @return true if match found
     */
    bool operator !=(const String& text) const;

    /**
     * Returns true if text matches with regular expression
     * @param text              Text to process
     * @return true if match found
     */
    bool matches(const String& text) const;

    /**
     * Returns list of strings matched with regular expression
     * @param text              Text to process
     * @return matched groups
     */
    Groups m(const String& text) const;

    /**
     * Replaces matches with replacement string
     * @param text              Text to process
     * @param outputPattern     Output pattern using "\\N" as placeholders, with "\\1" as first match
     * @return processed text
     */
    String s(const String& text, const String& outputPattern) const;

    /**
     * Replaces matches with replacement string
     * @param text              Text to process
     * @param replace           Callback function providing replacement s for matches
     * @param replaced          True if there were any replacements
     * @return processed text
     */
    String s(const String& text, std::function<String(const String&)> replace, bool& replaced) const;

    /**
     * Returns list of strings split by regular expression
     * @param text              Text to process
     * @return List of strings
     */
    Strings split(const String& text) const;

    /**
     * Replaces matches with replacement string
     * @param text              Text to process
     * @param outputPattern     Output pattern using "\\N" as placeholders, with "\\1" as first match
     * @param replaced          Optional flag if replacement was made
     * @return processed text
     */
    String replaceAll(const String& text, const String& outputPattern, bool& replaced) const;

    /**
     * Replaces matches with replacement string from map, using matched string as an index
     * @param text              Text to process
     * @param substitutions     Substitutions for matched strings
     * @param replaced          Optional flag if replacement was made
     * @return processed text
     */
    String replaceAll(const String& text, const std::map<String,String>& substitutions, bool& replaced) const;

    /**
     * Get regular expression pattern
     * @return 
     */
    const String& pattern() const;

private:

    /**
     * Find next placeholder
     * @param pos               Start position
     * @param outputPattern     Output pattern
     * @return placeholder position
     */
    size_t findNextPlaceholder(size_t pos, const String& outputPattern) const;

    int getNamedGroupCount() const;

    void getNameTable(const char*& nameTable, int& nameEntrySize) const;
};

/**
 * @}
 */
}

#endif

#endif
