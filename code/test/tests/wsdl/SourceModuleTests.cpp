/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <gtest/gtest.h>

#include <sptk5/wsdl/SourceModule.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

using namespace std;
using namespace sptk;

namespace {
filesystem::path makeUniqueTempDir(const string& prefix)
{
    const auto base = filesystem::temp_directory_path();
    const auto now = chrono::steady_clock::now().time_since_epoch().count();
    const auto dir = base / format("{}_{}", prefix, now);
    filesystem::create_directories(dir);
    return dir;
}

string readAllText(const filesystem::path& p)
{
    ifstream     in(p, ios::binary);
    stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
/// @brief Moves the files' modification times into the past.
///
/// Lets a test tell "rewritten" from "left alone" without waiting for wall-clock time to
/// cross the filesystem's timestamp granularity: a rewrite stamps the file with "now",
/// which is unmistakably newer than a backdated stamp even at 1-second resolution.
/// writeOutputFiles() decides whether to rewrite by comparing content, never timestamps,
/// so backdating cannot influence what is under test.
void backdate(const initializer_list<filesystem::path> paths)
{
    constexpr auto shift = chrono::seconds(2);
    for (const auto& path: paths)
    {
        filesystem::last_write_time(path, filesystem::last_write_time(path) - shift);
    }
}
} // namespace
namespace sptk {

TEST(SourceModuleTests,writeOutputFilesCreatesHeaderAndSource)
{
    const auto outDir = makeUniqueTempDir("sptk_SourceModuleTests_create");

    SourceModule sourceModule("GeneratedUnit", outDir.string());
    sourceModule.reset();
    sourceModule.header() << "#pragma once\n\n"
                          << "int generatedHeaderValue();\n";
    sourceModule.source() << "#include \"GeneratedUnit.h\"\n\n"
                          << "int generatedHeaderValue(){return 123;}\n";

    ASSERT_NO_THROW(sourceModule.writeOutputFiles());

    const auto headerPath = outDir / "GeneratedUnit.h";
    const auto sourcePath = outDir / "GeneratedUnit.cpp";

    EXPECT_TRUE(filesystem::exists(headerPath));
    EXPECT_TRUE(filesystem::exists(sourcePath));

    EXPECT_NE(string::npos, readAllText(headerPath).find("generatedHeaderValue"));
    EXPECT_NE(string::npos, readAllText(sourcePath).find("return 123"));
}

TEST(SourceModuleTests,writeOutputFilesDoesNotRewriteWhenContentIsSame)
{
    const auto outDir = makeUniqueTempDir("sptk_SourceModuleTests_norewrite");

    const auto headerPath = outDir / "GeneratedUnit.h";
    const auto sourcePath = outDir / "GeneratedUnit.cpp";

    {
        SourceModule sourceModule("GeneratedUnit", outDir.string());
        sourceModule.reset();
        sourceModule.header() << "#pragma once\n\n"
                              << "int f();\n";
        sourceModule.source() << "#include \"GeneratedUnit.h\"\n\n"
                              << "int f(){return 1;}\n";
        sourceModule.writeOutputFiles();
    }

    ASSERT_TRUE(filesystem::exists(headerPath));
    ASSERT_TRUE(filesystem::exists(sourcePath));

    backdate({headerPath, sourcePath});

    const auto headerTime1 = filesystem::last_write_time(headerPath);
    const auto sourceTime1 = filesystem::last_write_time(sourcePath);

    {
        SourceModule sourceModule("GeneratedUnit", outDir.string());
        sourceModule.reset();
        sourceModule.header() << "#pragma once\n\n"
                              << "int f();\n";
        sourceModule.source() << "#include \"GeneratedUnit.h\"\n\n"
                              << "int f(){return 1;}\n";
        sourceModule.writeOutputFiles();
    }

    const auto headerTime2 = filesystem::last_write_time(headerPath);
    const auto sourceTime2 = filesystem::last_write_time(sourcePath);

    EXPECT_EQ(headerTime1, headerTime2);
    EXPECT_EQ(sourceTime1, sourceTime2);
}

TEST(SourceModuleTests,writeOutputFilesRewritesWhenContentChanges)
{
    const auto outDir = makeUniqueTempDir("sptk_SourceModuleTests_rewrite");

    const auto headerPath = outDir / "GeneratedUnit.h";
    const auto sourcePath = outDir / "GeneratedUnit.cpp";

    {
        SourceModule sourceModule("GeneratedUnit", outDir.string());
        sourceModule.reset();
        sourceModule.header() << "#pragma once\n\n"
                              << "int f();\n";
        sourceModule.source() << "#include \"GeneratedUnit.h\"\n\n"
                              << "int f(){return 1;}\n";
        sourceModule.writeOutputFiles();
    }

    backdate({headerPath, sourcePath});

    const auto headerTime1 = filesystem::last_write_time(headerPath);
    const auto sourceTime1 = filesystem::last_write_time(sourcePath);

    {
        SourceModule sourceModule("GeneratedUnit", outDir.string());
        sourceModule.reset();
        sourceModule.header() << "#pragma once\n\n"
                              << "int f();\n";
        sourceModule.source() << "#include \"GeneratedUnit.h\"\n\n"
                              << "int f(){return 2;}\n"; // changed
        sourceModule.writeOutputFiles();
    }

    const auto headerTime2 = filesystem::last_write_time(headerPath);
    const auto sourceTime2 = filesystem::last_write_time(sourcePath);

    EXPECT_EQ(headerTime1, headerTime2); // header unchanged
    EXPECT_NE(sourceTime1, sourceTime2); // source rewritten
    EXPECT_NE(string::npos, readAllText(sourcePath).find("return 2"));
}

TEST(SourceModuleTests,resetClearsStreams)
{
    const auto outDir = makeUniqueTempDir("sptk_SourceModuleTests_reset");

    SourceModule sourceModule("GeneratedUnit", outDir.string());
    sourceModule.reset();
    sourceModule.header() << "abc";
    sourceModule.source() << "def";

    sourceModule.reset(); // should clear buffers

    sourceModule.header() << "H";
    sourceModule.source() << "S";
    sourceModule.writeOutputFiles();

    EXPECT_EQ("H", readAllText(outDir / "GeneratedUnit.h"));
    EXPECT_EQ("S", readAllText(outDir / "GeneratedUnit.cpp"));
}

} // namespace sptk_test
