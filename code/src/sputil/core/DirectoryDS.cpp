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

#include <string>
#include <sys/stat.h>

#include <sptk5/DirectoryDS.h>
#include <sptk5/filedefs.h>

#ifndef FL_ALIGN_LEFT
constexpr auto FL_ALIGN_LEFT = 4;
#endif

using namespace std;
using namespace sptk;
using namespace filesystem;

String DirectoryDS::getFileType(const directory_entry& file, CSmallPixmapType& image, DateTime& modificationTime)
{
    struct stat fileStat = {};

    if (stat(file.path().string().c_str(), &fileStat) != 0)
    {
        return "Unknown";
    }

    modificationTime = DateTime::convertCTime(fileStat.st_mtime);
#ifndef _WIN32
    const String ext = file.path().extension().string();
    auto         executable = S_ISEXEC(fileStat.st_mode);
#else
    String ext = file.path().extension().string();
    ext = ext.toLowerCase();
    bool executable = ext == ".exe" || ext == ".bat";
#endif

    const auto directory = is_directory(file.status());
    image = CSmallPixmapType::SXPM_DOCUMENT;

    string modeName;
    if (directory)
    {
        modeName = "Directory";
        executable = false;
        image = CSmallPixmapType::SXPM_FOLDER;
    }
    else if (S_ISREG(fileStat.st_mode))
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
            image = imageTypeFromExtension(ext);
        }
    }

    return modeName;
}

CSmallPixmapType DirectoryDS::imageTypeFromExtension(const String& ext)
{
    using enum CSmallPixmapType;
    static const map<String, CSmallPixmapType> imageTypes {
        {"doc", SXPM_DOC_DOCUMENT},
        {"docx", SXPM_DOC_DOCUMENT},
        {"odt", SXPM_DOC_DOCUMENT},
        {"txt", SXPM_TXT_DOCUMENT},
        {"xls", SXPM_XLS_DOCUMENT},
        {"csv", SXPM_XLS_DOCUMENT}};

    if (const auto itor = imageTypes.find(ext); itor != imageTypes.end())
    {
        return itor->second;
    }

    return SXPM_DOCUMENT;
}

// Define access mode constants if they aren't already defined.
#ifndef R_OK
#define R_OK 04
#endif

// dataset navigation

String DirectoryDS::absolutePath(const String& _path)
{
    String fullPath = absolute(_path.c_str()).string();
    return fullPath;
}

void DirectoryDS::directory(const String& dirName)
{
    m_directory = absolutePath(dirName);
}

namespace {
bool fileMatchesPattern(const String& fileName, const vector<SRegularExpression>& matchPatterns)
{
    return ranges::any_of(matchPatterns,
                          [&fileName](const auto& matchPattern)
                          {
                              return matchPattern->matches(fileName);
                          });
}
} // namespace

bool DirectoryDS::open()
{
    size_t index = 0;

    clear();

    if (!exists(m_directory.c_str()))
    {
        throw Exception("Directory doesn't exist");
    }

    if ((showPolicy() & DDS_HIDE_DOT_FILES) == 0)
    {
        for (const char* dirName: {".", ".."})
        {
            auto fields = make_unique<FieldList>(false);
            fields->push_back(" ", false).setImageNdx(static_cast<uint32_t>(CSmallPixmapType::SXPM_FOLDER));
            fields->push_back("Name", false) = dirName;
            fields->push_back("Size", false) = "";
            fields->push_back("Type", false) = "Directory";
            push_back(std::move(fields));
            ++index;
        }
    }

    for (const auto& file: directory_iterator(m_directory.c_str()))
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
        push_back(std::move(entry));
    }

    first();

    return !empty();
}

UFieldList DirectoryDS::makeFileListEntry(const directory_entry& file, size_t& index)
{
    auto     pixmapType = CSmallPixmapType::SXPM_TXT_DOCUMENT;
    DateTime modificationTime;
    String   modeName = getFileType(file, pixmapType, modificationTime);

    if (is_symlink(file.status()))
    {
        modeName += " symlink";
    }

    auto fields = make_unique<FieldList>(false);
    fields->push_back(" ", false).setImageNdx(static_cast<uint32_t>(pixmapType));
    fields->push_back("Name", false) = file.path().filename().string();
    if (modeName == "Directory")
    {
        fields->push_back("Size", false) = "";
    }
    else
    {
        fields->push_back("Size", false) = static_cast<int64_t>(file_size(file.path()));
    }
    fields->push_back("Type", false) = modeName;

    fields->push_back("Modified", false) = modificationTime;
    fields->push_back("", false) = static_cast<int32_t>(index); // Fake key value
    ++index;

    if (access(absolute(file).filename().string().c_str(), R_OK) != 0)
    {
        (*fields)[0].view().flags = FL_ALIGN_LEFT;
        (*fields)[1].view().flags = FL_ALIGN_LEFT;
    }

    return fields;
}

std::shared_ptr<RegularExpression> DirectoryDS::wildcardToRegexp(const String& wildcard)
{
    String regexpStr("^");
    auto   groupStarted = false;
    auto   charClassStarted = false;

    size_t pos = 0;
    while (pos < wildcard.length())
    {
        auto chr = wildcard[pos];

        if (charClassStarted)
        {
            switch (chr)
            {
                case '!':
                    chr = '^';
                    break;
                case ']':
                    charClassStarted = false;
                    break;
                default:
                    break;
            }
            regexpStr += chr;
            ++pos;
            continue;
        }

        switch (chr)
        {
            case '{':
                groupStarted = true;
                chr = '(';
                break;
            case ',':
                if (groupStarted)
                {
                    chr = '|';
                }
                break;
            case '}':
                if (groupStarted)
                {
                    groupStarted = false;
                    chr = ')';
                }
                break;
            case '\\':
                regexpStr += chr;
                ++pos;
                if (pos < wildcard.length())
                {
                    regexpStr += wildcard[pos];
                }
                break;
            case '?':
                chr = '.';
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
        regexpStr += chr;
        ++pos;
    }
    regexpStr += "$";
    return make_shared<RegularExpression>(regexpStr);
}
