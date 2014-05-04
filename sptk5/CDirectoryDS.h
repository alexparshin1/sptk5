/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDirectoryDS.h  -  description
                             -------------------
    begin                : Tue Mar 26 2002
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/
#ifndef __CDIRECTORYDS_H__
#define __CDIRECTORYDS_H__

#include <sys/stat.h>
#include <sptk5/CMemoryDS.h>
#include <sptk5/CStrings.h>
#include <sptk5/CSmallPixmapIDs.h>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// Directory Show Policies
enum CDirectoryDSpolicies {
    DDS_SHOW_ALL = 0,         ///< Show everything
    DDS_HIDE_FILES = 1,       ///< Hide files
    DDS_HIDE_DOT_FILES = 2,   ///< Hide files with the name started with '.' (Unix hidden files,mostly)
    DDS_HIDE_DIRECTORIES = 4, ///< Hide directories
    DDS_NO_SORT = 8           ///< Do not sort
};

/// @brief Directory datasource
///
/// A datasource with the list of files
/// and directories along with their attributes. It works just
/// as any other datasource. You set up the parameters, call open()
/// and may use the list. Method close() should be called aftewards
/// to release any allocated resourses.
class CDirectoryDS: public CMemoryDS
{
protected:
    /// Sets up an appropriate image and a name for the file type
    /// @param st const struct stat &, the file type information
    /// @param image CSmallPixmapType&, the image type
    /// @param fname const char *, file name
    /// @returns the file type name
    std::string getFileType(const struct stat& st, CSmallPixmapType& image, const char *fname) const;

private:
    std::string     m_directory;   ///< Current directory
    CStrings        m_pattern;     ///< Current file pattern
    int             m_showPolicy;  ///< Show policy, see CDirectoryDSpolicies for more information

public:
    /// Default Constructor
    CDirectoryDS() :
            CMemoryDS(),
            m_showPolicy(0)
    {
    }

    /// Destructor
    virtual ~CDirectoryDS()
    {
        close();
    }

    /// Returns current show policy, @see CDirectoryDSpolicies for more information
    /// @returns current show policy
    int showPolicy() const
    {
        return m_showPolicy;
    }

    /// Sets current show policy, see CDirectoryDSpolicies for more information
    void showPolicy(int type)
    {
        m_showPolicy = type;
    }

    /// Returns current directory
    void directory(std::string d);

    /// Sets current directory
    const std::string &directory() const
    {
        return m_directory;
    }

    /// Sets pattern in format like: "*.txt;*.csv;*.xls"
    void pattern(std::string pattern)
    {
        m_pattern.fromString(pattern, ";");
    }

    /// Returns pattern in format like: "*.txt;*.csv;*.xls"
    const std::string pattern() const
    {
        return m_pattern.asString(";");
    }

    /// Opens the directory and fills in the dataset
    virtual bool open()
    THROWS_EXCEPTIONS;
};
/// @}
}
#endif
