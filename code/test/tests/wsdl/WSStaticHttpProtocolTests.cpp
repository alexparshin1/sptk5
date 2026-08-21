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

#include <fstream>
#include <gtest/gtest.h>
#include <sptk5/wsdl/protocol/WSStaticHttpProtocol.h>

using namespace std;
using namespace sptk;

namespace {

/// @brief A directory of static files, with one file in it, and a file outside it to reach for.
class StaticFilesDirectory
{
    filesystem::path m_root;

public:
    StaticFilesDirectory()
        : m_root(filesystem::temp_directory_path() / "sptk_static_files_test")
    {
        remove_all(m_root);
        create_directories(m_root / "assets");
        ofstream(m_root / "index.html") << "<html/>";
        ofstream(m_root / "assets" / "app.js") << "//";
        ofstream(m_root.parent_path() / "sptk_static_files_secret") << "secret";

        // A directory whose name begins with the root's, to show that sharing a prefix is not
        // being contained in it.
        create_directories(sibling().parent_path());
        ofstream(sibling()) << "not ours";
    }

    ~StaticFilesDirectory()
    {
        remove_all(m_root);
        remove_all(sibling().parent_path());
        remove(m_root.parent_path() / "sptk_static_files_secret");
    }

    [[nodiscard]] filesystem::path sibling() const
    {
        return m_root.parent_path() / "sptk_static_files_test-elsewhere" / "file.txt";
    }

    [[nodiscard]] String root() const
    {
        return m_root.string();
    }
};

} // namespace

TEST(SPTK_WSStaticHttpProtocol, servesFilesInTheDirectory)
{
    const StaticFilesDirectory files;

    EXPECT_EQ(filesystem::path(files.root().c_str()) / "index.html",
              WSStaticHttpProtocol::resolveFile(files.root(), "/index.html"));
    EXPECT_EQ(filesystem::path(files.root().c_str()) / "assets" / "app.js",
              WSStaticHttpProtocol::resolveFile(files.root(), "/assets/app.js"));
}

TEST(SPTK_WSStaticHttpProtocol, resolvesPathsThatDoNotExistYet)
{
    const StaticFilesDirectory files;

    // Missing files are the caller's to notice: the web interface answers them with index.html,
    // and that decision needs the resolved path, not a refusal.
    EXPECT_EQ(filesystem::path(files.root().c_str()) / "no-such-page",
              WSStaticHttpProtocol::resolveFile(files.root(), "/no-such-page"));
}

TEST(SPTK_WSStaticHttpProtocol, refusesToLeaveTheDirectory)
{
    const StaticFilesDirectory files;

    // Every one of these reads a file the server process can open but is not serving.
    EXPECT_TRUE(WSStaticHttpProtocol::resolveFile(files.root(), "/../sptk_static_files_secret").empty());
    EXPECT_TRUE(WSStaticHttpProtocol::resolveFile(files.root(), "/assets/../../sptk_static_files_secret").empty());
    EXPECT_TRUE(WSStaticHttpProtocol::resolveFile(files.root(), "/../../../../../../etc/passwd").empty());
    EXPECT_TRUE(WSStaticHttpProtocol::resolveFile(files.root(), "/..").empty());
}

TEST(SPTK_WSStaticHttpProtocol, keepsAnAbsoluteLookingPathInsideTheDirectory)
{
    const StaticFilesDirectory files;

    // A request path always starts with a separator, and any number of them still names something
    // in the directory. Joined rather than appended, such a path would replace the root and the
    // system's own /etc/passwd would be served.
    const auto expected = filesystem::path(files.root().c_str()) / "etc" / "passwd";
    EXPECT_EQ(expected, WSStaticHttpProtocol::resolveFile(files.root(), "/etc/passwd"));
    EXPECT_EQ(expected, WSStaticHttpProtocol::resolveFile(files.root(), "//etc/passwd"));
}

TEST(SPTK_WSStaticHttpProtocol, doesNotTakeASiblingDirectoryForItsOwn)
{
    const StaticFilesDirectory files;

    ASSERT_TRUE(exists(files.sibling()));
    EXPECT_TRUE(WSStaticHttpProtocol::resolveFile(files.root(), "/../sptk_static_files_test-elsewhere/file.txt").empty());
}
