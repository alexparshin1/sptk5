/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DirectoryDS.cpp - description                          ║
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

#include <string>
#include <sys/stat.h>

#include <sptk5/DirectoryDS.h>
#include <sptk5/SystemException.h>

#include <sptk5/filedefs.h>

#define CASE_INSENSITIVE 1

#ifdef _WIN32
const char slash = '\\';
#else
const char slash = '/';
#endif

#ifndef FL_ALIGN_LEFT
#define FL_ALIGN_LEFT 4
#endif

using namespace std;
using namespace sptk;

/* That function is adapted from Rich Salz. */
bool fl_file_match(const char *s, const char *p)
{
    int nesting;

    for (;;) {
        switch (*p++) {

            case '?': // match any single character
                if (!*s++)
                    return false;
                break;

            case '*': // match 0-n of any characters
                // do trailing * quickly
                if (!*p)
                    return true;
                while (!fl_file_match(s, p))
                    if (!*s++)
                        return false;
                return true;

            case '[': // match one character in set of form [abc-d] or [^a-b]
            {
                if (!*s)
                    return false;
                bool reverse = (*p == '^' || *p == '!');
                if (reverse)
                    p++;
                bool matched = false;
                char last = 0;
                while (*p) {
                    if (*p == '-' && last) {
                        if (*s <= *++p && *s >= last)
                            matched = true;
                        //last = 0;
                    } else {
                        if (*s == *p)
                            matched = true;
                    }
                    last = *p++;
                    if (*p == ']')
                        break;
                }
                if (matched == reverse)
                    return false;
                s++;
                p++;
            }
                break;

            case '{': // {pattern1|pattern2|pattern3}
                NEXTCASE :
                if (fl_file_match(s, p))
                    return true;
                for (nesting = 0;;) {
                    switch (*p++) {
                        case '\\':
                            if (*p)
                                p++;
                            break;
                        case '{':
                            nesting++;
                            break;
                        case '}':
                            if (!nesting--)
                                return false;
                            break;
                        case '|':
                        case ',':
                            if (nesting == 0)
                                goto NEXTCASE;
                        case 0:
                            return false;
                        default:
                            break;
                    }
                }
            case '|': // skip rest of |pattern|pattern} when called recursively
            case ',':
                for (nesting = 0; *p && nesting >= 0;) {
                    switch (*p++) {
                        case '\\':
                            if (*p)
                                p++;
                            break;
                        case '{':
                            nesting++;
                            break;
                        case '}':
                            nesting--;
                            break;
                        default:
                            break;
                    }
                }
                break;
            case '}':
                break;

            case 0: // end of pattern
                return !*s;

#if CASE_INSENSITIVE

            case '\\': // quote next character
                if (*p)
                    p++;
                if (*s++ != *(p - 1))
                    return false;
                break;
            default:
                if (*s != *(p - 1) && tolower(*s) != *(p - 1))
                    return false;
                s++;
                break;
#else

            case '\\': // quote next character
                if (*p)
                    p++;
            default:
                if (*s++ != *(p - 1))
                    return false;
                break;
#endif

        }
    }
    //return false;
}

// Returns typename

string DirectoryDS::getFileType(const struct stat &st, CSmallPixmapType& image, const char *fname) const
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
            const char *ext = strrchr(fname, '.');
            const char *sep = strrchr(fname, slash);
            if (ext && ext > sep) {
                ext++;
                if (strcasecmp(ext, "doc") == 0)
                    image = SXPM_DOC_DOCUMENT;
                if (strcasecmp(ext, "txt") == 0)
                    image = SXPM_TXT_DOCUMENT;
                if (strcasecmp(ext, "xls") == 0)
                    image = SXPM_XLS_DOCUMENT;
                if (strcasecmp(ext, "csv") == 0)
                    image = SXPM_XLS_DOCUMENT;
            }
        }
    }

    return modeName;
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
    for (unsigned i = 0; i < pathItems.size(); i++) {
        if (pathItems[i] == "..") {
            pathItems.remove(i);
            i--;
            if (i > 0) {
                pathItems.remove(i);
                i--;
            }
        }
        if (pathItems[i] == ".") {
            pathItems.remove(i);
            i--;
        }
    }
#ifdef _WIN32
    path = pathItems.asString(slashStr);
#else
    path = "/" + pathItems.asString(slashStr);
#endif
    if (!path.length())
        path = slashStr;
    return path;
}

void DirectoryDS::directory(const String& d)
{
    m_directory = absolutePath(d);
}

// read the directory() and move item into the first entry

bool DirectoryDS::open()
{
    clear();

    size_t dlen = m_directory.length() - 1;
    if (dlen) {
        if (m_directory[dlen] != slash)
            m_directory += slash;
    }
#ifdef _WIN32
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile((m_directory + "*.*").c_str(), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;
#else
    dirent **files;
    //int num_files = fl_filename_list(m_directory.c_str(), &files);
    int num_files = scandir(m_directory.c_str(), &files, nullptr, alphasort);
    if (num_files <= 0)
        return false;
#endif

    struct stat st = {};

    vector<FieldList*> fileList;
    int n = 0;
    unsigned index = 0;
    do {

#ifdef _WIN32
        char* file = FindFileData.cFileName;
#else

        if (!files[n]) {
            n++;
            continue;
        }
        auto file = (char *) files[n]->d_name;
#endif

        size_t len = strlen(file);
        if (len && file[len - 1] == '/')
            file[len - 1] = 0;

        if ((showPolicy() & DDS_HIDE_DOT_FILES) && file[0] == '.') {
            n++;
            continue;
        }

        bool is_link = false;
        bool is_dir = false;

        string fullName = m_directory + file;
        if (lstat(fullName.c_str(), &st) != 0)
            throw SystemException("Can't access file '" + fullName + "'");

#ifndef _WIN32
        if ((st.st_mode & S_IFLNK) == S_IFLNK) {
            is_link = true;
            if (stat(fullName.c_str(), &st) != 0)
                throw SystemException("Can't get directory info");
        }
#endif
        if (!S_ISDIR(st.st_mode)) {
            size_t patternCount = m_pattern.size();
            if (patternCount) {
                bool matchFound = false;
                for (unsigned i = 0; i < patternCount; i++) {
                    if (fl_file_match(file, m_pattern[i].c_str())) {
                        matchFound = true;
                        break;
                    }
                }
                if (!matchFound) {
#ifndef _WIN32
                    free(files[n]);
                    files[n] = nullptr;
                    n++;
#endif
                    continue;
                }
            }
        } else
            is_dir = true;

        CSmallPixmapType pixmapType;
        String modeName = getFileType(st, pixmapType, file);

        if (is_link) {
            modeName += ' ';
            modeName += "link";
        }

        bool useEntry;
        if (is_dir)
            useEntry = (showPolicy() & DDS_HIDE_DIRECTORIES) == 0;
        else
            useEntry = (showPolicy() & DDS_HIDE_FILES) == 0;

        if (useEntry) {
            auto df = new FieldList(false);
            df->push_back(" ", false).setImageNdx(pixmapType);
            df->push_back("Name", false) = file;
            if (modeName == "Directory")
                df->push_back("Size", false) = "";
            else
                df->push_back("Size", false) = (uint32_t) st.st_size;
            df->push_back("Type", false) = modeName;
            df->push_back("Modified", false) = DateTime::convertCTime(st.st_mtime);
            df->push_back("", false) = (uint32_t) index; // Fake key value
            index++;

            if (access(fullName.c_str(), R_OK) != 0) {
                (*df)[uint32_t(0)].view.flags = FL_ALIGN_LEFT;
                (*df)[uint32_t(1)].view.flags = FL_ALIGN_LEFT;
            }

            if (is_dir)
                m_list.push_back(df);
            else
                fileList.push_back(df);
        }

#ifndef _WIN32
        free(files[n]);
        files[n] = nullptr;
#endif

        n++;
    }
#ifdef _WIN32
    while (FindNextFile(hFind, &FindFileData));
#else
    while (n < num_files)
        ;
#endif

    for (auto& file: fileList)
        m_list.push_back(file);
    fileList.clear();

    first();

    int defaultWidths[5] = {3, 30, 10, 10, 16};
    if (m_current)
        for (unsigned f = 0; f < 5; f++) {
            (*this)[f].view.flags = FL_ALIGN_LEFT;
            (*this)[f].view.width = defaultWidths[f];
        }

#ifndef _WIN32
    free(files);
#endif

    return !m_list.empty();
}

#if USE_GTEST
#include <gtest/gtest.h>

#ifdef _WIN32
	const String testTempDirectory = "C:\\gtest_temp_dir";
#else
	const String testTempDirectory = "/tmp/gtest_temp_dir";
#endif

struct TempDirectory
{
    String  m_path;
    explicit TempDirectory(const String& path)
    : m_path(path)
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
        buffer.fill('X',10);
        buffer.saveToFile(m_path + "/file1");
        buffer.saveToFile(m_path + "/file2");
    }

    ~TempDirectory()
    {
#ifdef _WIN32
		system(("rmdir /s /q " + m_path).c_str());
#else
		system(("rm -rf " + m_path).c_str());
#endif
    }
};

TEST (SPTK_DirectoryDS, open)
{
    TempDirectory dir(testTempDirectory + "1");

    DirectoryDS directoryDS(testTempDirectory + "1");
    directoryDS.open();
    map<String,int> files;
    while (!directoryDS.eof()) {
        files[ directoryDS["Name"].asString() ] =  directoryDS["Size"].asInteger();
        directoryDS.next();
    }
    directoryDS.close();

    EXPECT_EQ(size_t(5), files.size());
    EXPECT_EQ(10, files["file1"]);
}

TEST (SPTK_DirectoryDS, patterns)
{
    TempDirectory dir(testTempDirectory + "2");

    DirectoryDS directoryDS(testTempDirectory + "2", "file1;dir*", DDS_HIDE_DOT_FILES);
    directoryDS.open();
    map<String,int> files;
    while (!directoryDS.eof()) {
        files[ directoryDS["Name"].asString() ] =  directoryDS["Size"].asInteger();
        directoryDS.next();
    }
    directoryDS.close();

    EXPECT_EQ(size_t(2), files.size());
    EXPECT_EQ(10, files["file1"]);
}

#endif
