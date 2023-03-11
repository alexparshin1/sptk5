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

#include <fstream>
#include <gtest/gtest.h>
#include <sptk5/Printer.h>
#include <sptk5/Tar.h>
#include <sptk5/md5.h>

using namespace std;
using namespace sptk;

static const String file1_md5 {"2934e1a7ae11b11b88c9b0e520efd978"};
static const String file2_md5 {"adb45e22bba7108bb4ad1b772ecf6b40"};
static const String gtestTempDirectory {"gtest_temp_directory3"};
static const String testTar1 {"gtest_temp1.tar"};
static const String testTar2 {"gtest_temp2.tar"};

class SPTK_Tar
    : public ::testing::Test
{
protected:
    void SetUp() override
    {

        filesystem::create_directories(gtestTempDirectory.c_str());

        constexpr int TestFileBytes = 1000;

        Buffer file1;
        for (int i = 0; i < TestFileBytes; ++i)
        {
            file1.append((const char*) &i, sizeof(i));
        }
        file1.saveToFile(gtestTempDirectory + "/file1.txt");

        Buffer file2;
        for (int i = 0; i < TestFileBytes; ++i)
        {
            file2.append("ABCDEFG HIJKLMN OPQRSTUV\n");
        }
        file2.saveToFile(gtestTempDirectory + "/file2.txt");

        ASSERT_EQ(0, system(("tar cf " + testTar1 + " " + gtestTempDirectory).c_str()));
    }

    void TearDown() override
    {
        filesystem::remove_all(gtestTempDirectory.c_str());
        filesystem::remove(testTar1.c_str());
        filesystem::remove(testTar2.c_str());
        filesystem::remove("test.lst");
    }
};

TEST_F(SPTK_Tar, relativePath)
{
    auto relPath = ArchiveFile::relativePath("/tmp/mydir/myfile.txt", "/tmp/mydir");
    EXPECT_STREQ(relPath.string().c_str(), "myfile.txt");

    relPath = ArchiveFile::relativePath("/tmp/mydir1/mydir2/myfile.txt", "/tmp/mydir1");
    EXPECT_EQ(relPath, filesystem::path("mydir2/myfile.txt"));

    relPath = ArchiveFile::relativePath("/tmp/mydir1/myfile.txt", "/tmp/mydir");
    EXPECT_EQ(relPath, filesystem::path("/tmp/mydir1/myfile.txt"));
}

TEST_F(SPTK_Tar, read) /* NOLINT */
{
    Tar tar;

    EXPECT_NO_THROW(tar.read(testTar1));

    const auto& outfile1 = tar.file(gtestTempDirectory + "/file1.txt");
    const auto& outfile2 = tar.file(gtestTempDirectory + "/file2.txt");
    EXPECT_STREQ(file1_md5.c_str(), md5(outfile1).c_str());
    EXPECT_STREQ(file2_md5.c_str(), md5(outfile2).c_str());
}

TEST_F(SPTK_Tar, write) /* NOLINT */
{
    Tar tar;

    EXPECT_NO_THROW(tar.read(testTar1));
    EXPECT_NO_THROW(tar.save(testTar2));

    ASSERT_EQ(0, system(("tar tf " + testTar1 + " > test.lst").c_str()));
}
