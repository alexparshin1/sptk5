/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <sptk5/Strings.h>
#include <sptk5/sptk-config.h>

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#ifdef HAVE_PCRE2
#define PCRE2_STATIC
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

#define SPRE_CASELESS PCRE2_CASELESS
#define SPRE_MULTILINE PCRE2_MULTILINE
#define SPRE_DOTALL PCRE2_DOTALL
#define SPRE_EXTENDED PCRE2_EXTENDED
using pcre_offset_t = long;
#endif

#ifdef HAVE_PCRE

#include <pcre.h>

#define SPRE_CASELESS PCRE_CASELESS
#define SPRE_MULTILINE PCRE_MULTILINE
#define SPRE_DOTALL PCRE_DOTALL
#define SPRE_EXTENDED PCRE_EXTENDED
using pcre_offset_t = int;
#endif

#if defined(HAVE_PCRE2) | defined(HAVE_PCRE)

namespace sptk {

/**
 * @addtogroup utility Utility Classes.
 * @{.
 */

class MatchData;

/**
 * PCRE-type regular expressions
 */
class SP_EXPORT RegularExpression
{

#ifdef HAVE_PCRE2
    using PCREHandle = pcre2_code;    ///< Compiled PCRE expression handle
    using PCREExtraHandle = uint8_t*; ///< Not used
#else
    using PCREHandle = pcre;            ///< Compiled PCRE expression handle
    using PCREExtraHandle = pcre_extra; ///< Compiled PCRE expression optimization (for faster execution)
#endif

public:
    /**
     * Matched group that includes string value, as well as start and end positions of the value in the subject string
     */
    class SP_EXPORT Group
    {
    public:
        /**
         * Constructor
         * @param text          Matched string.
         * @param start         String start position in the subject.
         * @param end           String end position in the subject.
         */
        Group(const char* text, pcre_offset_t start, pcre_offset_t end)
            : value(text + start, static_cast<size_t>(end - start))
            , start(start)
            , end(end)
        {
        }

        /**
         * Default constructor
         */
        Group() = default;

#ifdef _WIN32
        [[noreturn]]
        Group(const Group&)
        {
            throw Exception("Copy ctor isn't supported for Group");
        }

        [[noreturn]]
        Group& operator=(const Group&)
        {
            throw Exception("Copy assign isn't supported for Group");
        }
#else
        Group(const Group&) = delete;
        Group& operator=(const Group&) = delete;
#endif
        Group(Group&&) = default;
        Group& operator=(Group&& other) = default;

        String        value;     ///< Matched fragment of the subject
        pcre_offset_t start {0}; ///< Start position of the matched fragment in the subject
        pcre_offset_t end {0};   ///< End position of the matched fragment in the subject
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
         * Get the unnamed group by index.
         * If the group doesn't exist, return the reference to an empty group.
         * @param index         Group index, 0-based.
         * @return const reference to a group.
         */
        const Group& operator[](int index) const;

        /**
         * Get named group by capture group name.
         * If the group doesn't exist, return the reference to an empty group.
         * @param name          Group name.
         * @return const reference to a group.
         */
        const Group& operator[](const char* name) const;

        /**
         * Get unnamed groups.
         * @return const reference to the unnamed groups object.
         */
        [[nodiscard]] const std::vector<Group>& groups() const
        {
            return m_groups;
        }

        /**
         * Get named groups.
         * @return const reference to the named groups object.
         */
        [[nodiscard]] const std::map<String, Group>& namedGroups() const
        {
            return m_namedGroups;
        }

        /**
         * @return true if there are no matched groups.
         */
        [[nodiscard]] bool empty() const
        {
            return m_groups.empty();
        }

        /**
         * @return false if there are no matched groups.
         */
        explicit operator bool() const
        {
            return !m_groups.empty();
        }

    protected:
        /**
         * Reserve more groups memory
         * @param groupCount    Number of groups to reserve more memory for.
         */
        void grow(size_t groupCount);

        /**
         * Add the new group by moving it to unnamed groups
         * @param group         Group to add.
         */
        void add(Group&& group)
        {
            m_groups.push_back(std::move(group));
        }

        /**
         * Add the new group by moving it to named groups
         * @param name          Group name.
         * @param group         Group to add.
         */
        void add(const String& name, Group&& group)
        {
            m_namedGroups[name] = std::move(group);
        }

    private:
        std::vector<Group>      m_groups;      ///< Unnamed groups
        std::map<String, Group> m_namedGroups; ///< Named groups
        static const Group      emptyGroup;    ///< Empty group to return if the group can't be found
    };

    /**
     * @brief Constructor
     *
     * Pattern options are a combination of flags matching Perl regular expression switches:
     * 'g'  global match, not just the first one.
     * 'i'  letters in the pattern match both upper and lower case  letters.
     * 'm'  multiple lines match.
     * 's'  dot character matches even newlines.
     * 'x'  ignore whitespaces.
     * @param pattern           PCRE pattern.
     * @param options           Pattern options.
     */
    explicit RegularExpression(std::string_view pattern, std::string_view options = "");

    /**
     * @brief Copy constructor.
     * @param other             Other object.
     */
    RegularExpression(const RegularExpression& other);

    /**
     * @brief Move constructor.
     * @param other             Other object.
     */
    RegularExpression(RegularExpression&& other) noexcept;

    /**
     * @brief Copy assignment.
     * @param other             Other object.
     */
    RegularExpression& operator=(const RegularExpression& other);

    /**
     * @brief Move assignment.
     * @param other             Other object.
     */
    RegularExpression& operator=(RegularExpression&& other) noexcept;

    /**
     * Returns true if the text matches with the regular expression
     * @param text              Input text.
     * @return true if match found.
     */
    bool operator==(const String& text) const;

    /**
     * Returns true if the text matches with the regular expression
     * @param text              Text to process.
     * @return true if match found.
     */
    bool matches(const String& text) const;

    /**
     * Returns the list of strings matched with the regular expression
     * @param text              Text to process.
     * @return matched groups.
     */
    Groups m(const String& text) const
    {
        size_t offset = 0;
        return m(text, offset);
    }

    /**
     * Returns the list of strings matched with the regular expression
     * @param text              Text to process.
     * @param offset            Search offset, updated after method execution.
     * @return matched groups.
     */
    Groups m(const String& text, size_t& offset) const;

    /**
     * Replaces matches with replacement string
     * @param text              Text to process.
     * @param outputPattern     Output pattern using "\\N" as placeholders, with "\\1" as the first match.
     * @return processed text.
     */
    String s(const String& text, const String& outputPattern) const;

    /**
     * Replaces matches with replacement string
     * @param text              Text to process.
     * @param replace           Callback function providing replacement s for matches.
     * @param replaced          True if there were any replacements.
     * @return processed text.
     */
    String s(const String& text, const std::function<String(const String&)>& replace, bool& replaced) const;

    /**
     * Returns the list of strings split by regular expression
     * @param text              Text to process.
     * @return List of strings.
     */
    Strings split(const String& text) const;

    /**
     * Replaces matches with replacement string
     * @param text              Text to process.
     * @param outputPattern     Output pattern using "\\N" as placeholders, with "\\1" as the first match.
     * @param replaced          Optional flag if replacement was made.
     * @return processed text.
     */
    String replaceAll(const String& text, const String& outputPattern, bool& replaced) const;

    /**
     * Replaces matches with replacement string from the map, using matched string as an index
     * @param text              Text to process.
     * @param substitutions     Substitutions for matched strings.
     * @param replaced          Optional flag if replacement was made.
     * @return processed text.
     */
    String replaceAll(const String& text, const std::map<String, String>& substitutions, bool& replaced) const;

    /**
     * Get regular expression pattern
     * @return.
     */
    const String& pattern() const;

private:
    mutable std::mutex               m_mutex;
    String                           m_pattern;          ///< Match pattern
    bool                             m_global {false};   ///< Global match (g) or first match only
    String                           m_error;            ///< Last pattern error (if any)
    std::shared_ptr<PCREHandle>      m_pcre;             ///< Compiled PCRE expression handle
    std::shared_ptr<PCREExtraHandle> m_pcreExtra;        ///< Compiled PCRE expression optimization (for faster execution)
    uint32_t                         m_options {0};      ///< PCRE pattern options
    size_t                           m_captureCount {0}; ///< The capture count

    /**
     * Initialize PCRE expression
     */
    void compile();

    /**
     * Computes match positions and lengths
     * @param text              Input text.
     * @param offset            Starting match offset, advanced with every successful match.
     * @param matchData         Output match positions array.
     * @return number of matches.
     */
    size_t nextMatch(const String& text, size_t& offset, MatchData& matchData) const;

    /**
     * Get capture group count from the compiled pattern
     * @return capture group count.
     */
    size_t getCaptureCount() const;

    /**
     * Get the named capture group count from the compiled pattern
     * @return named capture group count.
     */
    size_t getNamedGroupCount() const;

    /**
     * Get captur group name table from the compiled pattern
     * @return named capture group count.
     */
    void getNameTable(const char*& nameTable, int& nameEntrySize) const;

    /**
     * Find the next placeholder
     * @param pos               Start position.
     * @param outputPattern     Output pattern.
     * @return placeholder position.
     */
    static size_t findNextPlaceholder(size_t pos, const String& outputPattern);

    void      extractNamedMatches(const String& text, Groups& matchedStrings, const MatchData& matchData,
                                  size_t matchCount) const;
    MatchData createMatchData() const;
};

using SRegularExpression = std::shared_ptr<RegularExpression>;

/**
 * @}.
 */
} // namespace sptk

#endif
