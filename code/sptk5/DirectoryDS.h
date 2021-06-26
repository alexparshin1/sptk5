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

#pragma once

#include <memory>
#include <sys/stat.h>
#include <sptk5/MemoryDS.h>
#include <sptk5/Strings.h>
#include <sptk5/CSmallPixmapIDs.h>
#include <sptk5/sptk-config.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Directory Show Policies
 */
static constexpr int DDS_SHOW_ALL = 0;          ///< Show everything
static constexpr int DDS_HIDE_FILES = 1;        ///< Hide files
static constexpr int DDS_HIDE_DOT_FILES = 2;    ///< Hide files with the name started with '.' (*nix hidden files,mostly)
static constexpr int DDS_HIDE_DIRECTORIES = 4;  ///< Hide directories
static constexpr int DDS_NO_SORT = 8;           ///< Do not sort

/**
 * @brief Directory datasource
 *
 * A datasource with the list of files
 * and directories along with their attributes. It works just
 * as any other datasource. You set up the parameters, call open()
 * and may use the list. Method close() should be called aftewards
 * to release any allocated resourses.
 */
class SP_EXPORT DirectoryDS
    : public MemoryDS
{
public:
    /**
     * Default Constructor
     * @param _directory        Directory path
     * @param _pattern          OS pattern(s) to match, separated by ';'
     * @param _showPolicy       Bit combination of show policies
     *
     */
    DirectoryDS(const String& _directory = "", const String& _pattern = "", int _showPolicy = 0)
        : MemoryDS(), m_showPolicy(_showPolicy)
    {
        if (!_directory.empty())
        {
            directory(_directory);
        }
        if (!_pattern.empty())
        {
            pattern(_pattern);
        }
    }

    /**
     * Returns current show policy, @see CDirectoryDSpolicies for more information
     * @returns current show policy
     */
    int showPolicy() const
    {
        return m_showPolicy;
    }

    /**
     * Sets current show policy, see CDirectoryDSpolicies for more information
     */
    void showPolicy(int type)
    {
        m_showPolicy = type;
    }

    /**
     * Sets current directory
     */
    void directory(const String& d);

    /**
     * Returns current directory
     */
    String directory() const
    {
        return m_directory;
    }

    /**
     * Sets pattern in format like: "*.txt;*.csv;*.xls"
     * @param pattern           Patterns to match, separated with ';'
     */
    void pattern(const String& wildcards)
    {
        Strings patterns(wildcards, ";", Strings::SplitMode::DELIMITER);
        m_patterns.clear();
        for (const auto& pattern: patterns)
        {
            auto matchPattern = wildcardToRegexp(pattern);
            m_patterns.push_back(matchPattern);
        }
    }

    /**
     * Opens the directory and fills in the dataset
     */
    bool open() override;

    /**
     * Creates regular expression from wildcard
     * @param wildcard          Wilcard
     * @return regular expression object
     */
    static std::shared_ptr<RegularExpression> wildcardToRegexp(const String& wildcard);

protected:
    /**
     * Sets up an appropriate image and a name for the file type
     * @param file              File information
     * @returns the file type name
     */
    String getFileType(const fs::directory_entry& file, CSmallPixmapType& image,
                       DateTime& modificationTime) const;

private:
    /**
     * Current directory
     */
    String m_directory;

    /**
     * Current file pattern
     */
    std::vector<std::shared_ptr<RegularExpression> > m_patterns;

    /**
     * Show policy, see CDirectoryDSpolicies for more information
     */
    int m_showPolicy;

    /**
     * Returns absolute path to directory or file
     * @param path              Relative path
     * @return absolute path
     */
    static String absolutePath(const String& path);

    /**
     * Create a row in the data source
     * @param file              File information
     * @return data source row
     */
    FieldList makeFileListEntry(const fs::directory_entry& file, size_t& index) const;

    static CSmallPixmapType imageTypeFromExtention(const String& ext);
};
/**
 * @}
 */
}
