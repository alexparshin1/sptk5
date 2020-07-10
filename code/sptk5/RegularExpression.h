/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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
#include <atomic>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

class MatchData;

/**
 * PCRE-type regular expressions
 */
class SP_EXPORT RegularExpression
{
public:

    /**
     * Matched group that includes string value, as well as start and end positions of the value in the subject string
     */
    class SP_EXPORT Group
    {
    public:
        /**
         * Constructor
         * @param value         Matched string
         * @param start         String start position in subject
         * @param end           String end position in subject
         */
        Group(String value, size_t start, size_t end)
        : value(move(value)), start((pcre_offset_t)start), end((pcre_offset_t)end)
        {}

        /**
         * Default constructor
         */
        Group() = default;

        /**
         * Copy constructor
         * @param other         Object to copy from
         */
        Group(const Group& other) = default;

        /**
         * Move constructor
         * @param other         Object to copy from
         */
        Group(Group&& other) noexcept;

        /**
         * Copy assignment
         * @param other         Object to copy from
         */
        Group& operator = (const Group& other);

        /**
         * Move assignment
         * @param other         Object to copy from
         */
        Group& operator = (Group&& other);

        String        value;        ///< Matched fragment of subject
        pcre_offset_t start {0};    ///< Start position of the matched fragment in subject
        pcre_offset_t end {0};      ///< End position of the matched fragment in subject
    };

    /**
     * Matched groups, including unnamed and named groups (if any).
     * For named groups in global match, only the first match is considered.
     */
    class SP_EXPORT Groups
    {
        friend class RegularExpression;
    public:
        /**
         * Default constructor
         */
        Groups() = default;

        /**
         * Copy constructor
         * @param other         Other object to copy from
         */
        Groups(const Groups& other) = default;

        /**
         * Move constructor
         * @param other         Other object to move from
         */
        Groups(Groups&& other);

        /**
         * Copy assignment
         * @param other         Other object to move from
         */
        Groups& operator = (const Groups& other) = default;

        /**
         * Move assignment
         * @param other         Other object to move from
         */
        Groups& operator = (Groups&& other) = default;

        /**
         * Get unnamed group by index.
         * If group doesn't exist, return reference to empty group.
         * @param index         Group index, 0-based
         * @return const reference to a group
         */
        const Group& operator[] (size_t index) const;

        /**
         * Get named group by capture group name.
         * If group doesn't exist, return reference to empty group.
         * @param name          Group name
         * @return const reference to a group
         */
        const Group& operator[] (const char* name) const;

        /**
         * Get unnamed groups.
         * @return const reference to unnamed groups object
         */
        const std::vector<Group>& groups() const { return m_groups; }

        /**
         * Get named groups.
         * @return const reference to named groups object
         */
        const std::map<String, Group>& namedGroups() const { return m_namedGroups; }

        /**
         * @return true if there are no matched groups
         */
        bool empty() const { return m_groups.empty(); }

        /**
         * @return false if there are no matched groups
         */
        operator bool () const { return !m_groups.empty(); }

    protected:
        /**
         * Reserve more groups memory
         * @param groupCount    Number of groups to reserve more memory for
         */
        void grow(size_t groupCount);

        /**
         * Add new group by moving it to unnamed groups
         * @param group         Group to add
         */
        void add(Group&& group) { m_groups.push_back(std::move(group)); }

        /**
         * Add new group by moving it to named groups
         * @param name          Group name
         * @param group         Group to add
         */
        void add(const String& name, Group&& group) { m_namedGroups[name] = std::move(group); }

    private:

        /**
         * Clear groups
         */
        void clear();

        std::vector<Group>      m_groups;           ///< Unnamed groups
        std::map<String, Group> m_namedGroups;      ///< Named groups
        static const Group      emptyGroup;         ///< Empty group to return if group can't be found
    };

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

    String                  m_pattern;              ///< Match pattern
    bool                    m_global {false};       ///< Global match (g) or first match only
    String                  m_error;                ///< Last pattern error (if any)

#if HAVE_PCRE2
    pcre2_code*             m_pcre {nullptr};       ///< Compiled PCRE expression handle
    void*                   m_pcreExtra {nullptr};  ///< Dummy
#else
    pcre*                   m_pcre {nullptr};       ///< Compiled PCRE expression handle
    pcre_extra*             m_pcreExtra {nullptr};  ///< Compiled PCRE expression optimization (for faster execution)
#endif

    int32_t                 m_options {0};          ///< PCRE pattern options
    std::atomic<size_t>     m_captureCount {0};     ///< RE' capture count

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

    /**
     * Get capture group count from the compiled pattern
     * @return capture group count
     */
    size_t getCaptureCount() const;

    /**
     * Get named capture group count from the compiled pattern
     * @return named capture group count
     */
    size_t getNamedGroupCount() const;

    /**
     * Get captur group name table from the compiled pattern
     * @return named capture group count
     */
    void getNameTable(const char*& nameTable, int& nameEntrySize) const;

    /**
     * Find next placeholder
     * @param pos               Start position
     * @param outputPattern     Output pattern
     * @return placeholder position
     */
    size_t findNextPlaceholder(size_t pos, const String& outputPattern) const;

};

/**
 * @}
 */
}

#endif

#endif
