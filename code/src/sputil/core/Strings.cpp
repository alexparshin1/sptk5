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

#include <sstream>
#include <sptk5/Strings.h>
#include <sptk5/Buffer.h>
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;

static void splitByDelimiter(Strings& dest, const String& src, const char* delimitter)
{
    dest.clear();
    const auto* pos = src.c_str();
    size_t delimitterLength = strlen(delimitter);
    while (true)
    {
        const auto* end = strstr(pos, delimitter);
        if (end != nullptr)
        {
            dest.emplace_back(pos, size_t(end - pos));
            pos = end + delimitterLength;
        }
        else
        {
            if (*pos != 0)
            {
                dest.emplace_back(pos);
            }
            break;
        }
    }
}

static void splitByAnyChar(Strings& dest, const String& src, const char* delimitter)
{
    dest.clear();
    size_t pos = 0;
    while (pos != string::npos)
    {
        size_t end = src.find_first_of(delimitter, pos);
        if (end != string::npos)
        {
            dest.emplace_back(src.substr(pos, end - pos));
            pos = src.find_first_not_of(delimitter, end + 1);
        }
        else
        {
            if (pos + 1 < src.length())
            {
                dest.emplace_back(src.substr(pos));
            }
            break;
        }
    }
}

static void splitByRegExp(Strings& dest, const String& src, const char* pattern)
{
    RegularExpression regularExpression(pattern);

    dest.clear();
    dest = regularExpression.split(src);
}

Strings::Strings(const String& src, const char* delimiter, SplitMode mode) noexcept
{
    try
    {
        fromString(src.c_str(), delimiter, mode);
    }
    catch (const Exception& e)
    {
        push_back("# ERROR: " + String(e.what()));
    }
}

void Strings::fromString(const String& src, const char* delimitter, SplitMode mode)
{
    clear();
    switch (mode)
    {
        case SplitMode::ANYCHAR:
            splitByAnyChar(*this, src, delimitter);
            break;
        case SplitMode::REGEXP:
            splitByRegExp(*this, src, delimitter);
            break;
        default:
            splitByDelimiter(*this, src, delimitter);
            break;
    }
}

int Strings::indexOf(const String& s) const
{
    int result = -1;
    const_iterator itor;
    const_reverse_iterator xtor;

    switch (m_sorted)
    {
        case SortOrder::DESCENDING:
            xtor = lower_bound(rbegin(), rend(), s);
            if (xtor != rend() && *xtor == s)
            {
                result = (int) distance(rbegin(), xtor);
            }
            break;
        case SortOrder::ASCENDING:
            itor = lower_bound(begin(), end(), s);
            if (itor != end() && *itor == s)
            {
                result = (int) distance(begin(), itor);
            }
            break;
        default:
            itor = find(begin(), end(), s);
            if (itor != end() && *itor == s)
            {
                result = (int) distance(begin(), itor);
            }
            break;
    }
    return result;
}

void Strings::saveToFile(const String& fileName) const
{
    Buffer buffer;
    for (const auto& str: *this)
    {
        buffer.append(str);
        buffer.append("\n");
    }
    buffer.saveToFile(fileName);
}

void Strings::loadFromFile(const String& fileName)
{
    Buffer buffer;
    buffer.loadFromFile(fileName);

    clear();
    // Load text
    String text(buffer.c_str(), buffer.bytes());

    // Determine delimiter
    String delimiter = "\n";
    if (size_t pos1 = text.find_first_of("\n\r");
        pos1 != string::npos)
    {
        size_t pos2 = text.find_first_of("\n\r", pos1 + 1);
        delimiter = text.substr(pos1, 1);
        if (pos1 + 1 == pos2 && text[pos1] != text[pos2])
        { // Two chars delimiter
            delimiter = text.substr(pos1, 2);
        }
    }

    splitByDelimiter(*this, text, delimiter.c_str());
}

String Strings::join(const String& delimitter) const
{
    stringstream result;
    bool first = true;
    for (const auto& str: *this)
    {
        if (first)
        {
            result << str;
            first = false;
        }
        else
        {
            result << delimitter << str;
        }
    }
    return result.str();
}

Strings Strings::grep(const String& pattern) const
{
    RegularExpression regularExpression(pattern);
    Strings output;
    for (const String& str : *(this))
    {
        if (regularExpression.matches(str))
        {
            output.push_back(str);
        }
    }
    return output;
}

static bool sortAscending(const String& first, const String& second)
{
    return first < second;
}

static bool sortDescending(const String& first, const String& second)
{
    return first > second;
}

void Strings::sort(bool ascending)
{
    if (ascending)
    {
        ::sort(begin(), end(), sortAscending);
        m_sorted = SortOrder::ASCENDING;
    }
    else
    {
        ::sort(begin(), end(), sortDescending);
        m_sorted = SortOrder::DESCENDING;
    }
}

#if USE_GTEST

static const String testString("This is a test\ntext that contains several\nexample rows");
static const String resultString("This is a test\rtext that contains several\rexample rows");

TEST(SPTK_Strings, ctor)
{
    Strings strings(testString, "[\\n\\r]+", Strings::SplitMode::REGEXP);
    EXPECT_EQ(size_t(3), strings.size());
    EXPECT_STREQ(resultString.c_str(), strings.join("\r").c_str());

    Strings strings2(strings);
    EXPECT_EQ(size_t(3), strings2.size());
    EXPECT_STREQ(resultString.c_str(), strings2.join("\r").c_str());

    strings.fromString(testString, "\n", Strings::SplitMode::DELIMITER);
    EXPECT_EQ(size_t(3), strings.size());
    EXPECT_STREQ(resultString.c_str(), strings.join("\r").c_str());

    Strings strings3({"1", "2", "3"});
    EXPECT_EQ(size_t(3), strings3.size());
    EXPECT_STREQ("1,2,3", strings3.join(",").c_str());

    Strings numbers = {{"one",   3, 1},
                       {"two",   3, 2},
                       {"three", 5, 3}};
    EXPECT_EQ(size_t(3), numbers.size());
    EXPECT_STREQ("one,two,three", numbers.join(",").c_str());
    EXPECT_EQ(2, numbers[1].ident());
}

TEST(SPTK_Strings, sort)
{
    Strings strings(testString, "[\\n\\r]+", Strings::SplitMode::REGEXP);
    strings.sort();
    EXPECT_STREQ("This is a test\nexample rows\ntext that contains several", strings.join("\n").c_str());
}

TEST(SPTK_Strings, remove)
{
    Strings strings({"1", "2", "3"});
    strings.remove("2");
    EXPECT_STREQ("1,3", strings.join(",").c_str());
}

TEST(SPTK_Strings, indexOf)
{
    Strings strings(testString, "[\\n\\r]+", Strings::SplitMode::REGEXP);
    EXPECT_EQ(1, strings.indexOf("text that contains several"));
    EXPECT_EQ(-1, strings.indexOf("text that contains"));

    strings.sort();
    EXPECT_EQ(2, strings.indexOf("text that contains several"));
    EXPECT_EQ(-1, strings.indexOf("text that Contains"));

    strings.sort(false);
    EXPECT_EQ(2, strings.indexOf("text that contains several"));
    EXPECT_EQ(-1, strings.indexOf("text that Contains"));
}

TEST(SPTK_Strings, grep)
{
    Strings strings(testString, "[\\n\\r]+", Strings::SplitMode::REGEXP);

    Strings group1 = strings.grep("text");
    EXPECT_EQ(size_t(1), group1.size());

    Strings group2 = strings.grep("text|rows");
    EXPECT_EQ(size_t(2), group2.size());
}

#endif
