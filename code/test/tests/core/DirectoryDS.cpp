/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <string>

#include <sptk5/DirectoryDS.h>
#include <sptk5/Printer.h>
#include <sptk5/filedefs.h>

#include <gtest/gtest.h>

using namespace std;
using namespace sptk;
using namespace fs;

#ifdef _WIN32
const String testTempDirectory = "C:\\gtest_temp_dir";
#else
const String testTempDirectory = "/tmp/gtest_temp_dir";
#endif

class TempDirectory
{
public:
    explicit TempDirectory(const String& _path)
        : m_path(_path.c_str())
    {
        path dir = m_path / "dir1";
        try
        {
            create_directories(dir);
        }
        catch (const filesystem_error& e)
        {
            CERR("Can't create temp directory " << dir.filename().string() << ": " << e.what() << endl)
            return;
        }

        constexpr size_t charCount {10};
        Buffer buffer;
        buffer.fill('X', charCount);
        buffer.saveToFile((m_path / "file1").c_str());
        buffer.saveToFile((m_path / "file2").c_str());
    }

    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;

    ~TempDirectory()
    {
        remove_all(m_path);
    }

private:
    path m_path;
};

TEST(SPTK_DirectoryDS, open)
{
    TempDirectory dir(testTempDirectory + "1");

    DirectoryDS directoryDS(testTempDirectory + "1");
    directoryDS.open();
    map<String, int> files;
    while (!directoryDS.eof())
    {
        const auto& field = directoryDS["Name"];
        const auto& fileName = field.asString();
        const auto fileSize = directoryDS["Size"].asInteger();
        files[fileName] = fileSize;
        directoryDS.next();
    }
    directoryDS.close();

    EXPECT_EQ(size_t(5), files.size());
    EXPECT_EQ(10, files["file1"]);
}

TEST(SPTK_DirectoryDS, patternToRegexp)
{
    auto regexp = DirectoryDS::wildcardToRegexp("[abc]??");
    EXPECT_STREQ("^[abc]..$", regexp->pattern().c_str());

    regexp = DirectoryDS::wildcardToRegexp("[!a-f][c-z].doc");
    EXPECT_STREQ("^[^a-f][c-z]\\.doc$", regexp->pattern().c_str());

    regexp = DirectoryDS::wildcardToRegexp("{full,short}.*");
    EXPECT_STREQ("^(full|short)\\..*$", regexp->pattern().c_str());
}

TEST(SPTK_DirectoryDS, patterns)
{
    TempDirectory dir(testTempDirectory + "2");

    DirectoryDS directoryDS(testTempDirectory + "2", "file1;dir*", DDS_HIDE_DOT_FILES);
    directoryDS.open();
    map<String, int> files;
    while (!directoryDS.eof())
    {
        files[directoryDS["Name"].asString()] = directoryDS["Size"].asInteger();
        directoryDS.next();
    }
    directoryDS.close();

    EXPECT_EQ(size_t(2), files.size());
    EXPECT_EQ(10, files["file1"]);
}
