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

#include <string>
#include <sys/stat.h>

#include <sptk5/DirectoryDS.h>
#include <sptk5/filedefs.h>
#include <sptk5/Printer.h>

#ifndef FL_ALIGN_LEFT
constexpr int FL_ALIGN_LEFT = 4;
#endif

using namespace std;
using namespace sptk;
using namespace filesystem;

String DirectoryDS::getFileType(const directory_entry& file, CSmallPixmapType& image, DateTime& modificationTime) const
{
    struct stat st {};

    stat(file.path().string().c_str(), &st);

    String ext = file.path().extension().string();
    modificationTime = DateTime::convertCTime(st.st_mtime);
#ifndef _WIN32
    bool executable = S_ISEXEC(st.st_mode);
#else
    ext = ext.toLowerCase();
    bool executable = ext == "exe" || ext == "bat";
#endif

    bool directory = is_directory(file.status());
    image = CSmallPixmapType::SXPM_DOCUMENT;

    string modeName;
    if (directory)
    {
        modeName = "Directory";
        executable = false;
        image = CSmallPixmapType::SXPM_FOLDER;
    }
    else if (S_ISREG(st.st_mode))
    {
        if (executable)
        {
            modeName = "Executable";
        }
        else
        {
            modeName = "File";
        }
    }

    if (executable)
    {
        image = CSmallPixmapType::SXPM_EXECUTABLE;
    }
    else
    {
        if (!directory && !ext.empty())
        {
            image = imageTypeFromExtention(ext);
        }
    }

    return modeName;
}

CSmallPixmapType DirectoryDS::imageTypeFromExtention(const String& ext)
{
    static const map<String, CSmallPixmapType> imageTypes
        {
            {"doc",  CSmallPixmapType::SXPM_DOC_DOCUMENT},
            {"docx", CSmallPixmapType::SXPM_DOC_DOCUMENT},
            {"odt",  CSmallPixmapType::SXPM_DOC_DOCUMENT},
            {"txt",  CSmallPixmapType::SXPM_TXT_DOCUMENT},
            {"xls",  CSmallPixmapType::SXPM_XLS_DOCUMENT},
            {"csv",  CSmallPixmapType::SXPM_XLS_DOCUMENT}
        };

    if (const auto itor = imageTypes.find(ext); itor != imageTypes.end())
    {
        return itor->second;
    }

    return CSmallPixmapType::SXPM_DOCUMENT;
}

// Define access mode constants if they aren't already defined.
#ifndef R_OK
#define R_OK 04
#endif

// dataset navigation

String DirectoryDS::absolutePath(const String& _path)
{
    path p = _path.c_str();
    String fullPath = absolute(p).string();
    return fullPath;
}

void DirectoryDS::directory(const String& d)
{
    m_directory = absolutePath(d);
}

static bool fileMatchesPattern(const String& fileName, const vector<SRegularExpression>& matchPatterns)
{
    return any_of(matchPatterns.begin(),
                  matchPatterns.end(),
                  [&fileName](const auto& matchPattern) { return matchPattern->matches(fileName); });
}

bool DirectoryDS::open()
{
    size_t index = 0;

    if (m_directory.endsWith("\\") || m_directory.endsWith("/"))
    {
        m_directory = m_directory.substr(0, m_directory.length() - 1);
    }

    clear();

    if ((showPolicy() & DDS_HIDE_DOT_FILES) == 0)
    {
        for (const String& dirName: {".", ".."})
        {
            FieldList df(false);
            df.push_back(" ", false).setImageNdx((uint32_t) CSmallPixmapType::SXPM_FOLDER);
            df.push_back("Name", false) = dirName;
            df.push_back("Size", false) = "";
            df.push_back("Type", false) = "Directory";
            push_back(move(df));
            ++index;
        }
    }

    for (const auto& file : directory_iterator(m_directory.c_str()))
    {

        String fileName = file.path().filename().string();

        if ((showPolicy() & DDS_HIDE_DOT_FILES) && fileName[0] == '.')
        {
            continue;
        }

        if (!is_directory(file.status()))
        {
            if ((showPolicy() & DDS_HIDE_FILES) == DDS_HIDE_FILES)
            {
                continue;
            }
            if (!m_patterns.empty() && !fileMatchesPattern(fileName, m_patterns))
            {
                continue;
            }
        }
        else
        {
            if ((showPolicy() & DDS_HIDE_DIRECTORIES) == DDS_HIDE_DIRECTORIES)
            {
                continue;
            }
        }

        auto entry = makeFileListEntry(file, index);
        push_back(move(entry));
    }

    first();

    return !empty();
}

FieldList DirectoryDS::makeFileListEntry(const directory_entry& file, size_t& index) const
{
    CSmallPixmapType pixmapType = CSmallPixmapType::SXPM_TXT_DOCUMENT;
    DateTime modificationTime;
    String modeName = getFileType(file, pixmapType, modificationTime);

    if (is_symlink(file.status()))
    {
        modeName += " symlink";
    }

    FieldList df(false);
    df.push_back(" ", false).setImageNdx((uint32_t) pixmapType);
    df.push_back("Name", false) = file.path().filename().string();
    if (modeName == "Directory")
    {
        df.push_back("Size", false) = "";
    }
    else
    {
        df.push_back("Size", false) = (int64_t) file_size(file.path());
    }
    df.push_back("Type", false) = modeName;

    df.push_back("Modified", false) = modificationTime;
    df.push_back("", false) = (int32_t) index; // Fake key value
    ++index;

    if (access(file.path().filename().string().c_str(), R_OK) != 0)
    {
        df[uint32_t(0)].view().flags = FL_ALIGN_LEFT;
        df[uint32_t(1)].view().flags = FL_ALIGN_LEFT;
    }

    return df;
}

std::shared_ptr<RegularExpression> DirectoryDS::wildcardToRegexp(const String& wildcard)
{
    String regexpStr("^");
    bool groupStarted = false;
    bool charClassStarted = false;
    for (size_t pos = 0; pos < wildcard.length(); ++pos)
    {
        char ch = wildcard[pos];

        if (charClassStarted)
        {
            switch (ch)
            {
                case '!':
                    ch = '^';
                    break;
                case ']':
                    charClassStarted = false;
                    break;
                default:
                    break;
            }
            regexpStr += ch;
            continue;
        }

        switch (ch)
        {
            case '{':
                groupStarted = true;
                ch = '(';
                break;
            case ',':
                if (groupStarted)
                {
                    ch = '|';
                }
                break;
            case '}':
                if (groupStarted)
                {
                    groupStarted = false;
                    ch = ')';
                }
                break;
            case '\\':
                regexpStr += ch;
                ++pos;
                if (pos < wildcard.length())
                {
                    regexpStr += wildcard[pos];
                }
                break;
            case '?':
                ch = '.';
                break;
            case '*':
                regexpStr += ".";
                break;
            case '[':
                charClassStarted = true;
                break;
            case '(':
            case ')':
            case '+':
            case '.':
                regexpStr += "\\";
                break;
            default:
                break;
        }
        regexpStr += ch;
    }
    regexpStr += "$";
    return make_shared<RegularExpression>(regexpStr);
}

#if USE_GTEST

#ifdef _WIN32
const String testTempDirectory = "C:\\gtest_temp_dir";
#else
const String testTempDirectory = "/tmp/gtest_temp_dir";
#endif

class TempDirectory
{
public:
    explicit TempDirectory(const String& _path)
        : m_path(shared_ptr<path>(new path(_path.c_str()),
                                  [](path* ptr) {
                                      remove_all(*ptr);
                                      delete ptr;
                                  }))
    {
        path dir = *m_path / "dir1";
        try
        {
            create_directories(dir);
        }
        catch (const filesystem_error& e)
        {
            CERR("Can't create temp directory " << dir.filename().string() << ": " << e.what() << endl)
            return;
        }

        Buffer buffer;
        buffer.fill('X', 10);
        buffer.saveToFile((*m_path / "file1").c_str());
        buffer.saveToFile((*m_path / "file2").c_str());
    }

private:

    shared_ptr<path> m_path;
};

TEST (SPTK_DirectoryDS, open)
{
    TempDirectory dir(testTempDirectory + "1");

    DirectoryDS directoryDS(testTempDirectory + "1");
    directoryDS.open();
    map<String, int> files;
    while (!directoryDS.eof())
    {
        files[directoryDS["Name"].asString()] = directoryDS["Size"].asInteger();
        directoryDS.next();
    }
    directoryDS.close();

    EXPECT_EQ(size_t(5), files.size());
    EXPECT_EQ(10, files["file1"]);
}

TEST (SPTK_DirectoryDS, patternToRegexp)
{
    auto regexp = DirectoryDS::wildcardToRegexp("[abc]??");
    EXPECT_STREQ("^[abc]..$", regexp->pattern().c_str());

    regexp = DirectoryDS::wildcardToRegexp("[!a-f][c-z].doc");
    EXPECT_STREQ("^[^a-f][c-z]\\.doc$", regexp->pattern().c_str());

    regexp = DirectoryDS::wildcardToRegexp("{full,short}.*");
    EXPECT_STREQ("^(full|short)\\..*$", regexp->pattern().c_str());
}

TEST (SPTK_DirectoryDS, patterns)
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

#endif
