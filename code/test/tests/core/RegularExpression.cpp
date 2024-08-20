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

#include <future>
#include <regex>
#include <sptk5/cutils>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;

#if defined(HAVE_PCRE) | defined(HAVE_PCRE2)

static const String testPhrase("This is a test text to verify rexec text data group");

TEST(SPTK_RegularExpression, match_first)
{
    const RegularExpression matchFirst("test text", "g");
    const auto matches = matchFirst.m(testPhrase);
    String                  words;
    for (const auto& match: matches.groups())
    {
        words = match.value;
    }
    EXPECT_STREQ(words.c_str(), "test text");
}

TEST(SPTK_RegularExpression, match_first_group)
{
    const RegularExpression matchFirst("(test text)", "g");
    const auto matches = matchFirst.m(testPhrase);
    String words;
    for (const auto& match: matches.groups())
    {
        words = match.value;
    }
    EXPECT_STREQ(words.c_str(), "test text");
}

TEST(SPTK_RegularExpression, match_many)
{
    const RegularExpression matchWord("(\\w+)+", "g");
    const auto matches = matchWord.m(testPhrase);
    Strings words;
    for (const auto& match: matches.groups())
    {
        words.push_back(match.value);
    }
    EXPECT_STREQ(words.join("_").c_str(), "This_is_a_test_text_to_verify_rexec_text_data_group");
}

TEST(SPTK_RegularExpression, match)
{
    const RegularExpression match1("test.*verify");
    const RegularExpression match2("test  .*verify");
    EXPECT_TRUE(match1.matches(testPhrase));
    EXPECT_FALSE(match2.matches(testPhrase));
    EXPECT_TRUE(match1 == String(testPhrase));
    EXPECT_FALSE(match1 != String(testPhrase));
}

TEST(SPTK_RegularExpression, match_global)
{
    const RegularExpression match("(te[xs]t) (to verify|data)", "g");

    EXPECT_TRUE(match.matches(testPhrase));

    const auto matchedStrings = match.m(testPhrase);

    EXPECT_STREQ(matchedStrings[0].value.c_str(), "text");
    EXPECT_EQ(matchedStrings.groups().size(), static_cast<size_t>(4));
}

TEST(SPTK_RegularExpression, named_groups)
{
    const RegularExpression match("(?<aname>[xyz]+) (?<avalue>\\d+) (?<description>\\w+)");

    RegularExpression::Groups matchedStrings;
    const auto matchedNamedGroups = match.m("  xyz 1234 test1, xxx 333 test2,\r yyy 333 test3\r\nzzz 555 test4");

    EXPECT_STREQ(matchedNamedGroups["aname"].value.c_str(), "xyz");
    EXPECT_STREQ(matchedNamedGroups["avalue"].value.c_str(), "1234");
    EXPECT_STREQ(matchedNamedGroups["description"].value.c_str(), "test1");
}

TEST(SPTK_RegularExpression, replace)
{
    const RegularExpression match1("^(.*)(white).*(rabbit)(.*)");
    EXPECT_STREQ("white crow eats flies over rabbit",
                 match1.s("This is a white rabbit", "\\2 crow eats flies over \\3").c_str());
}

TEST(SPTK_RegularExpression, replaceAll)
{
    const map<String, String> substitutions = {
        {"$NAME", "John Doe"},
        {"$CITY", "London"},
        {"$YEAR", "2000"}};

    const RegularExpression matchPlaceholders("\\$[A-Z]+", "g");
    const String            text = "$NAME was in $CITY in $YEAR ";
    bool replaced(false);
    const String result = matchPlaceholders.replaceAll(text, substitutions, replaced);
    EXPECT_STREQ("John Doe was in London in 2000 ", result.c_str());
}

TEST(SPTK_RegularExpression, lambdaReplace)
{
    map<String, String> substitutions = {
        {"$NAME", "John Doe"},
        {"$CITY", "London"},
        {"$YEAR", "2000"}};

    const RegularExpression matchPlaceholders("\\$[A-Z]+", "g");
    const String text = "$NAME was in $CITY in $YEAR ";
    bool replaced(false);
    const String result = matchPlaceholders.s(
        text, [&substitutions](const String& match) {
            return substitutions[match];
        },
        replaced);
    EXPECT_STREQ("John Doe was in London in 2000 ", result.c_str());
}

TEST(SPTK_RegularExpression, extract)
{
    const RegularExpression match1("^(.*)(text).*(verify)(.*)");
    const auto matchedStrings = match1.m(testPhrase);
    EXPECT_TRUE(matchedStrings);
    EXPECT_EQ(static_cast<size_t>(4), matchedStrings.groups().size());
    EXPECT_STREQ("This is a test ", matchedStrings[0].value.c_str());
    EXPECT_STREQ(" rexec text data group", matchedStrings[3].value.c_str());
}

TEST(SPTK_RegularExpression, split)
{
    const RegularExpression match("[\\s]+");
    auto matchedStrings = match.split(testPhrase);
    EXPECT_EQ(static_cast<size_t>(11), matchedStrings.size());
    EXPECT_STREQ("This", matchedStrings[0].c_str());
    EXPECT_STREQ("text", matchedStrings[8].c_str());
}

TEST(SPTK_RegularExpression, match_performance)
{
    const String            data("red=#FF0000, green=#00FF00, blue=#0000FF");
    const RegularExpression match("((\\w+)=(#\\w+))");
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
                  << fixed << setprecision(1) << maxIterations / stopWatch.seconds() / oneThousand << "K/sec" << endl);
}

TEST(SPTK_RegularExpression, std_match_performance)
{
    const String data("red=#FF0000, green=#00FF00, blue=#0000FF");
    const std::regex match("(\\w+)=(#\\w+)");
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
                 << fixed << setprecision(1) << maxIterations / stopWatch.seconds() / oneThousand << "K/sec" << endl);
}

TEST(SPTK_RegularExpression, asyncExec)
{
    RegularExpression match("(?<aname>[xyz]+) (?<avalue>\\d+) (?<description>\\w+)");

    mutex amutex;
    deque<future<size_t>> states;

    constexpr size_t maxThreads = 10;
    for (size_t n = 0; n < maxThreads; ++n)
    {
        future<size_t> f = async(launch::async, [&match]() {
            RegularExpression::Groups matchedStrings;
            const auto matchedNamedGroups = match.m("  xyz 1234 test1, xxx 333 test2,\r yyy 333 test3\r\nzzz 555 test4");
            return matchedNamedGroups.namedGroups().size();
        });
        scoped_lock lock(amutex);
        states.push_back(std::move(f));
    }

    future<size_t> f;
    const auto statesCount = states.size();
    for (size_t n = 0; n < maxThreads;)
    {
        bool gotOne {false};
        if (statesCount > 0)
        {
            scoped_lock lock(amutex);
            if (!states.empty())
            {
                f = std::move(states.front());
                states.pop_front();
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
