/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DirectoryDS.h - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_DIRECTORYDS_H__
#define __SPTK_DIRECTORYDS_H__

#include <sys/stat.h>
#include <sptk5/MemoryDS.h>
#include <sptk5/Strings.h>
#include <sptk5/CSmallPixmapIDs.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Directory Show Policies
 */
enum DirectoryDSpolicies {
    /**
     * Show everything
     */
    DDS_SHOW_ALL = 0,

    /**
     * Hide files
     */
    DDS_HIDE_FILES = 1,

    /**
     * Hide files with the name started with '.' (Unix hidden files,mostly)
     */
    DDS_HIDE_DOT_FILES = 2,

    /**
     * Hide directories
     */
    DDS_HIDE_DIRECTORIES = 4,

    /**
     * Do not sort
     */
    DDS_NO_SORT = 8

};

/**
 * @brief Directory datasource
 *
 * A datasource with the list of files
 * and directories along with their attributes. It works just
 * as any other datasource. You set up the parameters, call open()
 * and may use the list. Method close() should be called aftewards
 * to release any allocated resourses.
 */
class DirectoryDS: public MemoryDS
{
protected:
    /**
     * Sets up an appropriate image and a name for the file type
     * @param st                The file type information
     * @param image             The image type
     * @param fname             The file name
     * @returns the file type name
     */
    std::string getFileType(const struct stat& st, CSmallPixmapType& image, const char *fname) const;

private:
    /**
     * Current directory
     */
    String          m_directory;

    /**
     * Current file pattern
     */
    Strings         m_pattern;

    /**
     * Show policy, see CDirectoryDSpolicies for more information
     */
    int             m_showPolicy;

    /**
     * Returns absolute path to directory or file
     * @param path              Relative path
     * @return absolute path
     */
    String absolutePath(const String& path) const;

public:
    /**
     * Default Constructor
     * @param _directory        Directory path
     * @param _pattern          OS pattern(s) to match, separated by ';'
     * @param _showPolicy       Bit combination of show policies
     *
     */
    DirectoryDS(const String& _directory="", const String& _pattern="", int _showPolicy=0)
    : MemoryDS(),
      m_showPolicy(_showPolicy)
    {
        if (!_directory.empty())
            directory(_directory);
        if (!_pattern.empty())
            pattern(_pattern);
    }

    /**
     * Destructor
     */
    ~DirectoryDS()
    {
        close();
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
    void pattern(const String& pattern)
    {
        m_pattern.fromString(pattern, ";", Strings::SM_DELIMITER);
    }

    /**
     * Returns pattern in format like: "*.txt;*.csv;*.xls"
     */
    String pattern() const
    {
        return m_pattern.asString(";");
    }

    /**
     * Opens the directory and fills in the dataset
     */
    virtual bool open();
};
/**
 * @}
 */
}
#endif
