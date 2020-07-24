/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
#include <filesystem>

#include <sptk5/DirectoryDS.h>
#include <sptk5/filedefs.h>
#include <sptk5/SystemException.h>

#define CASE_INSENSITIVE 1

#ifdef _WIN32
const char slash = '\\';
#else
#include <dirent.h>
const char slash = '/';
#endif

#ifndef FL_ALIGN_LEFT
#define FL_ALIGN_LEFT 4
#endif

using namespace std;
using namespace sptk;
using namespace filesystem;

string DirectoryDS::getFileType(const struct stat& st, CSmallPixmapType& image, const char* fname) const
{
    bool executable = S_ISEXEC(st.st_mode);
    bool directory = false;
    image = SXPM_DOCUMENT;

    string modeName;
    if (S_ISDIR(st.st_mode)) {
        modeName = "Directory";
        executable = false;
        directory = true;
        image = SXPM_FOLDER;
    } else if (S_ISREG(st.st_mode)) {
        if (executable)
            modeName = "Executable";
        else
            modeName = "File";
    }

    if (executable) {
        image = SXPM_EXECUTABLE;
    } else {
        if (!directory) {
            const char* ext = strrchr(fname, '.');
            const char* sep = strrchr(fname, slash);
            if (ext && ext > sep) {
                ++ext;
                image = imageTypeFromExtention(ext);
            }
        }
    }

    return modeName;
}

CSmallPixmapType DirectoryDS::imageTypeFromExtention(const char* ext) const
{
    static const map<String,CSmallPixmapType> imageTypes
    {
        { "doc", SXPM_DOC_DOCUMENT },
        { "docx", SXPM_DOC_DOCUMENT },
        { "odt", SXPM_DOC_DOCUMENT },
        { "txt", SXPM_TXT_DOCUMENT },
        { "xls", SXPM_XLS_DOCUMENT },
        { "csv", SXPM_XLS_DOCUMENT }
    };

    const auto itor = imageTypes.find(ext);
    if (itor != imageTypes.end())
        return itor->second;

    return SXPM_DOCUMENT;
}

// Define access mode constants if they aren't already defined.
#ifndef R_OK
#define R_OK 04
#endif

// dataset navigation

String DirectoryDS::absolutePath(const String& _path) const
{
    String path(_path);
    char slashStr[] = {slash, 0};
    char currentDir[256];
    String fullPath;
    if (getcwd(currentDir, 255) == nullptr)
        currentDir[0] = 0;

#ifdef _WIN32
    path = path.replace("\\/", "\\");
    if (path[1] == ':')
        fullPath = path;
    else if (path[0] == '\\') {
        fullPath = string(currentDir).substr(0, 2) + path;
    } else
        fullPath = string(currentDir) + slashStr + path;
#else

    path = path.replace("\\\\", "/");
    if (path[0] == slash)
        fullPath = path;
    else
        fullPath = string(currentDir) + slashStr + path;
#endif

    Strings pathItems(fullPath, slashStr);
    for (unsigned i = 0; i < pathItems.size(); ++i) {
        if (pathItems[i] == "..") {
            pathItems.remove(i);
            --i;
            if (i > 0) {
                pathItems.remove(i);
                --i;
            }
        }
        if (pathItems[i] == ".") {
            pathItems.remove(i);
            --i;
        }
    }
#ifdef _WIN32
    path = pathItems.join(slashStr);
#else
    path = "/" + pathItems.join(slashStr);
#endif
    if (!path.length())
        path = slashStr;
    return path;
}

void DirectoryDS::directory(const String& d)
{
    m_directory = absolutePath(d);
}

static bool fileMatchesPattern(const String& fileName, const vector<SRegularExpression>& matchPatterns)
{
    for (const auto& matchPattern: matchPatterns) {
        if (matchPattern->matches(fileName))
            return true;
    }
    return false;
}

Strings DirectoryDS::getFileNames()
{
    Strings fileNames;

    if (m_directory.endsWith("\\") || m_directory.endsWith("/"))
        m_directory = m_directory.substr(0, m_directory.length() - 1);

    for (const auto &file : directory_iterator(m_directory.c_str())) {
        fileNames.push_back(file.path().filename().string());
    }

#ifdef _WIN32
    //open a directory the WIN32 way
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA fdata;

	hFind = FindFirstFile((m_directory + "\\*").c_str(), &fdata);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			fileNames.push_back(fdata.cFileName);
		} while (FindNextFile(hFind, &fdata) != 0);
	}

	if (GetLastError() != ERROR_NO_MORE_FILES)
	{
		FindClose(hFind);
		throw Exception("Error opening directory '" + m_directory + "'");
	}

	FindClose(hFind);
#else
    DIR* dp = opendir(m_directory.c_str());

    if (dp != nullptr) {
        struct dirent *ep;
        while ((ep = readdir(dp))) {
            fileNames.push_back(ep->d_name);
        }
        closedir(dp);
    }
    else
        throw Exception("Error opening directory '" + m_directory + "'");
#endif
    return fileNames;
}

static void getFileInfo(const String& filename, struct stat& st, bool& is_link)
{
    if (lstat(filename.c_str(), &st) != 0)
        throw SystemException("Can't access file '" + filename + "'");

#ifndef _WIN32
    if ((st.st_mode & S_IFLNK) == S_IFLNK) {
        is_link = true;
        if (stat(filename.c_str(), &st) != 0)
            throw SystemException("Can't get file info");
    }
#endif
}

bool DirectoryDS::open()
{
    clear();

    vector<FieldList*>  fileList;
    Strings             fileNames = getFileNames();
    unsigned            index = 0;

    for (auto& fileName: fileNames) {

        if (fileName.endsWith("\\") || fileName.endsWith("/"))
            fileName = fileName.substr(0, fileName.length() - 1);

        if ((showPolicy() & DDS_HIDE_DOT_FILES) && fileName[0] == '.')
            continue;

        struct stat st = {};
        bool is_link = false;

        String fullName = m_directory + "/" + fileName;
        getFileInfo(fullName, st, is_link);

        bool is_dir = S_ISDIR(st.st_mode);
        if (!is_dir) {
            if ((showPolicy() & DDS_HIDE_FILES) == DDS_HIDE_FILES)
                continue;
            if (!m_patterns.empty() && !fileMatchesPattern(fileName, m_patterns))
                continue;
        } else {
            if ((showPolicy() & DDS_HIDE_DIRECTORIES) == DDS_HIDE_DIRECTORIES)
                continue;
        }

        FieldList* df = makeFileListEntry(st, index, fileName, fullName, is_link);

        if (is_dir)
            push_back(df);
        else
            fileList.push_back(df);

    }

    for (auto* df: fileList)
        push_back(df);
    fileList.clear();

    first();

    return !empty();
}

FieldList* DirectoryDS::makeFileListEntry(const struct stat& st, unsigned& index, const String& fileName,
                                          const String& fullName, bool is_link) const
{
    CSmallPixmapType pixmapType;
    String modeName = getFileType(st, pixmapType, fileName.c_str());

    if (is_link) {
        modeName += ' ';
        modeName += "link";
    }

    auto* df = new FieldList(false);
    df->push_back(" ", false).setImageNdx(pixmapType);
    df->push_back("Name", false) = fileName;
    if (modeName == "Directory")
        df->push_back("Size", false) = "";
    else
        df->push_back("Size", false) = (int32_t) st.st_size;
    df->push_back("Type", false) = modeName;
    df->push_back("Modified", false) = DateTime::convertCTime(st.st_mtime);
    df->push_back("", false) = (int32_t) index; // Fake key value
    ++index;

    if (access(fullName.c_str(), R_OK) != 0) {
        (*df)[uint32_t(0)].view.flags = FL_ALIGN_LEFT;
        (*df)[uint32_t(1)].view.flags = FL_ALIGN_LEFT;
    }
    return df;
}

std::shared_ptr<RegularExpression> DirectoryDS::wildcardToRegexp(const String& wildcard)
{
    String regexpStr("^");
    bool groupStarted = false;
    bool charClassStarted = false;
    for (size_t pos = 0; pos < wildcard.length(); ++pos) {
        char ch = wildcard[pos];

        if (charClassStarted) {
            switch (ch) {
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

        switch (ch) {
            case '{':
                groupStarted = true;
                ch = '(';
                break;
            case ',':
                if (groupStarted)
                    ch = '|';
                break;
            case '}':
                if (groupStarted) {
                    groupStarted = false;
                    ch = ')';
                }
                break;
            case '\\':
                regexpStr += ch;
                ++pos;
                if (pos < wildcard.length())
                    regexpStr += wildcard[pos];
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
    String m_path;

    explicit TempDirectory(String path)
    : m_path(move(path))
    {
#ifdef _WIN32
        int rc = system(("mkdir " + m_path).c_str());
        if (rc < 0)
            throw SystemException(("Can't create temp directory " + m_path).c_str());
        rc = system(("mkdir " + m_path + "\\dir1").c_str());
        if (rc < 0)
            throw SystemException(("Can't create temp directory " + m_path + "/dir1").c_str());
#else
        int rc = mkdir(m_path.c_str(), 0777);
        if (rc < 0)
            throw SystemException("Can't create temp directory " + m_path);
        rc = mkdir((m_path + "/dir1").c_str(), 0777);
        if (rc < 0)
            throw SystemException("Can't create temp directory " + m_path + "/dir1");
#endif
        Buffer buffer;
        buffer.fill('X', 10);
        buffer.saveToFile(m_path + "/file1");
        buffer.saveToFile(m_path + "/file2");
    }

    TempDirectory(const TempDirectory&) = delete;
    TempDirectory(TempDirectory&&) = default;
    TempDirectory& operator = (const TempDirectory&) = delete;
    TempDirectory& operator = (TempDirectory&&) noexcept = default;

    ~TempDirectory()
    {
        if (!trim(m_path).empty()) {
#ifdef _WIN32
            system(("rmdir /s /q " + m_path).c_str());
#else
            system(("rm -rf " + m_path).c_str());
#endif
        }
    }
};

TEST (SPTK_DirectoryDS, open)
{
    TempDirectory dir(testTempDirectory + "1");

    DirectoryDS directoryDS(testTempDirectory + "1");
    directoryDS.open();
    map<String, int> files;
    while (!directoryDS.eof()) {
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
    while (!directoryDS.eof()) {
        files[directoryDS["Name"].asString()] = directoryDS["Size"].asInteger();
        directoryDS.next();
    }
    directoryDS.close();

    EXPECT_EQ(size_t(2), files.size());
    EXPECT_EQ(10, files["file1"]);
}

#endif
