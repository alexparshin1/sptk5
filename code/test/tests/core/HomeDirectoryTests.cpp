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

#include <sptk5/Exception.h>
#include <sptk5/HomeDirectory.h>

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>

using namespace std;
using namespace sptk;

namespace {

class EnvVarGuard
{
public:
    explicit EnvVarGuard(const char* name)
        : m_name(name)
    {
        if (const char* v = getenv(m_name.c_str()))
        {
            m_oldValue = string(v);
        }
    }

    void set(const string& value) const
    {
#ifdef _WIN32
        _putenv_s(m_name.c_str(), value.c_str());
#else
        setenv(m_name.c_str(), value.c_str(), 1);
#endif
    }

    void unset() const
    {
#ifdef _WIN32
        _putenv_s(m_name.c_str(), "");
#else
        unsetenv(m_name.c_str());
#endif
    }

    ~EnvVarGuard()
    {
        if (m_oldValue.has_value())
        {
#ifdef _WIN32
            _putenv_s(m_name.c_str(), m_oldValue->c_str());
#else
            setenv(m_name.c_str(), m_oldValue->c_str(), 1);
#endif
        }
        else
        {
#ifdef _WIN32
            _putenv_s(m_name.c_str(), "");
#else
            unsetenv(m_name.c_str());
#endif
        }
    }

private:
    string           m_name;
    optional<string> m_oldValue;
};

} // namespace

namespace sptk {

#ifndef _WIN32

TEST(HomeDirectoryTests,locationPrefersHOME)
{
    EnvVarGuard home("HOME");
    EnvVarGuard user("USER");

    home.set("/tmp/sptk_home_test_dir");
    user.set("user_should_not_be_used");

    const filesystem::path p = HomeDirectory::location();
    EXPECT_EQ(filesystem::path("/tmp/sptk_home_test_dir"), p);
}

TEST(HomeDirectoryTests,locationFallsBackToHomeUserWhenHOMEMissing)
{
    EnvVarGuard home("HOME");
    EnvVarGuard user("USER");

    home.unset();
    user.set("testuser");

    const filesystem::path p = HomeDirectory::location();
    EXPECT_EQ(filesystem::path("/home/testuser"), p);
}

TEST(HomeDirectoryTests,locationThrowsWhenNoHOMEAndNoUSER)
{
    EnvVarGuard home("HOME");
    EnvVarGuard user("USER");

    home.unset();
    user.unset();

    EXPECT_THROW({ (void) HomeDirectory::location(); }, Exception);
}

#else

TEST(HomeDirectoryTests,locationUsesHomeDriveAndPathOnWindowsWhenPresent)
{
    EnvVarGuard homeDrive("HOMEDRIVE");
    EnvVarGuard homePath("HOMEPATH");

    homeDrive.set("C:");
    homePath.set("\\Users\\TestUser");

    const filesystem::path p = HomeDirectory::location();
    EXPECT_EQ(filesystem::path("C:\\Users\\TestUser"), p);
}

#endif

}