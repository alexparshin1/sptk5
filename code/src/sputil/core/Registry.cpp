/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Registry.cpp - description                             ║
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

#include <sptk5/Registry.h>
#include <sptk5/SystemException.h>
#include <sys/stat.h>

using namespace std;
using namespace sptk;

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <io.h>
    #include <direct.h>

    #define S_ISLNK(m)      (false)
    #define S_ISEXEC(m)     (((m) & _S_IEXEC) == _S_IEXEC)
    #if !defined(__UNIX_COMPILER__) && !defined(__BORLANDC__)
        #define S_ISREG(m)      (((m) & _S_IFREG) == _S_IFREG)
        #define S_ISDIR(m)      (((m) & _S_IFDIR) == _S_IFDIR)
        #define S_ISBLK(m)      (((m) & _S_IFBLK) == _S_IFBLK)
    #endif
#endif

//----------------------------------------------------------------------------
String Registry::homeDirectory()
{
#ifndef _WIN32
    const char* hdir = getenv("HOME");
    if (hdir == nullptr)
        hdir = ".";
    String homeDir = trim(hdir);
    if (homeDir.empty())
        homeDir = ".";
    homeDir += "/";
#else

    char *hdrive = getenv("HOMEDRIVE");
    char *hdir   = getenv("HOMEPATH");
    if (!hdir && !hdrive) {
        const char* wdir = getenv("WINDIR");
        if (wdir == nullptr)
            return "C:\\";
        return String(wdir) + String("\\");
    }

    string homeDrive;
    string homeDir;
    if (hdrive)
        homeDrive = hdrive;
    if (hdir)
        homeDir = hdir;
    if (homeDir == "\\")
        homeDir = homeDrive + "\\";
    else
        homeDir = homeDrive + homeDir;
    homeDir += "\\Local Settings\\Application Data\\Programs\\";
#endif

    return homeDir;
}

Registry::Registry(const String& fileName, const String& _programGroupName, RegistryMode mode)
: xml::Document("Configuration"), m_fileName(fileName)
{
    String programGroupName(_programGroupName);
    if (!m_fileName.empty()) {
        String directory;
        if (mode == USER_REGISTRY)
            directory = homeDirectory();
        directory += "/";
        if (!programGroupName.empty()) {
            while (programGroupName[0] == '.')
                programGroupName = programGroupName.substr(1);
            programGroupName = programGroupName.replace("[\\/\\\\]+", "_");
        }
        if (!programGroupName.empty())
            directory += "." + programGroupName + "/";
        m_fileName = directory + m_fileName;
    }
    m_fileName = m_fileName.replace("[\\/\\\\]+", "/");
}

Registry::~Registry()
{
    clear();
}

void Registry::prepareDirectory()
{
    struct stat st = {};
    size_t pos = m_fileName.rfind('/');
    if (pos == STRING_NPOS)
        return;
    string directory = m_fileName.substr(0, pos);
    if (stat(directory.c_str(), &st) == 0) {
        if (!S_ISDIR(st.st_mode))
            throw SystemException("Can't open directory '" + directory + "'");
    } else {
#ifdef _WIN32
        if (mkdir(directory.c_str()) != 0)
            throw SystemException("Can't create directory '"+directory+"'");
#else
        if (mkdir(directory.c_str(), 0770) != 0)
            throw SystemException("Can't create directory '" + directory + "'");
#endif

    }
}

void Registry::load(const Strings& inputData)
{
    clear();
    Buffer buffer(inputData.asString("\n"));
    xml::Document::load(buffer);
}

void Registry::load(const char* inputData)
{
    clear();
    xml::Document::load(inputData);
}

void Registry::load()
{
    Buffer inputData;
    inputData.loadFromFile(m_fileName);
    xml::Document::load(inputData);
}

void Registry::save(Strings& outputData)
{
    Buffer buffer;
    prepareDirectory();
    outputData.clear();
    xml::Document::save(buffer, true);
    outputData.fromString(buffer.data(), "\n", Strings::SM_DELIMITER);
}

void Registry::save()
{
    Buffer outputData;
    prepareDirectory();
    xml::Document::save(outputData, true);
    outputData.saveToFile(m_fileName);
}

void Registry::clean(Node* node)
{
    auto itor = node->begin();
    auto iend = node->end();
    xml::NodeVector toDelete;
    for (; itor != iend; ++itor) {
        Node* anode = *itor;
        if (anode->type() != DOM_ELEMENT) {
            toDelete.push_back(anode);
            continue;
        }
        if (!anode->empty())
            clean(anode);
    }
    auto it = toDelete.begin();
    for (; it != toDelete.end(); ++it)
        node->remove(*it);
}

void Registry::load(const xml::Document& data)
{
    clear();
    copy(data);
    clean(this);
}

void Registry::save(xml::Document& data) const
{
    data.copy(*this);
}
