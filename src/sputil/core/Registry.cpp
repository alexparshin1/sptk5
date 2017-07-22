/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Registry.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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
string Registry::homeDirectory()
{
#ifndef _WIN32
    string homeDir = trim(getenv("HOME"));
    if (homeDir.empty())
        homeDir = ".";
    homeDir += "/";
#else

    char *hdrive = getenv("HOMEDRIVE");
    char *hdir   = getenv("HOMEPATH");
    if (!hdir && !hdrive)
        return getenv("WINDIR") + string("\\");

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

Registry::Registry(const string& fileName, string programGroupName, RegistryMode mode)
: XMLDocument("Configuration"), m_fileName(fileName)
{
    if (!m_fileName.empty()) {
        string directory;
        if (mode == USER_REGISTRY)
            directory = homeDirectory();
        directory += "/";
        if (!programGroupName.empty()) {
            while (programGroupName[0] == '.')
                programGroupName = programGroupName.substr(1);
            programGroupName = replaceAll(programGroupName, "\\", "_");
            programGroupName = replaceAll(programGroupName, "/", "_");
        }
        if (!programGroupName.empty())
            directory += "." + programGroupName + "/";
        m_fileName = directory + m_fileName;
    }
    m_fileName = replaceAll(m_fileName, "\\", "/");
    m_fileName = replaceAll(m_fileName, "//", "/");
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
            throw Exception("Can't open directory '" + directory + "'");
    } else {
#ifdef _WIN32
        if (mkdir(directory.c_str()))
            throw Exception("Can't create directory '"+directory+"'");
#else
        mkdir(directory.c_str(), 0770);
        throw Exception("Can't create directory '" + directory + "'");
#endif

    }
}

void Registry::load(const Strings& inputData)
{
    clear();
    Buffer buffer = inputData.asString("\n");
    XMLDocument::load(buffer);
}

void Registry::load(const char* inputData)
{
    clear();
    XMLDocument::load(inputData);
}

void Registry::load()
{
    Buffer inputData;
    inputData.loadFromFile(m_fileName);
    XMLDocument::load(inputData);
}

void Registry::save(Strings& outputData)
{
    Buffer buffer;
    prepareDirectory();
    outputData.clear();
    XMLDocument::save(buffer);
    outputData.fromString(buffer.data(), "\n", Strings::SM_DELIMITER);
}

void Registry::save()
{
    Buffer outputData;
    prepareDirectory();
    XMLDocument::save(outputData);
    outputData.saveToFile(m_fileName);
}

void Registry::clean(XMLNode* node)
{
    XMLNode::iterator itor = node->begin();
    XMLNode::iterator iend = node->end();
    XMLNodeVector toDelete;
    for (; itor != iend; ++itor) {
        XMLNode* anode = *itor;
        if (anode->type() != DOM_ELEMENT) {
            toDelete.push_back(anode);
            continue;
        }
        if (anode->size())
            clean(anode);
    }
    XMLNodeVector::iterator it = toDelete.begin();
    for (; it != toDelete.end(); ++it)
        node->remove
                    (*it);
}

void Registry::load(const XMLDocument& data)
{
    clear();
    copy(data);
    clean(this);
}

void Registry::save(XMLDocument& data) const
{
    data.copy(*this);
}
